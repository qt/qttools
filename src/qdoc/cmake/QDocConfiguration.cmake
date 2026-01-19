# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# QDoc-specific configuration variables

# Minimum supported Clang version for QDoc
set(QDOC_MINIMUM_CLANG_VERSION "17")

# List of explicitly supported Clang versions for QDoc
set(QDOC_SUPPORTED_CLANG_VERSIONS
    "21.1" "20.1" "19.1" "18.1" "17.0.6"
)

# Check if user explicitly disabled QDoc via -no-feature-qdoc
# When TRUE, QDoc dependency warnings should be suppressed
set(QDOC_EXPLICITLY_DISABLED FALSE)
if(DEFINED FEATURE_qdoc AND NOT FEATURE_qdoc)
    set(QDOC_EXPLICITLY_DISABLED TRUE)
endif()

