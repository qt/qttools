// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef TEMPLATEGENERATOR_H
#define TEMPLATEGENERATOR_H

#include "filedocumentwriter.h"
#include "documentationhandler.h"
#include "outputproducer.h"
#include "outputcontext.h"
#include "filesystem/fileresolver.h"

#include <memory>
#include <optional>

QT_BEGIN_NAMESPACE

class Aggregate;
class CodeMarker;
class HrefResolver;
class LinkResolver;
class QDocDatabase;
namespace IR { struct AllMembersIR; struct Document; }

/*!
    \class TemplateGenerator
    \internal
    \brief Generates documentation using external templates and a pre-built IR.

    TemplateGenerator implements OutputProducer and DocumentationHandler to
    generate documentation without inheriting from Generator. It uses
    DocumentationTraverser for tree traversal and delegates content generation
    to templates via the IR system.

    \section1 Architecture

    The generator follows a composition-based design:
    \list
    \li \b{OutputProducer}: Lifecycle interface (prepare/produce/finalize)
    \li \b{DocumentationHandler}: Content generation callbacks for traverser
    \li \b{DocumentationTraverser}: Shared tree traversal logic
    \li \b{DocumentWriter}: Output abstraction (file or string for tests)
    \endlist

    \sa DocumentationTraverser, DocumentationHandler, OutputProducer
*/
class TemplateGenerator : public OutputProducer, public DocumentationHandler
{
public:
    explicit TemplateGenerator(FileResolver &fileResolver, QDocDatabase &qdb,
                               const QString &format = QString());
    ~TemplateGenerator() override;

    // === OutputProducer interface ===
    void prepare() override;
    void produce() override;
    void finalize() override;
    [[nodiscard]] QString format() const override;

    // === DocumentationHandler interface ===
    void beginDocument(const QString &fileName) override;
    void endDocument() override;
    [[nodiscard]] QString fileName(const Node *node) const override;
    void generateCollectionNode(CollectionNode *cn, CodeMarker *marker) override;
    void generateGenericCollectionPage(CollectionNode *cn, CodeMarker *marker) override;
    void generatePageNode(PageNode *pn, CodeMarker *marker) override;
    void generateCppReferencePage(Aggregate *aggregate, CodeMarker *marker) override;
    void generateQmlTypePage(QmlTypeNode *qcn, CodeMarker *marker) override;
    void generateProxyPage(Aggregate *aggregate, CodeMarker *marker) override;
    void mergeCollections(CollectionNode *cn) override;

    // Public accessors for configuration
    [[nodiscard]] QString fileExtension() const;

private:
    // Render phase: Format IR according to templates.
    void renderDocument(const IR::Document &ir, const QString &templateBaseName);
    void renderJson(const QJsonObject &json, const QString &templateBaseName);
    void generateMemberListingPage(const Node *node, const IR::AllMembersIR &allMembers);

    // Include resolution for Inja {% include %} directives.
    [[nodiscard]] QString resolveInclude(const QString &name) const;

    // Filename computation (adapted from Generator)
    [[nodiscard]] QString fileBase(const Node *node) const;

    // Creates the production writer (FileDocumentWriter).
    void createDefaultWriter();

    FileResolver &m_fileResolver;
    QDocDatabase &m_qdb;
    QString m_format;
    std::unique_ptr<FileDocumentWriter> m_writer;
    std::optional<OutputContext> m_context;
    QString m_templateDir;
    QString m_fileExtension = QStringLiteral("html");
    std::unique_ptr<HrefResolver> m_hrefResolver;
    std::unique_ptr<LinkResolver> m_linkResolver;
    bool m_emitStylesheet = false;
    QString m_stylesheetName;

    void copyAssets();
    void resolveImagePaths(IR::Document &ir);

    // For testing: allow injection of mock writers
    friend class TemplateGeneratorTest;
};

QT_END_NAMESPACE

#endif // TEMPLATEGENERATOR_H
