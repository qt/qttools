// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0


























// IMPORTANT!!!! If you want to add testdata to this file,
// always add it to the end in order to not change the linenumbers of translations!!!
//
//
//
//

#include <QCoreApplication>

bool qt_detectRTLLanguage()
{
     return QCoreApplication::tr("QT_LAYOUT_DIRECTION",
                         "Translate this string to the string 'LTR' in left-to-right"
                         " languages or to 'RTL' in right-to-left languages (such as Hebrew"
                         " and Arabic) to get proper widget layout.") == QLatin1String("RTL");
}
