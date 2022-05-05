/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Linguist of the Qt Toolkit.
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
******************************************************************************/

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
