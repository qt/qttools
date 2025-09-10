// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qhelp_global.h"

#include <QtCore/qcoreapplication.h>
#include <QtCore/qhash.h>
#include <QtCore/qmutex.h>
#include <QtGui/qtextdocument.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

QString QHelpGlobal::uniquifyConnectionName(const QString &name, void *pointer)
{
    static QMutex mutex;
    QMutexLocker locker(&mutex);
    static QHash<QString, quint16> idHash;
    return QString::asprintf("%ls-%p-%d", qUtf16Printable(name), pointer, ++idHash[name]);
}

QString QHelpGlobal::documentTitle(const QString &content)
{
    QString title = QCoreApplication::translate("QHelp", "Untitled");
    if (!content.isEmpty()) {
        const int start = content.indexOf("<title>"_L1, 0, Qt::CaseInsensitive) + 7;
        const int end = content.indexOf("</title>"_L1, 0, Qt::CaseInsensitive);
        if ((end - start) > 0) {
            title = content.mid(start, end - start);
            if (Qt::mightBeRichText(title) || title.contains(u'&')) {
                QTextDocument doc;
                doc.setHtml(title);
                title = doc.toPlainText();
            }
        }
    }
    return title;
}

QT_END_NAMESPACE
