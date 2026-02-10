# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#[[
    qt6_mangle_objc_symbols(<target>
        (NAMESPACE <namespace> | QT_NAMESPACE)
        (NAMESPACE_REPLACEMENT <replacement> | GENERATE_RANDOM_NAMESPACE)
        [EXCLUDE_CLASSES <class>...]
        [QUIET]
        [CODESIGN]
        [CODESIGN_IDENTITY <identity>]
    )

    Mangles Objective-C class and category names in the target binary by replacing
    the specified namespace with either a provided replacement string or a unique
    random string of the same length.

    Arguments:
    - target: The target whose binary will be patched
    - NAMESPACE: The namespace prefix to replace (e.g., "QtCore", "MyNamespace")
    - QT_NAMESPACE: Use the Qt namespace from Qt::Core's QT_NAMESPACE property
    - NAMESPACE_REPLACEMENT: The replacement string. With NAMESPACE mode, must be
                             same length as NAMESPACE. With QT_NAMESPACE mode, will be
                             padded with underscores if shorter or truncated if longer.
    - GENERATE_RANDOM_NAMESPACE: If specified, generates a random replacement string
    - EXCLUDE_CLASSES: List of class names to exclude from mangling
                       (can be specified multiple times)
    - QUIET: Suppress output from the mangler tool
    - CODESIGN: Re-sign the binary after mangling (macOS only)
    - CODESIGN_IDENTITY: Code signing identity to use
                         (default: "-" for ad-hoc signing)

    Example usage:
        # With explicit replacement:
        qt6_mangle_objc_symbols(MyApp
            NAMESPACE "QtCore"
            NAMESPACE_REPLACEMENT "QTCore"
            CODESIGN
        )

        # With random namespace:
        qt6_mangle_objc_symbols(MyApp
            NAMESPACE "QtCore"
            GENERATE_RANDOM_NAMESPACE
            CODESIGN
        )

        # With excluded classes:
        qt6_mangle_objc_symbols(MyApp
            NAMESPACE "QtCore"
            NAMESPACE_REPLACEMENT "QTCore"
            EXCLUDE_CLASSES QtCore_Internal QtCore_Private
            CODESIGN
        )

        # Using Qt namespace from Qt::Core:
        qt6_mangle_objc_symbols(MyApp
            QT_NAMESPACE
            NAMESPACE_REPLACEMENT "MyNS"
            CODESIGN
        )

    This function adds a POST_BUILD step to the target. Note that this step
    potentially invalidates code signatures, so custom codesigning needs to be
    installed after the name mangling (e.g., using the CODESIGN option). The
    POST_BUILD step:
    1. Mangles Objective-C symbols by replacing the namespace
    2. Optionally re-signs the binary if CODESIGN is specified

    Note: The replacement string must be the same length as the original
    namespace for binary safety. When GENERATE_RANDOM_NAMESPACE is used, a random
    string of the same length is automatically generated. When using QT_NAMESPACE
    with NAMESPACE_REPLACEMENT, if the replacement is shorter than the Qt namespace,
    it will be padded with underscores; if longer, a warning will be issued and the
    replacement will be truncated.
