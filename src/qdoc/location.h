/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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

#ifndef LOCATION_H
#define LOCATION_H

#include <QtCore/qcoreapplication.h>
#include <QtCore/qstack.h>

QT_BEGIN_NAMESPACE

class QRegularExpression;

class Location
{
public:
    Location();
    explicit Location(const QString &filePath);
    Location(const Location &other);
    ~Location() { delete m_stk; }

    Location &operator=(const Location &other);

    void start();
    void advance(QChar ch);
    void advanceLines(int n)
    {
        m_stkTop->m_lineNo += n;
        m_stkTop->m_columnNo = 1;
    }

    void push(const QString &filePath);
    void pop();
    void setEtc(bool etc) { m_etc = etc; }
    void setLineNo(int no) { m_stkTop->m_lineNo = no; }
    void setColumnNo(int no) { m_stkTop->m_columnNo = no; }

    [[nodiscard]] bool isEmpty() const { return m_stkDepth == 0; }
    [[nodiscard]] int depth() const { return m_stkDepth; }
    [[nodiscard]] const QString &filePath() const { return m_stkTop->m_filePath; }
    [[nodiscard]] QString fileName() const;
    [[nodiscard]] QString fileSuffix() const;
    [[nodiscard]] int lineNo() const { return m_stkTop->m_lineNo; }
    [[nodiscard]] int columnNo() const { return m_stkTop->m_columnNo; }
    [[nodiscard]] bool etc() const { return m_etc; }
    [[nodiscard]] QString toString() const;
    void warning(const QString &message, const QString &details = QString()) const;
    void error(const QString &message, const QString &details = QString()) const;
    void fatal(const QString &message, const QString &details = QString()) const;
    void report(const QString &message, const QString &details = QString()) const;

    static void initialize();

    static void terminate();
    static void information(const QString &message);
    static void internalError(const QString &hint);
    static int exitCode();

private:
    enum MessageType { Warning, Error, Report };

    struct StackEntry
    {
        QString m_filePath {};
        int m_lineNo {};
        int m_columnNo {};
    };
    friend class QTypeInfo<StackEntry>;

    void emitMessage(MessageType type, const QString &message, const QString &details) const;
    [[nodiscard]] QString top() const;

private:
    StackEntry m_stkBottom {};
    QStack<StackEntry> *m_stk {};
    StackEntry *m_stkTop {};
    int m_stkDepth {};
    bool m_etc {};

    static int s_tabSize;
    static int s_warningCount;
    static int s_warningLimit;
    static QString s_programName;
    static QString s_project;
    static QRegularExpression *s_spuriousRegExp;
};
Q_DECLARE_TYPEINFO(Location::StackEntry, Q_RELOCATABLE_TYPE);
Q_DECLARE_TYPEINFO(Location, Q_COMPLEX_TYPE); // stkTop = &stkBottom

QT_END_NAMESPACE

#endif
