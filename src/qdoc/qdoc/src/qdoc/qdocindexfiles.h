// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QDOCINDEXFILES_H
#define QDOCINDEXFILES_H

#include "node.h"
#include "tree.h"

#include <QtCore/qhash.h>

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
    struct ReadState
    {
        QString project;
        QList<std::pair<ClassNode *, QString>> bases;
        NodeList relatedNodes;
        QHash<QString, SharedCommentNode *> sharedDocNodes;
    };

    struct WriteState
    {
        Node *root {};
        NodeList relatedNodes;
    };

    static QDocIndexFiles *qdocIndexFiles();
    static void destroyQDocIndexFiles();

    QDocIndexFiles();
    ~QDocIndexFiles();

    void readIndexes(const QStringList &indexFiles);
    void readIndexFile(const QString &path);
    void readIndexSection(QXmlStreamReader &reader, Node *current, const QString &indexUrl,
                          ReadState &state);
    void insertTarget(TargetRec::TargetType type, const QXmlStreamAttributes &attributes,
                      Node *node);
    void resolveIndex(ReadState &state);
    int indexForNode(Node *node, WriteState &state);
    bool adoptRelatedNode(Aggregate *adoptiveParent, int index, ReadState &state);
    void writeTargets(QXmlStreamWriter &writer, Node *node);

    void generateIndex(const QString &fileName, const QString &url, const QString &title,
                       const Generator *hrefGenerator);
    void generateFunctionSection(QXmlStreamWriter &writer, FunctionNode *fn,
                                 const Generator *generator, WriteState &state,
                                 IndexSectionWriter *post);
    void generateFunctionSections(QXmlStreamWriter &writer, Aggregate *aggregate,
                                  const Generator *generator, WriteState &state,
                                  IndexSectionWriter *post);
    bool generateIndexSection(QXmlStreamWriter &writer, Node *node,
                              const Generator *generator, WriteState &state,
                              IndexSectionWriter *post = nullptr);
    void generateIndexSections(QXmlStreamWriter &writer, Node *node,
                               const Generator *generator, WriteState &state,
                               IndexSectionWriter *post = nullptr);
    QString appendAttributesToSignature(const FunctionNode *fn) const noexcept;
    SharedCommentNode *getSharedCommentNode(const QString &sharedName,
        const QString &fullName, Node *node, bool assign = false);

private:
    static QDocIndexFiles *s_qdocIndexFiles;
    QDocDatabase *m_qdb {};
    bool m_storeLocationInfo;
};

QT_END_NAMESPACE

#endif
