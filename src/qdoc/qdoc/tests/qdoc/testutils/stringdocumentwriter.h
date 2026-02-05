// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef STRINGDOCUMENTWRITER_H
#define STRINGDOCUMENTWRITER_H

#include "../../../src/qdoc/idocumentwriter.h"

#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

/*!
    \class StringDocumentWriter
    \internal
    \brief Test double for IDocumentWriter that captures output in memory.

    StringDocumentWriter enables unit testing of generators without filesystem
    access. All write operations capture content to an in-memory string that
    can be retrieved via content().

    \section1 Usage in Tests

    \code
    StringDocumentWriter writer;
    writer.beginDocument("test.html");

    myGenerator.generatePage(writer);

    writer.endDocument();

    REQUIRE(writer.content().contains("<title>"));
    \endcode

    \sa IDocumentWriter, FileDocumentWriter
*/
class StringDocumentWriter : public IDocumentWriter
{
public:
    StringDocumentWriter() = default;
    ~StringDocumentWriter() override = default;

    /*!
        Opens a virtual document with the given \a fileName.
        Clears any previously captured content.
    */
    void beginDocument(const QString &fileName)
    {
        m_currentFileName = fileName;
        m_content.clear();
        m_open = true;
    }

    /*!
        Closes the virtual document.
    */
    void endDocument()
    {
        m_open = false;
    }

    // IDocumentWriter interface
    void write(QStringView content) override
    {
        if (m_open)
            m_content += content;
    }

    void writeLine(QStringView content) override
    {
        if (m_open) {
            m_content += content;
            m_content += QLatin1Char('\n');
        }
    }

    [[nodiscard]] bool isOpen() const override
    {
        return m_open;
    }

    [[nodiscard]] QString currentFileName() const override
    {
        return m_currentFileName;
    }

    // Test utilities
    /*!
        Returns all content written since the last beginDocument() call.
    */
    [[nodiscard]] QString content() const { return m_content; }

    /*!
        Clears captured content without closing the document.
    */
    void clearContent() { m_content.clear(); }

private:
    QString m_content;
    QString m_currentFileName;
    bool m_open{false};
};

QT_END_NAMESPACE

#endif // STRINGDOCUMENTWRITER_H

