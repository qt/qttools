// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <llvm/Object/MachO.h>
#include <llvm/Object/MachOUniversal.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/raw_ostream.h>

#include <filesystem>
#include <optional>
#include <random>
#include <set>
#include <string>
#include <string_view>

using namespace llvm;
using namespace object;

namespace fs = std::filesystem;

namespace {

struct CommandLineArgs
{
    fs::path binaryPath;
    std::set<std::string> excludedClasses;
    bool quietMode {false};
    bool dryRun {false};
    std::string pattern;
    std::string replacement;
};

void printUsage(const char *progName)
{
    errs() << "Usage: " << progName << " [OPTIONS] <binary_to_patch>\n\n"
           << "A tool to patch Objective-C metadata in Mach-O binaries.\n\n"
           << "Positional arguments:\n"
           << "  binary_to_patch          The binary file to patch\n\n"
           << "Options:\n"
           << "  -h, --help               Show this help message and exit\n"
           << "  --quiet                  Suppress output messages\n"
           << "  --dry-run                Perform a dry run without modifying the file\n"
           << "  --exclude CLASS          Exclude class name from patching (can be repeated)\n"
           << "  --replace PATTERN REPLACEMENT\n"
           << "                           Replace pattern with replacement string\n"
           << "                           (must be same length for binary safety)\n\n"
           << "Examples:\n"
           << "  " << progName << " myapp.app/Contents/MacOS/myapp\n"
           << "  " << progName << " --quiet --exclude MyClass myapp\n"
           << "  " << progName << " --replace QtCore QTCore myapp\n";
}

std::optional<CommandLineArgs> parseCommandLine(int argc, char **argv)
{
    CommandLineArgs args;
    fs::path binaryPath;

    if (argc < 2) {
        errs() << "Error: No binary file specified.\n\n";
        printUsage(argv[0]);
        return std::nullopt;
    }

    int i = 1;
    while (i < argc) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return std::nullopt;
        } else if (arg == "--quiet") {
            args.quietMode = true;
            i++;
        } else if (arg == "--dry-run") {
            args.dryRun = true;
            i++;
        } else if (arg == "--exclude") {
            if (i + 1 >= argc) {
                errs() << "Error: --exclude requires a CLASS argument.\n";
                return std::nullopt;
            }
            args.excludedClasses.insert(argv[i + 1]);
            i += 2;
        } else if (arg == "--replace") {
            if (i + 2 >= argc) {
                errs() << "Error: --replace requires PATTERN and REPLACEMENT arguments.\n";
                return std::nullopt;
            }
            args.pattern = argv[i + 1];
            args.replacement = argv[i + 2];

            if (args.pattern.empty()) {
                errs() << "Error: replacement pattern cannot be empty.\n";
                return std::nullopt;
            }
            if (args.pattern.length() != args.replacement.length()) {
                errs() << "Error: for binary safety, the replacement pattern and "
                       << "the replacement string must be the same length.\n";
                return std::nullopt;
            }
            i += 3;
        } else if (arg[0] == '-') {
            errs() << "Error: unknown option '" << arg << "'.\n\n";
            printUsage(argv[0]);
            return std::nullopt;
        } else {
            if (!binaryPath.empty()) {
                errs() << "Error: multiple binary files specified.\n";
                return std::nullopt;
            }
            binaryPath = argv[i];
            i++;
        }
    }

    if (binaryPath.empty()) {
        errs() << "Error: No binary file specified.\n\n";
        printUsage(argv[0]);
        return std::nullopt;
    }

    std::error_code ec;
    if (!fs::exists(binaryPath, ec) || ec) {
        errs() << "Error: file '" << binaryPath.string() << "' does not exist.\n";
        return std::nullopt;
    }

    args.binaryPath = binaryPath;
    return args;
}

std::string generateRandomString(size_t length)
{
    constexpr std::string_view charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234"
                                         "56789";
    static std::mt19937 generator(std::random_device {}());
    std::uniform_int_distribution<int> distribution(0, charset.length() - 1);
    std::string result;
    for (size_t i = 0; i < length; ++i)
        result += charset[distribution(generator)];
    return result;
}

// Converts a virtual address to a file offset relative to the start of the Mach-O slice.
std::optional<uint64_t> virtualAddressToFileOffset(const MachOObjectFile *obj, uint64_t va)
{
    for (const auto &lci : obj->load_commands()) {
        if (lci.C.cmd == MachO::LC_SEGMENT_64) {
            const MachO::segment_command_64 seg = obj->getSegment64LoadCommand(lci);
            if (va >= seg.vmaddr && va < (seg.vmaddr + seg.vmsize))
                return (va - seg.vmaddr) + seg.fileoff;
        } else if (lci.C.cmd == MachO::LC_SEGMENT) {
            const MachO::segment_command seg = obj->getSegmentLoadCommand(lci);
            if (va >= seg.vmaddr && va < (seg.vmaddr + seg.vmsize))
                return (va - seg.vmaddr) + seg.fileoff;
        }
    }
    return std::nullopt;
}

