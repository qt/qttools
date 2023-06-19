/****************************************************************************
**
** Copyright (C) 2023 The Qt Company Ltd.
** Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Stephen Kelly <stephen.kelly@kdab.com>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QCoreApplication>
#include <QTranslator>
#include <QLocale>

#include <cstdio>

using namespace Qt::Literals::StringLiterals;

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QTranslator *translator = new QTranslator();
    QLocale frenchLocale = QLocale(QLocale::French);
    QLocale::setDefault(frenchLocale);
    if (!translator->load(QLocale(), QLatin1String("myobject.qm"), {},
                          qApp->applicationDirPath() + QLatin1String("/fr/"))) {
        qFatal("Could not load translation file!");
    }

    app.installTranslator(translator);

    QString text = QCoreApplication::translate("main", "Hello, world!");
    if (text != QLatin1String("Bonjour le monde!"))
        qFatal("Translation not found!");

    std::fprintf(stdout, "%s\n", qPrintable(text));
    return 0;
}
