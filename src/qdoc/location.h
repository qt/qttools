/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
****************************************************************************/

/*
  location.h
*/

#ifndef LOCATION_H
#define LOCATION_H

#include <QtCore/qcoreapplication.h>
#include <QtCore/qstack.h>

QT_BEGIN_NAMESPACE

class QRegExp;

class Location
{
    Q_DECLARE_TR_FUNCTIONS(QDoc::Location)

public:
    Location();
    Location(const QString &filePath);
    Location(const Location &other);
    ~Location() { delete stk; }

    Location &operator=(const Location &other);

    void start();
    void advance(QChar ch);
    void advanceLines(int n)
    {
        stkTop->lineNo += n;
        stkTop->columnNo = 1;
    }

    void push(const QString &filePath);
    void pop();
    void setEtc(bool etc) { etcetera = etc; }
    void setLineNo(int no) { stkTop->lineNo = no; }
    void setColumnNo(int no) { stkTop->columnNo = no; }

    bool isEmpty() const { return stkDepth == 0; }
    int depth() const { return stkDepth; }
    const QString &filePath() const { return stkTop->filePath; }
    QString fileName() const;
    QString fileSuffix() const;
    int lineNo() const { return stkTop->lineNo; }
    int columnNo() const { return stkTop->columnNo; }
    bool etc() const { return etcetera; }
    void warning(const QString &message, const QString &details = QString()) const;
    void error(const QString &message, const QString &details = QString()) const;
    void fatal(const QString &message, const QString &details = QString()) const;
    void report(const QString &message, const QString &details = QString()) const;

    static void initialize();

    static void terminate();
    static void information(const QString &message);
    static void internalError(const QString &hint);
    static QString canonicalRelativePath(const QString &path);
    static int exitCode();

private:
    enum MessageType { Warning, Error, Report };

    struct StackEntry
    {
        QString filePath;
        int lineNo;
        int columnNo;
    };
    friend class QTypeInfo<StackEntry>;

    void emitMessage(MessageType type, const QString &message, const QString &details) const;
    QString toString() const;
    QString top() const;

private:
    StackEntry stkBottom;
    QStack<StackEntry> *stk;
    StackEntry *stkTop;
    int stkDepth;
    bool etcetera;

    static int tabSize;
    static int warningCount;
    static int warningLimit;
    static QString programName;
    static QString project;
    static QRegExp *spuriousRegExp;
};
Q_DECLARE_TYPEINFO(Location::StackEntry, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(Location, Q_COMPLEX_TYPE); // stkTop = &stkBottom

QT_END_NAMESPACE

#endif
