// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef TEMPLATEGENERATOR_H
#define TEMPLATEGENERATOR_H

#include "generator.h"
#include "filesystem/fileresolver.h"

QT_BEGIN_NAMESPACE

class Aggregate;
class CodeMarker;

class TemplateGenerator : public Generator
{
public:
    explicit TemplateGenerator(FileResolver& file_resolver);
    ~TemplateGenerator() override = default;

    void initializeGenerator() override;
    void terminateGenerator() override;
    QString format() override;
    void generateDocs() override;

protected:
    [[nodiscard]] QString fileExtension() const override;
    void generateCppReferencePage(Aggregate *aggregate, CodeMarker *marker) override;
    void generateQmlTypePage(QmlTypeNode *qcn, CodeMarker *marker) override;
    void generatePageNode(PageNode *pn, CodeMarker *marker) override;
    void generateCollectionNode(CollectionNode *cn, CodeMarker *marker) override;

    qsizetype generateAtom(const Atom *atom, const Node *relative, CodeMarker *marker) override;

private:
    QString m_templateDir;
};

QT_END_NAMESPACE

#endif // TEMPLATEGENERATOR_H

