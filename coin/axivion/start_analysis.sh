#!/bin/bash
# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

$HOME/bauhaus-suite/setup.sh --non-interactive
export PATH=/home/qt/bauhaus-suite/bin:$PATH
export BAUHAUS_CONFIG=$(cd $(dirname $(readlink -f $0)) && pwd)
export AXIVION_VERSION_NAME=$(git rev-parse HEAD)
export EXCLUDE_FILES="build/*:src/3rdparty/*"
export CAFECC_BASEPATH="/home/qt/work/qt/$TESTED_MODULE_COIN"
gccsetup --cc gcc --cxx g++ --config "$BAUHAUS_CONFIG"
cd "$CAFECC_BASEPATH"
BAUHAUS_IR_COMPRESSION=none COMPILE_ONLY=1 cmake -G Ninja -DAXIVION_ANALYSIS_TOOLCHAIN_FILE=/home/qt/bauhaus-suite/profiles/cmake/axivion-launcher-toolchain.cmake -DCMAKE_PREFIX_PATH=/home/qt/work/qt/qttools/build -DCMAKE_PROJECT_INCLUDE_BEFORE=/home/qt/bauhaus-suite/profiles/cmake/axivion-before-project-hook.cmake -B build -S . --fresh
cmake --build build -j4

ROOT_DIR=src
MAGIC="Qt-Security score:critical"

INCLUDE_FILES=""

while IFS= read -r file; do
    if [ -z "$INCLUDE_FILES" ]; then
        INCLUDE_FILES="$file"
    else
        INCLUDE_FILES="$INCLUDE_FILES:$file"
    fi
done <<EOF
$(grep -rl "$MAGIC" "$ROOT_DIR")
EOF

if [ -z "$INCLUDE_FILES" ]; then
    echo "No Security Critical files found; running analysis with an explicit fallback sentinel."
    INCLUDE_FILES="__NO_FILES_FOUND__"
fi

export INCLUDE_FILES

# qtassistant/qtdesigner picked up their plugins (previously uncovered);
# linguist picked up the lupdate/lrelease/lconvert/lcheck/ltext2id tool family
# (previously uncovered); qtdoctools and qttoolsmisc are new modules for
# binaries that had no coverage at all before. Modules that are HEADER_MODULE
# in CMake (Linguist, UiPlugin, Tools, QDocCatchConversionsPrivate,
# QDocCatchGeneratorsPrivate) produce no binary and so have nothing to add.
for MODULE in qtassistant qtdesigner qthelp qtuitools linguist qtdoctools qttoolsmisc; do
    export MODULE
    export PLUGINS=""
    export IRNAME=build/$MODULE.ir
    if [ "$MODULE" == "qtassistant" ]
    then
        TARGET_NAME="build/bin/assistant.ir"
        TARGET_NAME="$TARGET_NAME:build/bin/qhelpgenerator.ir"
        TARGET_NAME="$TARGET_NAME:build/plugins/help/lib*.so*.ir"
        export TARGET_NAME
        export PACKAGE="Add-ons"
    elif [ "$MODULE" == "qtdesigner" ]
    then
        TARGET_NAME="build/bin/designer.ir"
        TARGET_NAME="$TARGET_NAME:build/lib/libQt6Designer.so.*.ir"
        TARGET_NAME="$TARGET_NAME:build/lib/libQt6DesignerComponentsPrivate.so.*.ir"
        TARGET_NAME="$TARGET_NAME:build/plugins/designer/lib*.so*.ir"
        export TARGET_NAME
        export PACKAGE="Add-ons"
    elif [ "$MODULE" == "qthelp" ]
    then
        export TARGET_NAME="build/lib/libQt6Help.so.*.ir"
        export PACKAGE="Add-ons"
    elif [ "$MODULE" == "qtuitools" ]
    then
        export TARGET_NAME="build/lib/libQt6UiTools.so.*.ir"
        export PACKAGE="Add-ons"
    elif [ "$MODULE" == "linguist" ]
    then
        TARGET_NAME="build/bin/linguist.ir"
        TARGET_NAME="$TARGET_NAME:build/bin/lupdate.ir"
        TARGET_NAME="$TARGET_NAME:build/bin/lupdate-pro.ir"
        TARGET_NAME="$TARGET_NAME:build/bin/lrelease.ir"
        TARGET_NAME="$TARGET_NAME:build/bin/lrelease-pro.ir"
        TARGET_NAME="$TARGET_NAME:build/bin/lconvert.ir"
        TARGET_NAME="$TARGET_NAME:build/bin/lcheck.ir"
        TARGET_NAME="$TARGET_NAME:build/bin/ltext2id.ir"
        export TARGET_NAME
        export PACKAGE="Add-ons"
    elif [ "$MODULE" == "qtdoctools" ]
    then
        # qdoc and its supporting command-line tools
        TARGET_NAME="build/bin/qdoc.ir"
        TARGET_NAME="$TARGET_NAME:build/bin/qtattributionsscanner.ir"
        TARGET_NAME="$TARGET_NAME:build/bin/objcnamemangler.ir"
        export TARGET_NAME
        export PACKAGE="Add-ons"
    elif [ "$MODULE" == "qttoolsmisc" ]
    then
        # Standalone qttools app binaries with no better home
        TARGET_NAME="build/bin/qtdiag.ir"
        TARGET_NAME="$TARGET_NAME:build/bin/pixeltool.ir"
        TARGET_NAME="$TARGET_NAME:build/bin/qdbus.ir"
        TARGET_NAME="$TARGET_NAME:build/bin/qdbusviewer.ir"
        TARGET_NAME="$TARGET_NAME:build/bin/kmap2qmap.ir"
        TARGET_NAME="$TARGET_NAME:build/bin/qev.ir"
        TARGET_NAME="$TARGET_NAME:build/bin/qdistancefieldgenerator.ir"
        TARGET_NAME="$TARGET_NAME:build/bin/qtplugininfo.ir"
        export TARGET_NAME
        export PACKAGE="Add-ons"
    fi
    axivion_ci "$@"
done
