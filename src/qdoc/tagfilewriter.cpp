// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "tagfilewriter.h"

#include "access.h"
#include "aggregate.h"
#include "classnode.h"
#include "enumnode.h"
#include "functionnode.h"
#include "htmlgenerator.h"
#include "location.h"
#include "node.h"
#include "propertynode.h"
#include "qdocdatabase.h"
#include "typedefnode.h"

QT_BEGIN_NAMESPACE

/*!
  \class TagFileWriter

  This class handles the generation of the QDoc tag files.
 */

/*!
  Default constructor. \a qdb is the pointer to the
  qdoc database that is used when reading and writing the
  index files.
 */
TagFileWriter::TagFileWriter() : m_qdb(QDocDatabase::qdocDB()) { }

/*!
  Generate the tag file section with the given \a writer for the \a parent
  node.
 */
void TagFileWriter::generateTagFileCompounds(QXmlStreamWriter &writer, const Aggregate *parent)
{
    const auto &nonFunctionList = const_cast<Aggregate *>(parent)->nonfunctionList();
    for (const auto *node : nonFunctionList) {
        if (!node->url().isNull() || node->isPrivate())
            continue;

        QString kind;
        switch (node->nodeType()) {
        case Node::Namespace:
            kind = "namespace";
            break;
        case Node::Class:
        case Node::Struct:
        case Node::Union:
        case Node::QmlType:
            kind = "class";
            break;
        default:
            continue;
        }
        const auto *aggregate = static_cast<const Aggregate *>(node);

        QString access = "public";
        if (node->isProtected())
            access = "protected";

        QString objName = node->name();

        // Special case: only the root node should have an empty name.
        if (objName.isEmpty() && node != m_qdb->primaryTreeRoot())
            continue;

        // *** Write the starting tag for the element here. ***
        writer.writeStartElement("compound");
        writer.writeAttribute("kind", kind);

        if (node->isClassNode()) {
            writer.writeTextElement("name", node->fullDocumentName());
            writer.writeTextElement("filename", m_generator->fullDocumentLocation(node, false));

            // Classes contain information about their base classes.
            const auto *classNode = static_cast<const ClassNode *>(node);
            const QList<RelatedClass> &bases = classNode->baseClasses();
            for (const auto &related : bases) {
                ClassNode *n = related.m_node;
                if (n)
                    writer.writeTextElement("base", n->name());
            }

            // Recurse to write all members.
            generateTagFileMembers(writer, aggregate);
            writer.writeEndElement();

            // Recurse to write all compounds.
            generateTagFileCompounds(writer, aggregate);
        } else {
            writer.writeTextElement("name", node->fullDocumentName());
            writer.writeTextElement("filename", m_generator->fullDocumentLocation(node, false));

            // Recurse to write all members.
            generateTagFileMembers(writer, aggregate);
            writer.writeEndElement();

            // Recurse to write all compounds.
            generateTagFileCompounds(writer, aggregate);
        }
    }
}

/*!
  Writes all the members of the \a parent node with the \a writer.
  The node represents a C++ class, namespace, etc.
 */
