// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef FILETRANSFORMER_H
#define FILETRANSFORMER_H

#include <QString>
#include <QSet>

QT_BEGIN_NAMESPACE

class RecordDirectory;
class ConversionData;

class FileTransformer
{
public:
    FileTransformer(RecordDirectory &records, bool labels, bool quiet);

    void transformUi();
    void transformSources();
    bool transformTsFiles(const QStringList &translations, bool sortMessages);

    void generateMetaIds();
    bool updateTsFiles(const QStringList &translations);

    // Supported file extensions
    static const QSet<QString> cppExtensions;
    static const QSet<QString> otherExtensions;

private:
    RecordDirectory &m_records;
    bool m_labels;
    bool m_quiet;
};

QT_END_NAMESPACE

#endif // FILETRANSFORMER_H
