// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "filedocumentwriter.h"

#include "location.h"

#include <QtCore/qdir.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

/*!
    \class FileDocumentWriter
    \internal
    \brief Production implementation of DocumentWriter that writes to files.

    FileDocumentWriter manages the lifecycle of output files, replacing the
    Generator's beginSubPage()/endSubPage() pattern with an explicit interface.

    \section1 Usage Pattern

    \code
    FileDocumentWriter writer(context);
    writer.beginDocument("output.html");
    writer.write("<html>");
    writer.writeLine("<body>Content</body>");
    writer.write("</html>");
    writer.endDocument();
    \endcode

    \section1 Error Handling

    If the file cannot be opened, beginDocument() reports an error and
    subsequent write calls become no-ops.

    \sa DocumentWriter, StringDocumentWriter, OutputContext
*/

/*!
    \fn const OutputContext &FileDocumentWriter::context() const

    Returns the output context used by this writer.
*/

/*!
    The constructor takes a \a context.

    \sa OutputContext
*/
FileDocumentWriter::FileDocumentWriter(OutputContext context)
    : m_context(std::move(context))
{
}

/*!
    The destructor ensures a call to endDocument before the object is destroyed.

    \sa endDocument()
*/
FileDocumentWriter::~FileDocumentWriter()
{
    endDocument();
}

/*!
    Opens a new document with the given \a fileName for writing.

    If a document is already open, it will be closed first.
*/
void FileDocumentWriter::beginDocument(const QString &fileName)
{
    // Close any currently open document
    endDocument();

    const QString &fullPath = m_context.outputDir.absoluteFilePath(fileName);
    m_currentFileName = fileName;

    // Ensure parent directory exists
    QFileInfo fileInfo(fullPath);
    QDir parentDir = fileInfo.dir();
    if (!parentDir.exists()) {
        if (!parentDir.mkpath(u"."_s)) {
            Location{}.error(u"Cannot create output directory '%1'"_s.arg(parentDir.path()));
            return;
        }
    }

    // Open the file
    m_file = std::make_unique<QFile>(fullPath);
    if (!m_file->open(QIODevice::WriteOnly | QIODevice::Text)) {
        Location{}.error(u"Cannot open file '%1' for writing: %2"_s
                                 .arg(fullPath, m_file->errorString()));
        m_file.reset();
        m_currentFileName.clear();
        return;
    }

    m_stream = std::make_unique<QTextStream>(m_file.get());
}

/*!
    Closes the current document, flushing any buffered content.
    Safe to call even if no document is open.
*/
void FileDocumentWriter::endDocument()
{
    if (m_stream) {
        m_stream->flush();
        m_stream.reset();
    }
    if (m_file) {
        m_file->close();
        m_file.reset();
    }
    m_currentFileName.clear();
}

void FileDocumentWriter::write(QStringView content)
{
    if (m_stream)
        *m_stream << content;
}

void FileDocumentWriter::writeLine(QStringView content)
{
    if (m_stream)
        *m_stream << content << '\n';
}

bool FileDocumentWriter::isOpen() const
{
    return m_file && m_file->isOpen();
}

QString FileDocumentWriter::currentFileName() const
{
    return m_currentFileName;
}

QT_END_NAMESPACE

