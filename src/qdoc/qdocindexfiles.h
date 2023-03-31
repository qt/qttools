// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QDOCINDEXFILES_H
#define QDOCINDEXFILES_H

#include "node.h"
#include "tree.h"

QT_BEGIN_NAMESPACE

class Atom;
class FunctionNode;
class Generator;
class QDocDatabase;
class WebXMLGenerator;
class QXmlStreamReader;
class QXmlStreamWriter;
class QXmlStreamAttributes;

// A callback interface for extending index sections
class IndexSectionWriter
{
public:
    virtual ~IndexSectionWriter() = default;
    virtual void append(QXmlStreamWriter &writer, Node *node) = 0;
};

class QDocIndexFiles
{
    friend class QDocDatabase;
    friend class WebXMLGenerator; // for using generateIndexSections()

private:
    static QDocIndexFiles *qdocIndexFiles();
    static void destroyQDocIndexFiles();

    QDocIndexFiles();
    ~QDocIndexFiles();

    void readIndexes(const QStringList &indexFiles);
    void readIndexFile(const QString &path);
    void readIndexSection(QXmlStreamReader &reader, Node *current, const QString &indexUrl);
    void insertTarget(TargetRec::TargetType type, const QXmlStreamAttributes &attributes,
                      Node *node);
    void resolveIndex();
    int indexForNode(Node *node);
    bool adoptRelatedNode(Aggregate *adoptiveParent, int index);

    void generateIndex(const QString &fileName, const QString &url, const QString &title,
                       Generator *g);
    void generateFunctionSection(QXmlStreamWriter &writer, FunctionNode *fn);
    void generateFunctionSections(QXmlStreamWriter &writer, Aggregate *aggregate);
    bool generateIndexSection(QXmlStreamWriter &writer, Node *node,
                              IndexSectionWriter *post = nullptr);
    void generateIndexSections(QXmlStreamWriter &writer, Node *node,
                               IndexSectionWriter *post = nullptr);
    QString appendAttributesToSignature(const FunctionNode *fn) const noexcept;

private:
    static QDocIndexFiles *s_qdocIndexFiles;
    QDocDatabase *m_qdb {};
    Generator *m_gen {};
    QString m_project;
    QList<std::pair<ClassNode *, QString>> m_basesList;
    NodeList m_relatedNodes;
    bool m_storeLocationInfo;
};

QT_END_NAMESPACE

#endif
