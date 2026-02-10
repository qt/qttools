// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef DOCUMENTWRITER_H
#define DOCUMENTWRITER_H

#include <QtCore/qstring.h>
#include <QtCore/qstringview.h>

QT_BEGIN_NAMESPACE

/*!
    \class DocumentWriter
    \internal
    \brief Interface for writing documentation output.

    DocumentWriter abstracts the output destination for documentation
    generators, enabling:

    \list
    \li Unit testing with in-memory writers (StringDocumentWriter)
    \li Production file-based output (FileDocumentWriter)
    \li Future extensions (network, database, etc.)
    \endlist

    This interface replaces Generator::out() which was coupled to a
    QTextStream stack managed by beginSubPage()/endSubPage().

    \section1 Usage Pattern

    \code
    void generateDocument(DocumentWriter &writer) {
        writer.write("<html>");
        writer.writeLine("<body>");
        writer.write("Content here");
        writer.writeLine("</body></html>");
    }
    \endcode

    \sa FileDocumentWriter, StringDocumentWriter
*/
class DocumentWriter
{
public:
    virtual ~DocumentWriter() = default;

    /*!
        Writes \a content to the output without a trailing newline.
    */
    virtual void write(QStringView content) = 0;

    /*!
        Writes \a content to the output followed by a newline.
    */
    virtual void writeLine(QStringView content = {}) = 0;

    /*!
        Returns \c true if a document is currently open for writing.
    */
    [[nodiscard]] virtual bool isOpen() const = 0;

    /*!
        Returns the file name of the currently open document,
        or an empty string if no document is open.
    */
    [[nodiscard]] virtual QString currentFileName() const = 0;
};

QT_END_NAMESPACE

#endif // DOCUMENTWRITER_H

