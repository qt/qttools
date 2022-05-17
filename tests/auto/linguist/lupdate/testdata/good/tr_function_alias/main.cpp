// Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Stephen Kelly <stephen.kelly@kdab.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0


























#ifndef MYOBJECT_H
#define MYOBJECT_H

#include <QtCore>

const char *c_1 = QT_TRANSLATE_NOOP("scope", "string");
const char *c_2 = QT_TRANSLATE_NOOP_ALIAS("scope", "string_alias");
const char *c_3 = QT_TRANSLATE_NOOP_UTF8("scope", "utf8_string");
const char *c_4 = QT_TRANSLATE_NOOP_UTF8_ALIAS("scope", "utf8_string_alias");
const char *c_5[2] = QT_TRANSLATE_NOOP3("scope", "string_with_comment", "comment");
const char *c_6[2] = QT_TRANSLATE_NOOP3_ALIAS("scope", "string_with_comment_alias", "comment");
const char *c_7[2] = QT_TRANSLATE_NOOP3_UTF8("scope", "utf8_string_with_comment", "comment");
const char *c_8[2] = QT_TRANSLATE_NOOP3_UTF8_ALIAS("scope", "utf8_string_with_comment_alias", "comment");
const char *c_9 = QT_TRID_NOOP("this_a_id");
const char *c_10 = QT_TRID_NOOP_ALIAS("this_a_id_alias");
QString test = qtTrId("yet_another_id");
QString test_alias = qtTrId_alias("yet_another_id_alias");

class Bogus : QObject {
    Q_OBJECT

    static const char * const s_strings[];
};

const char * const Bogus::s_strings[] = {
    QT_TR_NOOP("this should be in Bogus"),
    QT_TR_NOOP_ALIAS("this should be in Bogus Alias"),
    QT_TR_NOOP_UTF8("this should be utf8 in Bogus"),
    QT_TR_NOOP_UTF8_ALIAS("this should be utf8 in Bogus Alias")
};

class MyObject : public QObject
{
    Q_OBJECT
    explicit MyObject(QObject *parent = 0)
    {
        tr("Boo", "nsF::D");
        tr_alias("Boo_alias", "nsB::C");
        tr("utf8_Boo", "nsF::D"); // trUtf8 is now obsolete
        trUtf8_alias("utf8_Boo_alias", "nsF::D");
        QCoreApplication::translate("QTranslator", "Simple");
        translate_alias("QTranslator", "Simple with comment alias", "with comment");
    }
};

struct NonQObject
{
    Q_DECLARE_TR_FUNCTIONS_ALIAS(NonQObject)

    NonQObject()
    {
        tr("NonQObject_Boo", "nsF::NonQObject_D");
        tr_alias("NonQObject_Boo_alias", "nsB::NonQObject_C");
        tr("utf_NonQObject_Boo", "nsF::D");
        trUtf8_alias("utf8_NonQObject_Boo_alias", "nsF::D");
        QCoreApplication::translate("NonQObject_QTranslator", "Simple");
        translate_alias("NonQObject_QTranslator", "Simple with comment alias", "with comment");
    }
};

#endif
