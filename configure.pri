defineReplace(extractVersion)      { return($$replace(1, ^(\\d+\\.\\d+\\.\\d+)(svn|git)?$, \\1)) }
defineReplace(extractMajorVersion) { return($$replace(1, ^(\\d+)\\.\\d+\\.\\d+(svn|git)?$, \\1)) }
defineReplace(extractMinorVersion) { return($$replace(1, ^\\d+\\.(\\d+)\\.\\d+(svn|git)?$, \\1)) }
defineReplace(extractPatchVersion) { return($$replace(1, ^\\d+\\.\\d+\\.(\\d+)(svn|git)?$, \\1)) }

defineTest(versionIsAtLeast) {
    actual_major_version = $$extractMajorVersion($$1)
    actual_minor_version = $$extractMinorVersion($$1)
    actual_patch_version = $$extractPatchVersion($$1)
    required_min_major_version = $$extractMajorVersion($$2)
    required_min_minor_version = $$extractMinorVersion($$2)
    required_min_patch_version = $$extractPatchVersion($$2)

    isEqual(actual_major_version, $$required_min_major_version) {
        isEqual(actual_minor_version, $$required_min_minor_version) {
            isEqual(actual_patch_version, $$required_min_patch_version): return(true)
            greaterThan(actual_patch_version, $$required_min_patch_version): return(true)
        }
        greaterThan(actual_minor_version, $$required_min_minor_version): return(true)
    }
    greaterThan(actual_major_version, $$required_min_major_version): return(true)

    return(false)
}

