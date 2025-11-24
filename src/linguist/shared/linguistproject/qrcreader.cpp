// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "qrcreader.h"
#include "fmt.h"

#include <QtCore/qcoreapplication.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qxmlstream.h>

using namespace Qt::Literals::StringLiterals;

QT_BEGIN_NAMESPACE

bool isSupportedExtension(const QString &ext)
{
    return ext == "qml"_L1 || ext == "js"_L1 || ext == "qs"_L1 || ext == "ui"_L1 || ext == "jui"_L1;
}

ReadQrcResult readQrcFile(const QString &resourceFile, const QString &content)
{
    ReadQrcResult result;
    QString dirPath = QFileInfo(resourceFile).path();
    QXmlStreamReader reader(content);
    bool isFileTag = false;
    QStringList tagStack{ "RCC"_L1, "qresource"_L1, "file"_L1 };
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
                    fn = dirPath + u'/' + fn;
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

QT_END_NAMESPACE
