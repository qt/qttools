function(qt_tools_find_llvm_version_from_lib_dir lib_dir out_var)
    set(candidate_version "")

    file(GLOB version_dirs LIST_DIRECTORIES true "${lib_dir}/clang/*")
    foreach(version_dir ${version_dirs})
        get_filename_component(file_name "${version_dir}" NAME)
        if(file_name MATCHES "^([0-9]+\.[0-9]+\.[0-9]+)$")
            if(NOT candidate_version)
                set(candidate_version "${CMAKE_MATCH_1}")
            else()
                if("${CMAKE_MATCH_1}" VERSION_GREATER_EQUAL "${candidate_version}")
                    set(candidate_version "${CMAKE_MATCH_1}")
                endif()
            endif()
        endif()
    endforeach()
    set(${out_var} "${candidate_version}" PARENT_SCOPE)
endfunction()

function(qt_tools_find_lib_clang)
    # We already looked up all the libclang information before, just create the target
    # and exit early.
    if(QT_LIB_CLANG_LIBS)
        qt_tools_create_lib_clang_target()
        set(WrapLibClang_FOUND TRUE PARENT_SCOPE)
        return()
    endif()

    if(NOT QDOC_USE_STATIC_LIBCLANG AND DEFINED ENV{QDOC_USE_STATIC_LIBCLANG})
        set(QDOC_USE_STATIC_LIBCLANG "$ENV{QDOC_USE_STATIC_LIBCLANG}")
    endif()

    if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
        # on Windows we have only two host compilers, MSVC or mingw. The former we never
        # use for cross-compilation where it isn't also the target compiler. The latter
        # is not detectable as this .prf file is evaluated against the target configuration
        # and therefore checking for "mingw" won't work when the target compiler is clang (Android)
        # or qcc (QNX).
        if(MSVC)
            if(NOT LLVM_INSTALL_DIR)
                set(LLVM_INSTALL_DIR "$ENV{LLVM_INSTALL_DIR_MSVC}")
            endif()
        else()
            if(NOT LLVM_INSTALL_DIR)
                set(LLVM_INSTALL_DIR "$ENV{LLVM_INSTALL_DIR_MINGW}")
            endif()
        endif()
    endif()

    if(NOT LLVM_INSTALL_DIR)
        set(LLVM_INSTALL_DIR "$ENV{LLVM_INSTALL_DIR}")
    endif()

    # Assume libclang is installed on the target system
    if(NOT LLVM_INSTALL_DIR)
        set(llvm_config_candidates
            llvm-config-9
            llvm-config-8
            llvm-config-7
            llvm-config-6.0
            llvm-config-5.0
            llvm-config-4.0
            llvm-config-3.9
            llvm-config)
        foreach(candidate ${llvm_config_candidates})
            execute_process(
                COMMAND ${candidate} --prefix
                OUTPUT_VARIABLE LLVM_INSTALL_DIR
                OUTPUT_STRIP_TRAILING_WHITESPACE)
            if(LLVM_INSTALL_DIR)
                execute_process(
                    COMMAND ${candidate} --includedir
                    OUTPUT_VARIABLE CLANG_INCLUDE_PATH
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
                set(LIBCLANG_MAIN_HEADER "${CLANG_INCLUDE_PATH}/clang-c/Index.h")
                if(NOT EXISTS "${LIBCLANG_MAIN_HEADER}")
                    if(LLVM_INSTALL_DIR)
                        message(STATUS "Cannot find libclang's main header file, "
                                       "candidate: ${LIBCLANG_MAIN_HEADER}.")
                        continue()
                    endif()
                else()
                    message(STATUS "QDoc: "
                        "Using Clang installation found in ${LLVM_INSTALL_DIR}. "
                        "Set the LLVM_INSTALL_DIR environment variable to override.")
                    break()
                endif()
            endif()
        endforeach()
    endif()
    if(EXISTS "${LLVM_INSTALL_DIR}")
       get_filename_component(LLVM_INSTALL_DIR "${LLVM_INSTALL_DIR}" ABSOLUTE)
    endif()

    if(TEST_architecture_arch MATCHES "x86_64")
        set(replace_value "64")
    else()
        set(replace_value "32")
    endif()
    string(REPLACE "_ARCH_" "${replace_value}" clang_install_dir "${LLVM_INSTALL_DIR}")

    if(NOT LLVM_INSTALL_DIR)
        if(WIN32)
            return()
        endif()
        if(APPLE)
            # Default to homebrew llvm on macOS. The CLANG_VERSION test below will complain if
            # missing.
            execute_process(
                COMMAND brew --prefix llvm
                OUTPUT_VARIABLE clang_install_dir
                OUTPUT_STRIP_TRAILING_WHITESPACE)
        else()
            set(clang_install_dir "/usr")
        endif()
    endif()

    # note: llvm_config only exits on unix
    set(llvm_config "${clang_install_dir}/bin/llvm-config")
    if(EXISTS "${llvm_config}")
        execute_process(
            COMMAND "${llvm_config}" --libdir
            OUTPUT_VARIABLE QT_LIB_CLANG_LIBDIR
            OUTPUT_STRIP_TRAILING_WHITESPACE)
        execute_process(
            COMMAND "${llvm_config}" --includedir
            OUTPUT_VARIABLE QT_LIB_CLANG_INCLUDEPATH
            OUTPUT_STRIP_TRAILING_WHITESPACE)
        execute_process(
            COMMAND "${llvm_config}" --version
            OUTPUT_VARIABLE QT_LIB_CLANG_VERSION
            OUTPUT_STRIP_TRAILING_WHITESPACE)
    else()
        set(QT_LIB_CLANG_LIBDIR "${clang_install_dir}/lib")
        set(QT_LIB_CLANG_INCLUDEPATH "${clang_install_dir}/include")
        qt_tools_find_llvm_version_from_lib_dir("${QT_LIB_CLANG_LIBDIR}" QT_LIB_CLANG_VERSION)
    endif()

    if(NOT QT_LIB_CLANG_VERSION AND LLVM_INSTALL_DIR)
        message(STATUS "Cannot determine version of clang installation in ${clang_install_dir}.")
        return()
    endif()

    if(QT_LIB_CLANG_VERSION VERSION_LESS "3.9.0")
        message(STATUS "LLVM/Clang version >= 3.9.0 required, version provided: ${QT_LIB_CLANG_VERSION}.")
        return()
    endif()

    set(LIBCLANG_MAIN_HEADER "${QT_LIB_CLANG_INCLUDEPATH}/clang-c/Index.h")
    if(NOT EXISTS "${LIBCLANG_MAIN_HEADER}" AND LLVM_INSTALL_DIR)
        message(STATUS "Cannot find libclang's main header file, "
                       "candidate: ${LIBCLANG_MAIN_HEADER}.")
        return()
    endif()

    # FIXME: What's the use case and how to handle it properly? Aka when would the default libdirs
    # contain already a flag pointing to clang libdir.
    # !contains(QMAKE_DEFAULT_LIBDIRS, $$CLANG_LIBDIR): CLANG_LIBS = -L$${CLANG_LIBDIR}
    set(QT_LIB_CLANG_LIBS "")
    set(QT_LIB_CLANG_DEFINES "")

    if(NOT QDOC_USE_STATIC_LIBCLANG)
        if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
            list(APPEND QT_LIB_CLANG_LIBS -llibclang -ladvapi32 -lshell32)
        else()
            list(APPEND QT_LIB_CLANG_LIBS -lclang)
        endif()
    else()
        if(MSVC)
            list(APPEND QT_LIB_CLANG_DEFINES "CINDEX_LINKAGE=")
            list(APPEND QT_LIB_CLANG_LIBS -llibclang_static -ladvapi32 -lshell32 -lMincore)
        else()
            if(NOT CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
                list(APPEND QT_LIB_CLANG_LIBS -Wl,--start-group)
            endif()
            qt_tools_get_flag_list_of_llvm_static_libs(llvm_static_libs)
            list(APPEND QT_LIB_CLANG_LIBS ${llvm_static_libs})
            if(NOT CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
                list(APPEND QT_LIB_CLANG_LIBS -Wl,--end-group)
            endif()
            list(APPEND QT_LIB_CLANG_LIBS -lz)
            if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
                list(APPEND QT_LIB_CLANG_LIBS -lpsapi -lshell32 -lole32 -luuid -ladvapi32 -lversion)
            else()
                list(APPEND QT_LIB_CLANG_LIBS -ldl)
            endif()
            if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
                list(APPEND QT_LIB_CLANG_LIBS -lcurses -lm -lxml2)
            endif()
        endif() # MSVC
    endif() # QDOC_USE_STATIC_LIBCLANG

    qt_tools_create_lib_clang_target()
    set(QT_LIB_CLANG_LIBS "${QT_LIB_CLANG_LIBS}" CACHE STRING "" FORCE)
    set(QT_LIB_CLANG_LIBDIR "${QT_LIB_CLANG_LIBDIR}" CACHE STRING "" FORCE)
    set(QT_LIB_CLANG_INCLUDEPATH "${QT_LIB_CLANG_INCLUDEPATH}" CACHE STRING "" FORCE)
    set(QT_LIB_CLANG_DEFINES "${QT_LIB_CLANG_DEFINES}" CACHE STRING "" FORCE)
    set(QT_LIB_CLANG_VERSION "${QT_LIB_CLANG_VERSION}" CACHE STRING "" FORCE)
    set(QT_LIBCLANG_RESOURCE_DIR "\"${QT_LIB_CLANG_LIBDIR}/clang/${QT_LIB_CLANG_VERSION}/include\""
        CACHE STRING "Qt libclang resource dir.")
    set(WrapLibClang_FOUND TRUE PARENT_SCOPE)
endfunction()

function(qt_tools_create_lib_clang_target)
    add_library(WrapLibClang::WrapLibClang INTERFACE IMPORTED)
    target_link_libraries(WrapLibClang::WrapLibClang INTERFACE ${QT_LIB_CLANG_LIBS})
    target_link_directories(WrapLibClang::WrapLibClang INTERFACE ${QT_LIB_CLANG_LIBDIR})
    target_include_directories(WrapLibClang::WrapLibClang INTERFACE ${QT_LIB_CLANG_INCLUDEPATH})
    target_compile_definitions(WrapLibClang::WrapLibClang INTERFACE ${QT_LIB_CLANG_DEFINES})
endfunction()

function(qt_tools_get_flag_list_of_llvm_static_libs out_var)
    set(${out_var}
        -lclangAnalysis
        -lclangApplyReplacements
        -lclangARCMigrate
        -lclangAST
        -lclangASTMatchers
        -lclangBasic
        -lclangChangeNamespace
        -lclangCodeGen
        -lclangCrossTU
        -lclangDaemon
        -lclangDriver
        -lclangDynamicASTMatchers
        -lclangEdit
        -lclangFormat
        -lclangFrontend
        -lclangFrontendTool
        -lclangHandleCXX
        -lclangIncludeFixer
        -lclangIncludeFixerPlugin
        -lclangIndex
        -lclangLex
        -lclangMove
        -lclangParse
        -lclangQuery
        -lclangReorderFields
        -lclangRewrite
        -lclangRewriteFrontend
        -lclangSema
        -lclangSerialization
        -lclang_static
        -lclangStaticAnalyzerCheckers
        -lclangStaticAnalyzerCore
        -lclangStaticAnalyzerFrontend
        -lclangTidy
        -lclangTidyAndroidModule
        -lclangTidyBoostModule
        -lclangTidyBugproneModule
        -lclangTidyCERTModule
        -lclangTidyCppCoreGuidelinesModule
        -lclangTidyFuchsiaModule
        -lclangTidyGoogleModule
        -lclangTidyHICPPModule
        -lclangTidyLLVMModule
        -lclangTidyMiscModule
        -lclangTidyModernizeModule
        -lclangTidyMPIModule
        -lclangTidyObjCModule
        -lclangTidyPerformanceModule
        -lclangTidyPlugin
        -lclangTidyReadabilityModule
        -lclangTidyUtils
        -lclangTooling
        -lclangToolingASTDiff
        -lclangToolingCore
        -lclangToolingRefactor
        -lfindAllSymbols
        -lLLVMAArch64AsmParser
        -lLLVMAArch64AsmPrinter
        -lLLVMAArch64CodeGen
        -lLLVMAArch64Desc
        -lLLVMAArch64Disassembler
        -lLLVMAArch64Info
        -lLLVMAArch64Utils
        -lLLVMAMDGPUAsmParser
        -lLLVMAMDGPUAsmPrinter
        -lLLVMAMDGPUCodeGen
        -lLLVMAMDGPUDesc
        -lLLVMAMDGPUDisassembler
        -lLLVMAMDGPUInfo
        -lLLVMAMDGPUUtils
        -lLLVMAnalysis
        -lLLVMARMAsmParser
        -lLLVMARMAsmPrinter
        -lLLVMARMCodeGen
        -lLLVMARMDesc
        -lLLVMARMDisassembler
        -lLLVMARMInfo
        -lLLVMARMUtils
        -lLLVMAsmParser
        -lLLVMAsmPrinter
        -lLLVMBinaryFormat
        -lLLVMBitReader
        -lLLVMBitWriter
        -lLLVMBPFAsmParser
        -lLLVMBPFAsmPrinter
        -lLLVMBPFCodeGen
        -lLLVMBPFDesc
        -lLLVMBPFDisassembler
        -lLLVMBPFInfo
        -lLLVMCodeGen
        -lLLVMCore
        -lLLVMCoroutines
        -lLLVMCoverage
        -lLLVMDebugInfoCodeView
        -lLLVMDebugInfoDWARF
        -lLLVMDebugInfoMSF
        -lLLVMDebugInfoPDB
        -lLLVMDemangle
        -lLLVMDlltoolDriver
        -lLLVMExecutionEngine
        -lLLVMFuzzMutate
        -lLLVMGlobalISel
        -lLLVMHexagonAsmParser
        -lLLVMHexagonCodeGen
        -lLLVMHexagonDesc
        -lLLVMHexagonDisassembler
        -lLLVMHexagonInfo
        -lLLVMInstCombine
        -lLLVMInstrumentation
        -lLLVMInterpreter
        -lLLVMipo
        -lLLVMIRReader
        -lLLVMLanaiAsmParser
        -lLLVMLanaiAsmPrinter
        -lLLVMLanaiCodeGen
        -lLLVMLanaiDesc
        -lLLVMLanaiDisassembler
        -lLLVMLanaiInfo
        -lLLVMLibDriver
        -lLLVMLineEditor
        -lLLVMLinker
        -lLLVMLTO
        -lLLVMMC
        -lLLVMMCDisassembler
        -lLLVMMCJIT
        -lLLVMMCParser
        -lLLVMMipsAsmParser
        -lLLVMMipsAsmPrinter
        -lLLVMMipsCodeGen
        -lLLVMMipsDesc
        -lLLVMMipsDisassembler
        -lLLVMMipsInfo
        -lLLVMMIRParser
        -lLLVMMSP430AsmPrinter
        -lLLVMMSP430CodeGen
        -lLLVMMSP430Desc
        -lLLVMMSP430Info
        -lLLVMNVPTXAsmPrinter
        -lLLVMNVPTXCodeGen
        -lLLVMNVPTXDesc
        -lLLVMNVPTXInfo
        -lLLVMObjCARCOpts
        -lLLVMObject
        -lLLVMObjectYAML
        -lLLVMOption
        -lLLVMOrcJIT
        -lLLVMPasses
        -lLLVMPowerPCAsmParser
        -lLLVMPowerPCAsmPrinter
        -lLLVMPowerPCCodeGen
        -lLLVMPowerPCDesc
        -lLLVMPowerPCDisassembler
        -lLLVMPowerPCInfo
        -lLLVMProfileData
        -lLLVMRuntimeDyld
        -lLLVMScalarOpts
        -lLLVMSelectionDAG
        -lLLVMSparcAsmParser
        -lLLVMSparcAsmPrinter
        -lLLVMSparcCodeGen
        -lLLVMSparcDesc
        -lLLVMSparcDisassembler
        -lLLVMSparcInfo
        -lLLVMSupport
        -lLLVMSymbolize
        -lLLVMSystemZAsmParser
        -lLLVMSystemZAsmPrinter
        -lLLVMSystemZCodeGen
        -lLLVMSystemZDesc
        -lLLVMSystemZDisassembler
        -lLLVMSystemZInfo
        -lLLVMTableGen
        -lLLVMTarget
        -lLLVMTransformUtils
        -lLLVMVectorize
        -lLLVMWindowsManifest
        -lLLVMX86AsmParser
        -lLLVMX86AsmPrinter
        -lLLVMX86CodeGen
        -lLLVMX86Desc
        -lLLVMX86Disassembler
        -lLLVMX86Info
        -lLLVMX86Utils
        -lLLVMXCoreAsmPrinter
        -lLLVMXCoreCodeGen
        -lLLVMXCoreDesc
        -lLLVMXCoreDisassembler
        -lLLVMXCoreInfo
        -lLLVMXRay
    PARENT_SCOPE)
endfunction()

# Tries to find libclang. If successful, creates an imported target called
# WrapLibClang::WrapLibClang.
qt_tools_find_lib_clang()