#]]
function(qt6_mangle_objc_symbols target)
    set(options
        GENERATE_RANDOM_NAMESPACE
        QUIET
        CODESIGN
        QT_NAMESPACE
    )
    set(oneValueArgs
        NAMESPACE
        NAMESPACE_REPLACEMENT
        CODESIGN_IDENTITY
    )
    set(multiValueArgs
        EXCLUDE_CLASSES
    )

    cmake_parse_arguments(PARSE_ARGV 1 arg "${options}" "${oneValueArgs}"
                          "${multiValueArgs}")

    if(NOT arg_NAMESPACE AND NOT arg_QT_NAMESPACE)
        message(FATAL_ERROR
            "qt6_mangle_objc_symbols: Either NAMESPACE or QT_NAMESPACE argument "
            "is required")
    endif()

    if(arg_NAMESPACE AND arg_QT_NAMESPACE)
        message(FATAL_ERROR
            "qt6_mangle_objc_symbols: NAMESPACE and QT_NAMESPACE are mutually "
            "exclusive")
    endif()

    if(NOT TARGET ${target})
        message(FATAL_ERROR
            "qt6_mangle_objc_symbols: ${target} is not a valid target")
    endif()

    # Handle QT_NAMESPACE mode
    if(arg_QT_NAMESPACE)
        if(NOT TARGET ${QT_CMAKE_EXPORT_NAMESPACE}::Core)
            message(FATAL_ERROR
                "qt6_mangle_objc_symbols: QT_NAMESPACE mode requires Qt::Core "
                "target to be available")
        endif()

        get_target_property(qt_namespace ${QT_CMAKE_EXPORT_NAMESPACE}::Core
                            QT_NAMESPACE)
        if(NOT qt_namespace OR qt_namespace STREQUAL "qt_namespace-NOTFOUND")
            message(FATAL_ERROR
                "qt6_mangle_objc_symbols: Could not determine Qt namespace from "
                "Qt::Core. QT_NAMESPACE property is not set.")
        endif()

        set(arg_NAMESPACE "${qt_namespace}")
    endif()

    # Check that either NAMESPACE_REPLACEMENT or GENERATE_RANDOM_NAMESPACE
    # is specified
    if(NOT arg_NAMESPACE_REPLACEMENT AND NOT arg_GENERATE_RANDOM_NAMESPACE)
        message(FATAL_ERROR
            "qt6_mangle_objc_symbols: Either NAMESPACE_REPLACEMENT or "
            "GENERATE_RANDOM_NAMESPACE must be specified")
    endif()

    if(arg_NAMESPACE_REPLACEMENT AND arg_GENERATE_RANDOM_NAMESPACE)
        message(FATAL_ERROR
            "qt6_mangle_objc_symbols: NAMESPACE_REPLACEMENT and "
            "GENERATE_RANDOM_NAMESPACE are mutually exclusive")
    endif()

    # Check if we're building on a platform that supports Objective-C mangling
    # (macOS/iOS)
    if(NOT APPLE)
        if(NOT arg_QUIET)
            message(WARNING
                "qt6_mangle_objc_symbols: Objective-C mangling is only supported "
                "on Apple platforms. Skipping.")
        endif()
        return()
    endif()


    # Build the command arguments
    set(mangler_args "")

    if(arg_QUIET)
        list(APPEND mangler_args --quiet)
    endif()

    # Add excluded classes
    if(arg_EXCLUDE_CLASSES)
        foreach(excluded_class IN LISTS arg_EXCLUDE_CLASSES)
            list(APPEND mangler_args --exclude "${excluded_class}")
        endforeach()
    endif()

    # Determine the replacement string
    if(arg_GENERATE_RANDOM_NAMESPACE)
        # Generate a random string of the same length as the namespace
        string(LENGTH "${arg_NAMESPACE}" namespace_len)
        string(RANDOM LENGTH ${namespace_len}
               ALPHABET
               "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
               replacement_namespace)

        set(comment_suffix "with random namespace '${replacement_namespace}'")
    else()
        set(replacement_namespace "${arg_NAMESPACE_REPLACEMENT}")

        # Validate that replacement has the same length as namespace
        string(LENGTH "${arg_NAMESPACE}" namespace_len)
        string(LENGTH "${replacement_namespace}" replacement_len)

        if(arg_QT_NAMESPACE)
            # In QT_NAMESPACE mode, pad with underscores if shorter, warn if
            # longer
            if(replacement_len LESS namespace_len)
                string(REPEAT "_" ${namespace_len} padding)
                string(SUBSTRING "${padding}" ${replacement_len} -1
                       needed_padding)
                set(replacement_namespace
                    "${replacement_namespace}${needed_padding}")
                string(LENGTH "${replacement_namespace}" replacement_len)
            elseif(replacement_len GREATER namespace_len)
                message(WARNING
                    "qt6_mangle_objc_symbols: NAMESPACE_REPLACEMENT "
                    "'${replacement_namespace}' is longer than Qt namespace "
                    "'${arg_NAMESPACE}' (${replacement_len} > ${namespace_len}). "
                    "Truncating to match length.")
                string(SUBSTRING "${replacement_namespace}" 0 ${namespace_len}
                       replacement_namespace)
                string(LENGTH "${replacement_namespace}" replacement_len)
            endif()
        else()
            # In manual NAMESPACE mode, require exact length match
            if(NOT namespace_len EQUAL replacement_len)
                message(FATAL_ERROR
                    "qt6_mangle_objc_symbols: NAMESPACE_REPLACEMENT "
                    "'${replacement_namespace}' must be the same length as "
                    "NAMESPACE '${arg_NAMESPACE}' "
                    "(${replacement_len} != ${namespace_len})")
            endif()
        endif()

        set(comment_suffix "with namespace '${replacement_namespace}'")
    endif()

    list(APPEND mangler_args --replace "${arg_NAMESPACE}"
         "${replacement_namespace}")

    # Build the full command
    set(commands "")
    set(target_file "$<TARGET_FILE:${target}>")

    list(APPEND commands
        COMMAND "$<TARGET_FILE:${QT_CMAKE_EXPORT_NAMESPACE}::objcnamemangler>"
                ${mangler_args}
                "${target_file}"
    )

    if(arg_CODESIGN)
        if(NOT arg_CODESIGN_IDENTITY)
            set(arg_CODESIGN_IDENTITY "-")
        endif()

        list(APPEND commands
            COMMAND codesign --force --sign "${arg_CODESIGN_IDENTITY}"
                    "${target_file}"
        )
        set(comment_suffix "${comment_suffix}, then re-signing")
    endif()

    add_custom_command(TARGET ${target} POST_BUILD
        ${commands}
        VERBATIM
        COMMENT "Mangling Objective-C symbols in namespace '${arg_NAMESPACE}' "
                "${comment_suffix}"
    )
endfunction()

if(NOT QT_NO_CREATE_VERSIONLESS_FUNCTIONS)
    #[[
        qt_mangle_objc_symbols(<target>
            (NAMESPACE <namespace> | QT_NAMESPACE)
            (NAMESPACE_REPLACEMENT <replacement> | GENERATE_RANDOM_NAMESPACE)
            [EXCLUDE_CLASSES <class>...]
            [QUIET]
            [CODESIGN]
            [CODESIGN_IDENTITY <identity>]
        )

        This is a versionless wrapper for qt6_mangle_objc_symbols().
        See qt6_mangle_objc_symbols() for detailed documentation.
    #]]
    function(qt_mangle_objc_symbols target)
        if(QT_DEFAULT_MAJOR_VERSION EQUAL 6)
            qt6_mangle_objc_symbols(${target} ${ARGN})
        else()
            message(FATAL_ERROR
                "qt_mangle_objc_symbols() is only available in Qt 6.")
        endif()
    endfunction()
endif()
