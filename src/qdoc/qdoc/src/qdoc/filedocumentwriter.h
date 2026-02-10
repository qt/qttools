// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef FILEDOCUMENTWRITER_H
#define FILEDOCUMENTWRITER_H

#include "documentwriter.h"
#include "outputcontext.h"

#include <QtCore/qfile.h>
#include <QtCore/qtextstream.h>

#include <memory>

QT_BEGIN_NAMESPACE

class FileDocumentWriter : public DocumentWriter
{
public:
    explicit FileDocumentWriter(OutputContext context);
    ~FileDocumentWriter() override;

    FileDocumentWriter(const FileDocumentWriter &) = delete;
    FileDocumentWriter &operator=(const FileDocumentWriter &) = delete;
    FileDocumentWriter(FileDocumentWriter &&) noexcept = default;
    FileDocumentWriter &operator=(FileDocumentWriter &&) noexcept = default;

    void beginDocument(const QString &fileName);

    void endDocument();

    void write(QStringView content) override;
    void writeLine(QStringView content) override;
    [[nodiscard]] bool isOpen() const override;
    [[nodiscard]] QString currentFileName() const override;

    [[nodiscard]] const OutputContext &context() const { return m_context; }

private:
    OutputContext m_context;
    std::unique_ptr<QFile> m_file;
    std::unique_ptr<QTextStream> m_stream;
    QString m_currentFileName;
};

QT_END_NAMESPACE

#endif // FILEDOCUMENTWRITER_H

