// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "qrcreader.h"
#include "fmt.h"

#include <QtCore/qcoreapplication.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qxmlstream.h>

bool isSupportedExtension(const QString &ext)
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
            if (curDepth >= tagStack.size() || reader.name() != tagStack.at(curDepth)) {
                result.errorString = FMT::tr("unexpected <%1> tag\n")
                    .arg(reader.name().toString());
                result.line = reader.lineNumber();
                return result;
            }
            if (++curDepth == tagStack.size())
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
