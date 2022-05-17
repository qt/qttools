// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QtCore/QCoreApplication>
#include <QtCore/QHash>
#include <QtCore/QRegularExpression>
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
