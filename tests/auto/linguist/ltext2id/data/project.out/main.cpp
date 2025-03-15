// Copyright (C) 2025 The Qt Company Ltd
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QObject>

//% "translation"
//@ test
const char* text = QT_TRID_NOOP("test-translation");
static { const char *source; const char *comment; } = QT_TRANSLATE_NOOP3("test", "translation2 with comment", "comment");

//% "only macro"
//@ test
const char* onlyMacro1 = QT_TRID_NOOP("test-onlyamacro");
//% "only macro"
//@ test
const char* onlyMacro2 = QT_TRID_NOOP("test-onlyamacro");

class test {
    Q_OBJECT
    void func ()
    {
        //% "translation"
        //@ test
        qtTrId("test-translation");
        //% "translation2 with comment"
        //@ test
        qtTrId("test-translation2awitha-45525");
        //% "id based source text"
        qtTrId("existing-id-based");

        //% "conflict with existing id"
        //@ test
        qtTrId("test-conflictawitha-54521_2");

        //% "conflict with existing id"
        qtTrId("test-conflictawitha-54521");

        //% "a source text"
        //@ test
        qtTrId("test-aasourcea-57463");
    }
};
