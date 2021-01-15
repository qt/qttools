/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Assistant of the Qt Toolkit.
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
**
**
**
**
**
**
**
**
**
**
**
****************************************************************************/

#include <QtCore/QCoreApplication>
#include <QtCore/QRegExp>
#include <QtCore/QMutexLocker>
#include <QtGui/QTextDocument>

#include "qhelp_global.h"

QString QHelpGlobal::uniquifyConnectionName(const QString &name, void *pointer)
{
    static QMutex mutex;
    QMutexLocker locker(&mutex);

    static QHash<QString,quint16> idHash;

    return QString::fromLatin1("%1-%2-%3").
        arg(name).arg(quintptr(pointer)).arg(++idHash[name]);
}

QString QHelpGlobal::documentTitle(const QString &content)
{
    QString title = QCoreApplication::translate("QHelp", "Untitled");
    if (!content.isEmpty()) {
        const int start = content.indexOf(QLatin1String("<title>"), 0, Qt::CaseInsensitive) + 7;
        const int end = content.indexOf(QLatin1String("</title>"), 0, Qt::CaseInsensitive);
        if ((end - start) > 0) {
            title = content.mid(start, end - start);
            if (Qt::mightBeRichText(title) || title.contains(QLatin1Char('&'))) {
                QTextDocument doc;
                doc.setHtml(title);
                title = doc.toPlainText();
            }
        }
    }
    return title;
}

QString QHelpGlobal::codecFromData(const QByteArray &data)
{
    QString codec = codecFromXmlData(data);
    if (codec.isEmpty())
        codec = codecFromHtmlData(data);
    return codec.isEmpty() ? QLatin1String("utf-8") : codec;
}

QString QHelpGlobal::codecFromHtmlData(const QByteArray &data)
{
    const QString &head = QString::fromUtf8(data.constData(), qMin(1000, data.size()));
    int start = head.indexOf(QLatin1String("<meta"), 0, Qt::CaseInsensitive);
    if (start > 0) {
        const QRegExp r(QLatin1String("charset=([^\"\\s]+)"));
        while (start != -1) {
            const int end = head.indexOf(QLatin1Char('>'), start) + 1;
            if (end <= start)
                break;
            const QString &meta = head.mid(start, end - start).toLower();
            if (r.indexIn(meta) != -1)
                return r.cap(1);
            start = head.indexOf(QLatin1String("<meta"), end,
                                    Qt::CaseInsensitive);
        }
    }
    return QString();
}

QString QHelpGlobal::codecFromXmlData(const QByteArray &data)
{
    const QString head = QString::fromUtf8(data.constData(), qMin(1000, data.size()));
    const QRegExp encodingExp(QLatin1String("^\\s*<\\?xml version="
                  "\"\\d\\.\\d\" encoding=\"([^\"]+)\"\\?>.*"));
    return encodingExp.exactMatch(head) ? encodingExp.cap(1) : QString();
}
