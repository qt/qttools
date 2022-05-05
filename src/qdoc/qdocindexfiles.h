/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
******************************************************************************/

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

private:
    static QDocIndexFiles *s_qdocIndexFiles;
    QDocDatabase *m_qdb {};
    Generator *m_gen {};
    QString m_project;
    QList<QPair<ClassNode *, QString>> m_basesList;
    NodeList m_relatedNodes;
    bool m_storeLocationInfo;
};

QT_END_NAMESPACE

#endif