defineReplace(findLLVMVersionFromLibDir) {
    libdir = $$1
    version_dirs = $$files($$libdir/clang/*)
    for (version_dir, version_dirs) {
        fileName = $$basename(version_dir)
        version = $$find(fileName, ^(\\d+\\.\\d+\\.\\d+)$)
        !isEmpty(version) {
            isEmpty(candidateVersion): candidateVersion = $$version
            else: versionIsAtLeast($$version, $$candidateVersion): candidateVersion = $$version
        }
    }
    return($$candidateVersion)
}

defineReplace(FindCleanLLVMInstallDir) {
    # Assume libclang is installed on the target system
    equals(QMAKE_HOST.os, Windows) {
        # on Windows we have only two host compilers, MSVC or mingw. The former we never
        # use for cross-compilation where it isn't also the target compiler. The latter
        # is not detectable as this .prf file is evaluated against the target configuration
        # and therefore checking for "mingw" won't work when the target compiler is clang (Android)
        # or qcc (QNX).
        msvc {
            isEmpty(LLVM_INSTALL_DIR): LLVM_INSTALL_DIR = $$(LLVM_INSTALL_DIR_MSVC)
        } else {
            isEmpty(LLVM_INSTALL_DIR): LLVM_INSTALL_DIR = $$(LLVM_INSTALL_DIR_MINGW)
        }
    }
    isEmpty(LLVM_INSTALL_DIR): LLVM_INSTALL_DIR = $$(LLVM_INSTALL_DIR)
    isEmpty(LLVM_INSTALL_DIR) {
        llvmConfigCandidates = \
            llvm-config-10 \
            llvm-config-9 \
            llvm-config-8 \
            llvm-config-7 \
            llvm-config-6.0 \
            llvm-config-5.0 \
            llvm-config-4.0 \
            llvm-config-3.9 \
            llvm-config

        for (candidate, llvmConfigCandidates) {
            LLVM_INSTALL_DIR = $$system("$$candidate --prefix 2>$$QMAKE_SYSTEM_NULL_DEVICE")
            !isEmpty(LLVM_INSTALL_DIR) {
                CLANG_INCLUDEPATH = $$system("$$candidate --includedir 2>/dev/null")
                LIBCLANG_MAIN_HEADER = $$CLANG_INCLUDEPATH/clang-c/Index.h
                !exists($$LIBCLANG_MAIN_HEADER) {
                    !isEmpty(LLVM_INSTALL_DIR): \
                        qtLog("Cannot find libclang's main header file, candidate: $${LIBCLANG_MAIN_HEADER}.")
                        continue
                } else {
                    qtLog("QDoc:" \
                          "Using Clang installation found in $${LLVM_INSTALL_DIR}." \
                          "Set the LLVM_INSTALL_DIR environment variable to override.")
                    break()
                }
            }
        }
    }
    LLVM_INSTALL_DIR = $$clean_path($$LLVM_INSTALL_DIR)
    return($$LLVM_INSTALL_DIR)
}

defineReplace(FindClangInstallDir) {
    llvm_install_dir=$$1
    contains(QMAKE_HOST.arch, x86_64): \
         clangInstallDir = $$replace(llvm_install_dir, _ARCH_, 64)
    else: \
        clangInstallDir = $$replace(llvm_install_dir, _ARCH_, 32)
    isEmpty(llvm_install_dir) {
        macos {
            # Default to homebrew llvm on macOS. The CLANG_VERSION test below will complain if missing.
            clangInstallDir = $$system("brew --prefix llvm")
        } else {
            clangInstallDir = /usr
        }
    }
    return($$clangInstallDir)
}

defineReplace(CheckClangCppLibForLupdateParser) {
    clangLibDir = $$1
    libToTest = clangTooling \
                          clangFrontendTool \
                          clangFrontend \
                          clangDriver \
                          clangSerialization \
                          clangCodeGen \
                          clangParse \
                          clangSema \
                          clangStaticAnalyzerFrontend \
                          clangStaticAnalyzerCheckers \
                          clangStaticAnalyzerCore \
                          clangAnalysis \
                          clangARCMigrate \
                          clangASTMatchers \
                          clangAST \
                          clangRewrite \
                          clangRewriteFrontend \
                          clangEdit \
                          clangLex \
                          clangIndex \
                          clangBasic

    versionIsAtLeast($$CLANG_VERSION, "9.0.0"){
        libToTest += clangToolingRefactoring
    } else {
        libToTest += clangToolingRefactor
    }


    CLANG_CPP_LIBS =
    for (lib, libToTest) {
        libFullPath =
        msvc {
            libFullPath += $$clangLibDir/$${lib}.lib
        } else {
            equals(QMAKE_HOST.os, Windows): \
                libFullPath += $$clangLibDir/$${lib}.lib
            else: \
                libFullPath += $$clangLibDir/lib$${lib}.a
        }

        CLANG_CPP_LIBS += -l$$lib
        !exists($$libFullPath): {
            CLANG_CPP_LIBS =
            qtLog("Cannot locate $$libFullPath.")
            return($$CLANG_CPP_LIBS)
        }
    }
    return($$CLANG_CPP_LIBS)
}

defineReplace(CheckClangLlvmLibForLupdateParser) {
    clangLibDir = $$1
    libToTest += LLVMOption \
                          LLVMProfileData \
                          LLVMMCParser \
                          LLVMMC \
                          LLVMBitReader \
                          LLVMCore \
                          LLVMBinaryFormat \
                          LLVMSupport \
                          LLVMDemangle

    versionIsAtLeast($$CLANG_VERSION, "9.0.0") {
        libToTest += LLVMBitstreamReader\
                     LLVMRemarks
    }

    CLANG_LLVM_LIBS =
    for (lib, libToTest) {
        libFullPath =
        msvc {
            libFullPath += $$clangLibDir/$${lib}.lib
        } else {
            equals(QMAKE_HOST.os, Windows): \
                libFullPath += $$clangLibDir/$${lib}.lib
            else: \
                libFullPath += $$clangLibDir/lib$${lib}.a
        }

        CLANG_LLVM_LIBS += -l$$lib
        !exists($$libFullPath): {
            CLANG_LLVM_LIBS =
            return($$CLANG_LLVM_LIBS)
        }
    }
    !equals(QMAKE_HOST.os, Windows): {
        equals(QMAKE_HOST.os, Darwin): CLANG_LLVM_LIBS += -lz -lcurses
        else: CLANG_LLVM_LIBS += -lz -ltinfo
    }
    return($$CLANG_LLVM_LIBS)
}

defineTest(qtConfTest_libclang) {
    isEmpty(QDOC_USE_STATIC_LIBCLANG): QDOC_USE_STATIC_LIBCLANG = $$(QDOC_USE_STATIC_LIBCLANG)

    LLVM_INSTALL_DIR = $$FindCleanLLVMInstallDir()
    isEmpty(LLVM_INSTALL_DIR) {
        equals(QMAKE_HOST.os, Windows) {
            return(false)
        }
    }
    clangInstallDir = $$FindClangInstallDir($$LLVM_INSTALL_DIR)

    # note: llvm_config only exits on unix
    llvm_config = $$clangInstallDir/bin/llvm-config
    exists($$llvm_config) {
        CLANG_LIBDIR = $$system("$$llvm_config --libdir 2>/dev/null")
        CLANG_INCLUDEPATH = $$system("$$llvm_config --includedir 2>/dev/null")
        output = $$system("$$llvm_config --version 2>/dev/null")
        CLANG_VERSION = $$extractVersion($$output)
    } else {
        CLANG_LIBDIR = $$clangInstallDir/lib
        CLANG_INCLUDEPATH = $$clangInstallDir/include
        CLANG_VERSION = $$findLLVMVersionFromLibDir($$CLANG_LIBDIR)
    }
    isEmpty(CLANG_VERSION) {
        !isEmpty(LLVM_INSTALL_DIR): \
            qtLog("Cannot determine version of clang installation in $${clangInstallDir}.")
        return(false)
    } else {
        CLANG_MAJOR_VERSION = $$extractMajorVersion($$CLANG_VERSION)
        CLANG_MINOR_VERSION = $$extractMinorVersion($$CLANG_VERSION)
        CLANG_PATCH_VERSION = $$extractPatchVersion($$CLANG_VERSION)
    }

    LIBCLANG_MAIN_HEADER = $$CLANG_INCLUDEPATH/clang-c/Index.h
    !exists($$LIBCLANG_MAIN_HEADER) {
        !isEmpty(LLVM_INSTALL_DIR): \
            qtLog("Cannot find libclang's main header file, candidate: $${LIBCLANG_MAIN_HEADER}.")
        return(false)
    }

    !contains(QMAKE_DEFAULT_LIBDIRS, $$CLANG_LIBDIR): CLANG_LIBS = -L$${CLANG_LIBDIR}

    CLANG_DEFINES =
    HAS_CLANGCPP = false
    CLANGCPP_LIB =

    isEmpty(QDOC_USE_STATIC_LIBCLANG) {
        # entering here in case of user (as opposed to CI)

        #--------------- QDoc needs --------------------------------
        equals(QMAKE_HOST.os, Windows): \
            CLANG_LIBS += -llibclang -ladvapi32 -lshell32 -lversion
        else: \
            CLANG_LIBS += -lclang

        #--------------- Lupdate clang-based parser needs Clang C++ and llvm libraries
        CLANGCPP_DY_LIB = $$CLANG_LIBDIR/libclang_shared.so
        # Checking clang cpp libraries
        # check for shared libraries (not available for Windows at the moment)
        exists($$CLANGCPP_DY_LIB): {
            CLANGCPP_LIBS += -lclang_shared
        } else {
        # check for static libraries
            CLANGCPP_LIBS += $$CheckClangCppLibForLupdateParser($$CLANG_LIBDIR)
        }
        # Checking for LLVM libraries needed for Lupdate clang-based parser
        # At this point, if CLANGCPP_LIBS is empty, no need to go further
        !isEmpty(CLANGCPP_LIBS): {
            LLVM_DY_LIB = $$CLANG_LIBDIR/libLLVM.so
            # check for shared libraries  (not available for Windows at the moment)
            exists($$LLVM_DY_LIB): {
                CLANGCPP_LIBS += -lLLVM
                HAS_CLANGCPP = true # Clang cpp and llvm libraries have been found
            } else: {
            # check for static libraries
                CLANGLLVM_LIBS = $$CheckClangLlvmLibForLupdateParser($$CLANG_LIBDIR)
                !isEmpty(CLANGLLVM_LIBS): {
                    CLANGCPP_LIBS += $$CLANGLLVM_LIBS
                    HAS_CLANGCPP = true # Clang cpp and llvm libraries have been found
                }
            }
        #----------------------------------------------------------------------
        }
    } else {
        # CI
        HAS_CLANGCPP = true #just assuming for now
        equals(QMAKE_HOST.os, Windows) {
            versionIsAtLeast($$CLANG_VERSION, "10.0.0") {
                CLANG_DEFINES += CINDEX_NO_EXPORTS
            } else {
                CLANG_DEFINES += CINDEX_LINKAGE=
            }
        }
        !equals(QMAKE_HOST.os, Darwin):!msvc: CLANG_LIBS+=-Wl,--start-group
        CLANG_LIBS += -lclangAnalysis \
                -lclangARCMigrate \
                -lclangAST \
                -lclangASTMatchers \
                -lclangBasic \
                -lclangCodeGen \
                -lclangCrossTU \
                -lclangDriver \
                -lclangDynamicASTMatchers \
                -lclangEdit \
                -lclangFormat \
                -lclangFrontend \
                -lclangFrontendTool \
                -lclangHandleCXX \
                -lclangIndex \
                -lclangLex \
                -lclangParse \
                -lclangRewrite \
                -lclangRewriteFrontend \
                -lclangSema \
                -lclangSerialization \
                -lclangStaticAnalyzerCheckers \
                -lclangStaticAnalyzerCore \
                -lclangStaticAnalyzerFrontend \
                -lclangTooling \
                -lclangToolingASTDiff \
                -lclangToolingCore

        versionIsAtLeast($$CLANG_VERSION, "10.0.0") {
            equals(QMAKE_HOST.os, Windows): \
                CLANG_LIBS += -llibclang
            else: \
                CLANG_LIBS += -lclang

            CLANG_LIBS += -lclangToolingInclusions
        } else {
            equals(QMAKE_HOST.os, Windows): \
                CLANG_LIBS += -llibclang_static
            else: \
                CLANG_LIBS += -lclang_static

            CLANG_LIBS += \
                    -lclangApplyReplacements \
                    -lclangChangeNamespace \
                    -lclangDaemon \
                    -lclangIncludeFixer \
                    -lclangIncludeFixerPlugin \
                    -lclangMove \
                    -lclangQuery \
                    -lclangReorderFields \
                    -lclangTidy \
                    -lclangTidyAndroidModule \
                    -lclangTidyBoostModule \
                    -lclangTidyBugproneModule \
                    -lclangTidyCERTModule \
                    -lclangTidyCppCoreGuidelinesModule \
                    -lclangTidyFuchsiaModule \
                    -lclangTidyGoogleModule \
                    -lclangTidyHICPPModule \
                    -lclangTidyLLVMModule \
                    -lclangTidyMiscModule \
                    -lclangTidyModernizeModule \
                    -lclangTidyMPIModule \
                    -lclangTidyObjCModule \
                    -lclangTidyPerformanceModule \
                    -lclangTidyPlugin \
                    -lclangTidyReadabilityModule \
                    -lclangTidyUtils \
                    -lclangToolingRefactor \
                    -lfindAllSymbols
        }

        msvc {
            LLVM_LIBS_STRING += $$system("$$llvm_config --libnames")
        } else {
            LLVM_LIBS_STRING += $$system("$$llvm_config --libs")
        }
        LLVM_LIBS_STRING += $$system("$$llvm_config --system-libs")
        CLANG_LIBS += $$split(LLVM_LIBS_STRING, " ")

        !equals(QMAKE_HOST.os, Darwin):!msvc: CLANG_LIBS+=-Wl,--end-group
        equals(QMAKE_HOST.os, Windows): CLANG_LIBS += -lversion
    }

    !versionIsAtLeast($$CLANG_VERSION, "3.9.0") {
        log("LLVM/Clang version >= 3.9.0 required, version provided: $${CLANG_VERSION}.$$escape_expand(\\n)")
        return(false)
    }

    !versionIsAtLeast($$CLANG_VERSION, "8.0.0") {
        # if not InitLLVM.h is missing
        log("LLVM/Clang version >= 8.0.0 required for Clang-based lupdate parser. \
            Version provided: $${CLANG_VERSION}.$$escape_expand(\\n)")
        HAS_CLANGCPP = false
    }

    $${1}.libs = $$CLANG_LIBS
    export($${1}.libs)
    $${1}.cache += libs

    $${1}.includepath = $$CLANG_INCLUDEPATH
    export($${1}.includepath)
    $${1}.cache += includepath

    $${1}.libdir = $$CLANG_LIBDIR
    export($${1}.libdir)
    $${1}.cache += libdir

    $${1}.defines = $$CLANG_DEFINES
    export($${1}.defines)
    $${1}.cache += defines

    $${1}.version = $$CLANG_VERSION
    export($${1}.version)
    $${1}.cache += version

    $${1}.major_version = $$CLANG_MAJOR_VERSION
    export($${1}.major_version)
    $${1}.cache += major_version

    $${1}.minor_version = $$CLANG_MINOR_VERSION
    export($${1}.minor_version)
    $${1}.cache += minor_version

    $${1}.patch_version = $$CLANG_PATCH_VERSION
    export($${1}.patch_version)
    $${1}.cache += patch_version

    $${1}.has_clangcpp = $$HAS_CLANGCPP
    export($${1}.has_clangcpp)
    $${1}.cache += has_clangcpp

    $${1}.clangcpp_libs = $$CLANGCPP_LIBS
    export($${1}.clangcpp_libs)
    $${1}.cache += clangcpp_libs

    export($${1}.cache)

    return(true)
}
