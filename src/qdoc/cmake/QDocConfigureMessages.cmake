# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Platform-specific warning messages for missing LLVM/Clang dependencies.
function(qdoc_generate_clang_warning_message OUT_VAR)
    if(WIN32)
        set(PLATFORM_GUIDANCE "
On Windows:
- Use Qt's prebuilt LLVM packages:
  https://download.qt.io/development_releases/prebuilt/libclang/qt/
- Extract and set LLVM_INSTALL_DIR to the installation path
- Note: These packages only support Release builds")
    elseif(APPLE)
        set(PLATFORM_GUIDANCE "
On macOS:
- Recommended: Use Qt's prebuilt LLVM packages:
  https://download.qt.io/development_releases/prebuilt/libclang/qt/
- Alternative: Install via Homebrew: `brew install llvm`
  * ARM64: `configure LLVM_INSTALL_DIR=/opt/homebrew/opt/llvm FEATURE_clang=ON`
  * x86_64: `configure LLVM_INSTALL_DIR=/usr/local/opt/llvm FEATURE_clang=ON`")
    elseif(UNIX)
        set(PLATFORM_GUIDANCE "
On Linux:
- Recommended: Use Qt's prebuilt LLVM packages:
  https://download.qt.io/development_releases/prebuilt/libclang/qt/
- Alternative system packages:
  * Debian/Ubuntu: `sudo apt install libclang-dev`
  * Fedora/RHEL: `sudo dnf install clang-devel`
  * ArchLinux: `sudo pacman -S clang llvm`")
    endif()

    set(${OUT_VAR} "
QDoc will not be compiled because required LLVM/Clang libraries could not be located.
This affects Qt documentation generation and Clang-based lupdate parsing.

${PLATFORM_GUIDANCE}

After installation, reconfigure with FEATURE_clang=ON.

For detailed setup instructions: https://doc.qt.io/qt/qdoc-guide-clang.html
" PARENT_SCOPE)
endfunction()

# Warning message for missing QmlPrivate dependency.
function(qdoc_generate_qmlprivate_warning_message OUT_VAR)
    set(${OUT_VAR} "QDoc will not be compiled because the QmlPrivate library wasn't found." PARENT_SCOPE)
endfunction()

# Warning message for missing Qt features.
function(qdoc_generate_missing_features_warning_message OUT_VAR)
    set(${OUT_VAR} "QDoc cannot be compiled without Qt's commandline parser or thread features." PARENT_SCOPE)
endfunction()

# Warning message for insufficient Clang version.
function(qdoc_generate_clang_version_warning_message OUT_VAR)
    set(${OUT_VAR} "QDoc will not be compiled because it requires libclang ${QDOC_MINIMUM_CLANG_VERSION} or newer." PARENT_SCOPE)
endfunction()

# Internal helper: Register a QDoc warning with automatic suppression.
# Warnings are suppressed when the user explicitly disabled QDoc via -no-feature-qdoc.
function(_qdoc_add_configure_warning MESSAGE CONDITION)
    qt_configure_add_report_entry(
        TYPE WARNING
        MESSAGE "${MESSAGE}"
        CONDITION (${CONDITION}) AND NOT QDOC_EXPLICITLY_DISABLED
    )
endfunction()

# Register all QDoc-related configure warnings.
function(qdoc_register_configure_warnings)
    qdoc_generate_clang_warning_message(_clang_warning)
    qdoc_generate_qmlprivate_warning_message(_qmlprivate_warning)
    qdoc_generate_missing_features_warning_message(_missing_features_warning)
    qdoc_generate_clang_version_warning_message(_clang_version_warning)

    _qdoc_add_configure_warning("${_clang_warning}"
        "NOT QT_FEATURE_clang")
    _qdoc_add_configure_warning("${_qmlprivate_warning}"
        "NOT TARGET Qt::QmlPrivate")
    _qdoc_add_configure_warning("${_missing_features_warning}"
        "NOT QT_FEATURE_commandlineparser OR NOT QT_FEATURE_thread")
    _qdoc_add_configure_warning("${_clang_version_warning}"
        "QT_LIB_CLANG_VERSION VERSION_LESS QDOC_MINIMUM_CLANG_VERSION")
endfunction()
