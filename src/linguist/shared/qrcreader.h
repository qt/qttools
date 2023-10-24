// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial

#ifndef QRCREADER_H
#define QRCREADER_H

#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>

class ReadQrcResult
{
public:
    QStringList files;
    QString errorString;
    qint64 line = 0;

    bool hasError() const { return !errorString.isEmpty(); }
};

ReadQrcResult readQrcFile(const QString &resourceFile, const QString &content);

bool isSupportedExtension(const QString &ext);

#endif // QRCREADER_H
