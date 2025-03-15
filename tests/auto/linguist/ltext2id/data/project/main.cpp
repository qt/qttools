// Copyright (C) 2025 The Qt Company Ltd
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QObject>

const char* text = QT_TRANSLATE_NOOP("test", "translation");
static { const char *source; const char *comment; } = QT_TRANSLATE_NOOP3("test", "translation2 with comment", "comment");

const char* onlyMacro1 = QT_TRANSLATE_NOOP("test", "only macro");
const char* onlyMacro2 = QT_TRANSLATE_NOOP("test", "only macro");

class test {
    Q_OBJECT
    void func ()
    {
        tr("translation");
        tr("translation2 with comment", "comment");
        //% "id based source text"
        qtTrId("existing-id-based");

        tr("conflict with existing id");

        //% "conflict with existing id"
        qtTrId("test-conflictawitha-54521");

        tr("a source text", "with ), \), ( , \", and ' ) \) and \( as a comment");
    }
};