void patchClassNameSection(const SectionRef &section,
                           const MachOObjectFile *machOObj,
                           WritableMemoryBuffer &writableMB,
                           uint64_t sliceOffset,
                           const CommandLineArgs &args)
{
    uint64_t sectionFileOffset = 0;
    if (machOObj->is64Bit()) {
        const MachO::section_64 sec = machOObj->getSection64(section.getRawDataRefImpl());
        sectionFileOffset = sec.offset;
    } else {
        const MachO::section sec = machOObj->getSection(section.getRawDataRefImpl());
        sectionFileOffset = sec.offset;
    }

    Expected<StringRef> contentsOrErr = section.getContents();
    if (auto e = contentsOrErr.takeError()) {
        consumeError(std::move(e));
        return;
    }
    StringRef contents = *contentsOrErr;
    const char *current = contents.begin();
    while (current < contents.end()) {
        StringRef name(current);
        if (name.empty()) {
            current++;
            continue;
        }

        if (args.excludedClasses.count(name.str())) {
            if (!args.quietMode)
                outs() << "[CLASS] Skipping excluded class: " << name.str() << "\n";
            current += name.size() + 1;
            continue;
        }

        uint64_t realFileOffset = sliceOffset + sectionFileOffset + (current - contents.begin());

        if (!args.pattern.empty()) {
            std::string originalName = name.str();
            std::string newName = originalName;
            size_t pos = 0;
            bool replaced = false;
            while ((pos = newName.find(args.pattern, pos)) != std::string::npos) {
                newName.replace(pos, args.pattern.length(), args.replacement);
                pos += args.replacement.length();
                replaced = true;
            }

            if (replaced) {
                if (!args.quietMode) {
                    outs() << "[CLASS] Found: " << originalName << " at file offset "
                           << realFileOffset << "\n"
                           << "  -> Replaced with: " << newName << "\n";
                }
                char *patchLocation = writableMB.getBufferStart() + realFileOffset;
                memcpy(patchLocation, newName.c_str(), originalName.length());
            }
        } else {
            if (!args.quietMode)
                outs() << "[CLASS] Found: " << name.str() << " at file offset "
                       << realFileOffset << "\n";

            std::string randomString = generateRandomString(name.size());
            char *patchLocation = writableMB.getBufferStart() + realFileOffset;
            memcpy(patchLocation, randomString.c_str(), randomString.length());
            if (!args.quietMode)
                outs() << "  -> Replaced with: " << randomString << "\n";
        }

        current += name.size() + 1;
    }
}

void patchCategoryListSection(const SectionRef &section,
                              const MachOObjectFile *machOObj,
                              const MemoryBuffer &originalMB,
                              WritableMemoryBuffer &writableMB,
                              uint64_t sliceOffset,
                              const CommandLineArgs &args)
{
    Expected<StringRef> contentsOrErr = section.getContents();
    if (auto e = contentsOrErr.takeError()) {
        consumeError(std::move(e));
        return;
    }
    StringRef contents = *contentsOrErr;
    const char *data = contents.data();
    unsigned ptrSize = machOObj->is64Bit() ? 8 : 4;

    for (unsigned i = 0; i + ptrSize <= contents.size(); i += ptrSize) {
        uint64_t categoryVa = (ptrSize == 8) ? *(const uint64_t *)(data + i)
                                             : *(const uint32_t *)(data + i);
        auto categoryOffsetOpt = virtualAddressToFileOffset(machOObj, categoryVa);
        if (!categoryOffsetOpt)
            continue;

        const char *categoryStructPtr
            = originalMB.getBufferStart() + sliceOffset + *categoryOffsetOpt;
        uint64_t categoryNameVa = (ptrSize == 8) ? *(const uint64_t *)categoryStructPtr
                                                 : *(const uint32_t *)categoryStructPtr;

        auto nameOffsetOpt = virtualAddressToFileOffset(machOObj, categoryNameVa);
        if (!nameOffsetOpt)
            continue;

        uint64_t realNameOffset = sliceOffset + *nameOffsetOpt;
        StringRef categoryName(originalMB.getBufferStart() + realNameOffset);
        if (categoryName.empty())
            continue;

        if (!args.pattern.empty()) {
            std::string originalName = categoryName.str();
            std::string newName = originalName;
            size_t pos = 0;
            bool replaced = false;
            while ((pos = newName.find(args.pattern, pos)) != std::string::npos) {
                newName.replace(pos, args.pattern.length(), args.replacement);
                pos += args.replacement.length();
                replaced = true;
            }

            if (replaced) {
                if (!args.quietMode) {
                    outs() << "[CATEGORY] Found: " << originalName << " at file offset "
                           << realNameOffset << "\n"
                           << "  -> Replaced with: " << newName << "\n";
                }
                char *patchLocation = writableMB.getBufferStart() + realNameOffset;
                memcpy(patchLocation, newName.c_str(), originalName.length());
            }
        } else {
            if (!args.quietMode)
                outs() << "[CATEGORY] Found: " << categoryName.str() << " at file offset "
                       << realNameOffset << "\n";

            std::string randomString = generateRandomString(categoryName.size());
            char *patchLocation = writableMB.getBufferStart() + realNameOffset;
            memcpy(patchLocation, randomString.c_str(), randomString.length());

            if (!args.quietMode)
                outs() << "  -> Replaced with: " << randomString << "\n";
        }
    }
}

