// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "textedit.h"

#include <QFile>
#include <QFileInfo>

TextEdit::TextEdit(QWidget *parent)
    : QTextEdit(parent)
{
    setReadOnly(true);
}

void TextEdit::setContents(const QString &fileName)
{
    const QFileInfo fi(fileName);
    srcUrl = QUrl::fromLocalFile(fi.absoluteFilePath());
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly)) {
        const QString data(QString::fromUtf8(file.readAll()));
        if (fileName.endsWith(".html"))
            setHtml(data);
        else
            setPlainText(data);
    }
}

QVariant TextEdit::loadResource(int type, const QUrl &name)
{
    if (type == QTextDocument::ImageResource) {
        QFile file(srcUrl.resolved(name).toLocalFile());
        if (file.open(QIODevice::ReadOnly))
            return file.readAll();
    }
    return QTextEdit::loadResource(type, name);
}