void TagFileWriter::generateTagFileMembers(QXmlStreamWriter &writer, const Aggregate *parent)
{
    auto childNodes = parent->childNodes();
    std::sort(childNodes.begin(), childNodes.end(), Node::nodeNameLessThan);
    for (const auto *node : childNodes) {
        if (!node->url().isNull())
            continue;

        QString nodeName;
        QString kind;
        switch (node->nodeType()) {
        case Node::Enum:
            nodeName = "member";
            kind = "enumeration";
            break;
        case Node::TypeAlias: // Treated as typedef
        case Node::Typedef:
            nodeName = "member";
            kind = "typedef";
            break;
        case Node::Property:
            nodeName = "member";
            kind = "property";
            break;
        case Node::Function:
            nodeName = "member";
            kind = "function";
            break;
        case Node::Namespace:
            nodeName = "namespace";
            break;
        case Node::Class:
        case Node::Struct:
        case Node::Union:
            nodeName = "class";
            break;
        case Node::Variable:
        default:
            continue;
        }

        QString access;
        switch (node->access()) {
        case Access::Public:
            access = "public";
            break;
        case Access::Protected:
            access = "protected";
            break;
        case Access::Private:
        default:
            continue;
        }

        QString objName = node->name();

        // Special case: only the root node should have an empty name.
        if (objName.isEmpty() && node != m_qdb->primaryTreeRoot())
            continue;

        // *** Write the starting tag for the element here. ***
        writer.writeStartElement(nodeName);
        if (!kind.isEmpty())
            writer.writeAttribute("kind", kind);

        switch (node->nodeType()) {
        case Node::Class:
        case Node::Struct:
        case Node::Union:
            writer.writeCharacters(node->fullDocumentName());
            writer.writeEndElement();
            break;
        case Node::Namespace:
            writer.writeCharacters(node->fullDocumentName());
            writer.writeEndElement();
            break;
        case Node::Function: {
            /*
              Function nodes contain information about
              the type of function being described.
            */

            const auto *functionNode = static_cast<const FunctionNode *>(node);
            writer.writeAttribute("protection", access);
            writer.writeAttribute("virtualness", functionNode->virtualness());
            writer.writeAttribute("static", functionNode->isStatic() ? "yes" : "no");

            if (functionNode->isNonvirtual())
                writer.writeTextElement("type", functionNode->returnType());
            else
                writer.writeTextElement("type", "virtual " + functionNode->returnType());

            writer.writeTextElement("name", objName);
            const QStringList pieces =
                    m_generator->fullDocumentLocation(node, false).split(QLatin1Char('#'));
            writer.writeTextElement("anchorfile", pieces[0]);
            writer.writeTextElement("anchor", pieces[1]);
            QString signature = functionNode->signature(Node::SignatureReturnType);
            signature = signature.mid(signature.indexOf(QChar('('))).trimmed();
            if (functionNode->isConst())
                signature += " const";
            if (functionNode->isFinal())
                signature += " final";
            if (functionNode->isOverride())
                signature += " override";
            if (functionNode->isPureVirtual())
                signature += " = 0";
            writer.writeTextElement("arglist", signature);
        }
            writer.writeEndElement(); // member
            break;
        case Node::Property: {
            const auto *propertyNode = static_cast<const PropertyNode *>(node);
            writer.writeAttribute("type", propertyNode->dataType());
            writer.writeTextElement("name", objName);
            const QStringList pieces =
                    m_generator->fullDocumentLocation(node, false).split(QLatin1Char('#'));
            writer.writeTextElement("anchorfile", pieces[0]);
            writer.writeTextElement("anchor", pieces[1]);
            writer.writeTextElement("arglist", QString());
        }
            writer.writeEndElement(); // member
            break;
        case Node::Enum: {
            const auto *enumNode = static_cast<const EnumNode *>(node);
            writer.writeTextElement("name", objName);
            const QStringList pieces =
                    m_generator->fullDocumentLocation(node, false).split(QLatin1Char('#'));
            writer.writeTextElement("anchorfile", pieces[0]);
            writer.writeTextElement("anchor", pieces[1]);
            writer.writeEndElement(); // member

            for (const auto &item : enumNode->items()) {
                writer.writeStartElement("member");
                writer.writeAttribute("kind", "enumvalue");
                writer.writeTextElement("name", item.name());
                writer.writeTextElement("anchorfile", pieces[0]);
                writer.writeTextElement("anchor", pieces[1]);
                writer.writeTextElement("arglist", QString());
                writer.writeEndElement(); // member
            }
        } break;
        case Node::TypeAlias: // Treated as typedef
        case Node::Typedef: {
            const auto *typedefNode = static_cast<const TypedefNode *>(node);
            if (typedefNode->associatedEnum())
                writer.writeAttribute("type", typedefNode->associatedEnum()->fullDocumentName());
            else
                writer.writeAttribute("type", QString());
            writer.writeTextElement("name", objName);
            const QStringList pieces =
                    m_generator->fullDocumentLocation(node, false).split(QLatin1Char('#'));
            writer.writeTextElement("anchorfile", pieces[0]);
            writer.writeTextElement("anchor", pieces[1]);
            writer.writeTextElement("arglist", QString());
        }
            writer.writeEndElement(); // member
            break;

        case Node::Variable:
        default:
            break;
        }
    }
}

/*!
  Writes a tag file named \a fileName.
 */
void TagFileWriter::generateTagFile(const QString &fileName, Generator *g)
{
    QFile file(fileName);
    QFileInfo fileInfo(fileName);

    // If no path was specified or it doesn't exist,
    // default to the output directory
    if (fileInfo.fileName() == fileName || !fileInfo.dir().exists())
        file.setFileName(m_generator->outputDir() + QLatin1Char('/') + fileInfo.fileName());

    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        Location().warning(QString("Failed to open %1 for writing.").arg(file.fileName()));
        return;
    }

    m_generator = g;
    QXmlStreamWriter writer(&file);
    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    writer.writeStartElement("tagfile");
    generateTagFileCompounds(writer, m_qdb->primaryTreeRoot());
    writer.writeEndElement(); // tagfile
    writer.writeEndDocument();
    file.close();
}

QT_END_NAMESPACE
