/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
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

#include "utilities.h"

#include <QtTest/QtTest>

QT_BEGIN_NAMESPACE
Q_LOGGING_CATEGORY(lcQdoc, "qt.test")
QT_END_NAMESPACE

class tst_Utilities : public QObject
{
    Q_OBJECT

private slots:
    void loggingCategoryName();
    void loggingCategoryDefaults();
    void startDebugging();
    void stopDebugging();
    void debugging();
};

void tst_Utilities::loggingCategoryName()
{
    const QString expected = "qt.test";
    QCOMPARE(lcQdoc().categoryName(), expected);
}

void tst_Utilities::loggingCategoryDefaults()
{
    QVERIFY(lcQdoc().isCriticalEnabled());
    QVERIFY(lcQdoc().isWarningEnabled());
    QVERIFY(!lcQdoc().isDebugEnabled());
    QVERIFY(lcQdoc().isInfoEnabled());
}

void tst_Utilities::startDebugging()
{
    QVERIFY(!lcQdoc().isDebugEnabled());
    Utilities::startDebugging("test");
    QVERIFY(lcQdoc().isDebugEnabled());
}

void tst_Utilities::stopDebugging()
{
    Utilities::startDebugging("test");
    QVERIFY(lcQdoc().isDebugEnabled());
    Utilities::stopDebugging("test");
    QVERIFY(!lcQdoc().isDebugEnabled());
}

void tst_Utilities::debugging()
{
    QVERIFY(!lcQdoc().isDebugEnabled());
    QVERIFY(!Utilities::debugging());
    Utilities::startDebugging("test");
    QVERIFY(lcQdoc().isDebugEnabled());
    QVERIFY(Utilities::debugging());
}

QTEST_APPLESS_MAIN(tst_Utilities)

#include "tst_utilities.moc"
