#!/bin/bash
# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

for file in darkicons/*.png; do
    if [[ ! $file == *"-disabled.png" ]]; then
        magick $file -fill gray -opaque white -colorize 100% ${file%.png}-disabled.png
    fi
done

for file in lighticons/*.png; do
    if [[ ! $file == *"-disabled.png" ]]; then
        magick $file -fill gray -opaque black -colorize 100% ${file%.png}-disabled.png
    fi
done

magick check.png -fill green -opaque black -colorize 100% darkmarks/on-mark.png
magick check.png -fill gray -opaque black -colorize 100% darkmarks/obsolete-mark.png
magick check.png -fill yellow -opaque black -colorize 100% darkmarks/warning-mark.png
magick menu-dots.png -fill gray -opaque black -colorize 100% darkmarks/empty-mark.png
magick menu-dots.png -fill yellow -opaque black -colorize 100% darkmarks/off-mark.png
magick alert.png -fill red -opaque black -colorize 100% darkmarks/danger-mark.png

magick check.png -fill green -opaque black -colorize 100% lightmarks/on-mark.png
magick check.png -fill gray -opaque black -colorize 100% lightmarks/obsolete-mark.png
magick check.png -fill orange -opaque black -colorize 100% lightmarks/warning-mark.png
magick menu-dots.png -fill gray -opaque black -colorize 100% lightmarks/empty-mark.png
magick menu-dots.png -fill orange -opaque black -colorize 100% lightmarks/off-mark.png
magick alert.png -fill red -opaque black -colorize 100% lightmarks/danger-mark.png
