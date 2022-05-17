// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

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
