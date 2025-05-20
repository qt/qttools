# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

qt_internal_add_example(arrowpad)
qt_internal_add_example(hellotr)
qt_internal_add_example(i18n)
qt_internal_add_example(trollprint)
if(TARGET Qt6::Quick)
    qt_internal_add_example(localizedclock)
    qt_internal_add_example(localizedclock-switchlocale)
endif()
