// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef TEMPLATEGENERATOR_H
#define TEMPLATEGENERATOR_H

#include "filedocumentwriter.h"
#include "idocumentationhandler.h"
#include "ioutputproducer.h"
#include "outputcontext.h"
#include "filesystem/fileresolver.h"

#include <memory>
#include <optional>

QT_BEGIN_NAMESPACE

class Aggregate;
class CodeMarker;
class QDocDatabase;
struct DocumentIR;

class TemplateGenerator : public IOutputProducer, public IDocumentationHandler
{
public:
    explicit TemplateGenerator(FileResolver &fileResolver, QDocDatabase &qdb);
    ~TemplateGenerator() override;

    void prepare() override;
    void produce() override;
    void finalize() override;
    [[nodiscard]] QString format() const override;

    void beginDocument(const Node *node, const QString &fileName) override;
    void endDocument() override;
    [[nodiscard]] QString fileName(const Node *node) const override;
    void generateCollectionNode(CollectionNode *cn, CodeMarker *marker) override;
    void generateGenericCollectionPage(CollectionNode *cn, CodeMarker *marker) override;
    void generatePageNode(PageNode *pn, CodeMarker *marker) override;
    void generateCppReferencePage(Aggregate *aggregate, CodeMarker *marker) override;
    void generateQmlTypePage(QmlTypeNode *qcn, CodeMarker *marker) override;
    void generateProxyPage(Aggregate *aggregate, CodeMarker *marker) override;
    void mergeCollections(CollectionNode *cn) override;

    [[nodiscard]] QString fileExtension() const;

private:
    void renderDocument(const DocumentIR &ir, const QString &templateBaseName);
    [[nodiscard]] QString fileBase(const Node *node) const;
    void createDefaultWriter();

    FileResolver &m_fileResolver;
    QDocDatabase &m_qdb;
    std::unique_ptr<FileDocumentWriter> m_writer;
    std::optional<OutputContext> m_context;
    QString m_templateDir;
    QString m_fileExtension = QStringLiteral("html");

    // For testing: allow injection of mock writers
    friend class TemplateGeneratorTest;
};

QT_END_NAMESPACE

#endif // TEMPLATEGENERATOR_H
