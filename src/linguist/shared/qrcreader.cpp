/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Linguist of the Qt Toolkit.
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

#include "qrcreader.h"

#include <QtCore/qcoreapplication.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qxmlstream.h>

class FMT {
    Q_DECLARE_TR_FUNCTIONS(Linguist)
};

static bool isSupportedExtension(const QString &ext)
{
    return ext == QLatin1String("qml")
        || ext == QLatin1String("js") || ext == QLatin1String("qs")
        || ext == QLatin1String("ui") || ext == QLatin1String("jui");
}

ReadQrcResult readQrcFile(const QString &resourceFile, const QString &content)
{
    ReadQrcResult result;
    QString dirPath = QFileInfo(resourceFile).path();
    QXmlStreamReader reader(content);
    bool isFileTag = false;
    QStringList tagStack;
    tagStack << QLatin1String("RCC") << QLatin1String("qresource") << QLatin1String("file");
    int curDepth = 0;
    while (!reader.atEnd()) {
        QXmlStreamReader::TokenType t = reader.readNext();
        switch (t) {
        case QXmlStreamReader::StartElement:
            if (curDepth >= tagStack.count() || reader.name() != tagStack.at(curDepth)) {
                result.errorString = FMT::tr("unexpected <%1> tag\n")
                    .arg(reader.name().toString());
                result.line = reader.lineNumber();
                return result;
            }
            if (++curDepth == tagStack.count())
                isFileTag = true;
            break;

        case QXmlStreamReader::EndElement:
            isFileTag = false;
            if (curDepth == 0 || reader.name() != tagStack.at(curDepth - 1)) {
                result.errorString = FMT::tr("unexpected closing <%1> tag\n")
                    .arg(reader.name().toString());
                result.line = reader.lineNumber();
                return result;
            }
            --curDepth;
            break;

        case QXmlStreamReader::Characters:
            if (isFileTag) {
                QString fn = reader.text().toString();
                if (!QFileInfo(fn).isAbsolute())
                    fn = dirPath + QLatin1Char('/') + fn;
                QFileInfo cfi(fn);
                if (isSupportedExtension(cfi.suffix()))
                    result.files << cfi.filePath();
            }
            break;

        default:
            break;
        }
    }
    if (reader.error() != QXmlStreamReader::NoError) {
        result.errorString = reader.errorString();
        result.line = reader.lineNumber();
    }
    return result;
}
