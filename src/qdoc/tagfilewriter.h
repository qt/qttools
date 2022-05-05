/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
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

#ifndef TAGFILEWRITER_H
#define TAGFILEWRITER_H

#include <QtCore/qxmlstream.h>

QT_BEGIN_NAMESPACE

class Aggregate;
class Generator;
class QDocDatabase;

class TagFileWriter
{
public:
    TagFileWriter();
    ~TagFileWriter() = default;

    void generateTagFile(const QString &fileName, Generator *generator);

private:
    void generateTagFileCompounds(QXmlStreamWriter &writer, const Aggregate *inner);
    void generateTagFileMembers(QXmlStreamWriter &writer, const Aggregate *inner);

    QDocDatabase *m_qdb { nullptr };
    Generator *m_generator { nullptr };
};

QT_END_NAMESPACE

#endif // TAGFILEWRITER_H