Error patchMachOSlice(MachOObjectFile *machOObj,
                      const MemoryBuffer &originalMB,
                      WritableMemoryBuffer &writableMB,
                      uint64_t sliceOffset,
                      const CommandLineArgs &args)
{
    if (!args.quietMode) {
        outs() << "--- Patching architecture: " << machOObj->getArchTriple().getArchName()
               << " (slice offset: " << sliceOffset << ") ---\n";
    }
    for (const SectionRef &section : machOObj->sections()) {
        Expected<StringRef> sectionNameOrErr = section.getName();
        if (auto e = sectionNameOrErr.takeError())
            return e;
        StringRef sectionName = *sectionNameOrErr;

        if (sectionName == "__objc_classname")
            patchClassNameSection(section, machOObj, writableMB, sliceOffset, args);
        else if (sectionName == "__objc_catlist")
            patchCategoryListSection(section, machOObj, originalMB, writableMB, sliceOffset, args);
    }
    return Error::success();
}

} // namespace

int main(int argc, char **argv)
{
    auto argsOpt = parseCommandLine(argc, argv);
    if (!argsOpt)
        return 1;
    const auto &args = *argsOpt;

    Expected<OwningBinary<Binary>> binOrErr = createBinary(args.binaryPath.string());
    if (auto e = binOrErr.takeError()) {
        errs() << "Error opening binary: " << toString(std::move(e)) << "\n";
        return 1;
    }
    OwningBinary<Binary> &bin = binOrErr.get();

    ErrorOr<std::unique_ptr<MemoryBuffer>> mbOrErr = MemoryBuffer::getFile(args.binaryPath.string());
    if (std::error_code ec = mbOrErr.getError()) {
        errs() << "Error reading file into buffer: " << ec.message() << "\n";
        return 1;
    }
    std::unique_ptr<MemoryBuffer> originalMB {std::move(mbOrErr.get())};
    std::unique_ptr<WritableMemoryBuffer> writableMB
        = WritableMemoryBuffer::getNewMemBuffer(originalMB->getBufferSize());
    memcpy(writableMB->getBufferStart(), originalMB->getBufferStart(), originalMB->getBufferSize());

    if (auto *machOUni = dyn_cast<MachOUniversalBinary>(bin.getBinary())) {
        for (const auto &objForArch : machOUni->objects()) {
            Expected<std::unique_ptr<MachOObjectFile>> machOObjOrErr = objForArch.getAsObjectFile();
            if (auto e = machOObjOrErr.takeError()) {
                errs() << "Failed to get object for architecture: " << toString(std::move(e))
                       << "\n";
                continue;
            }
            if (auto e = patchMachOSlice(
                    machOObjOrErr->get(), *originalMB, *writableMB, objForArch.getOffset(), args)) {
                errs() << "Failed to patch Mach-O slice: " << toString(std::move(e)) << "\n";
            }
        }
    } else if (auto *machOObj = dyn_cast<MachOObjectFile>(bin.getBinary())) {
        if (auto e = patchMachOSlice(machOObj, *originalMB, *writableMB, 0, args)) {
            errs() << "Failed to patch Mach-O file: " << toString(std::move(e)) << "\n";
            return 1;
        }
    } else {
        errs() << "The provided file is not a valid Mach-O binary.\n";
        return 1;
    }

    if (args.dryRun) {
        if (!args.quietMode)
            outs() << "\nDry run complete. Binary was not modified.\n";
        return 0;
    }

    std::error_code ec;
    raw_fd_ostream outFile(args.binaryPath.string(), ec);
    if (ec) {
        errs() << "Error opening file for writing: " << ec.message() << "\n";
        return 1;
    }
    outFile.write(writableMB->getBufferStart(), writableMB->getBufferSize());
    outFile.close();

    if (!args.quietMode)
        outs() << "\nSuccessfully patched binary in-place: " << args.binaryPath.string() << "\n";

    return 0;
}
