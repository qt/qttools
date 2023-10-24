# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial

if(TARGET Qt::WebKitWidgets)
    add_subdirectory(qwebview)
endif()
if(TARGET Qt::AxContainer AND WIN32)
    add_subdirectory(activeqt)
endif()
if(TARGET Qt::QuickWidgets)
    add_subdirectory(qquickwidget)
endif()
