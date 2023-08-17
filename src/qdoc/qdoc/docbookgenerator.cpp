// Copyright (C) 2019 Thibaut Cuvelier
// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "docbookgenerator.h"

#include "access.h"
#include "aggregate.h"
#include "classnode.h"
#include "codemarker.h"
#include "collectionnode.h"
#include "config.h"
#include "enumnode.h"
#include "examplenode.h"
#include "functionnode.h"
#include "generator.h"
#include "node.h"
#include "propertynode.h"
#include "quoter.h"
#include "qdocdatabase.h"
#include "qmlpropertynode.h"
#include "sharedcommentnode.h"
#include "typedefnode.h"
#include "variablenode.h"

#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include <QtCore/quuid.h>
#include <QtCore/qurl.h>
#include <QtCore/qregularexpression.h>
#include <QtCore/qversionnumber.h>

#include <cctype>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

static const char dbNamespace[] = "http://docbook.org/ns/docbook";
static const char xlinkNamespace[] = "http://www.w3.org/1999/xlink";
static const char itsNamespace[] = "http://www.w3.org/2005/11/its";

DocBookGenerator::DocBookGenerator(FileResolver& file_resolver) : XmlGenerator(file_resolver) {}

inline void DocBookGenerator::newLine()
{
    m_writer->writeCharacters("\n");
}

inline void DocBookGenerator::writeRawHtml(const QString &rawCode)
{
    m_writer->writeStartElement(dbNamespace, "programlisting");
    m_writer->writeAttribute("role", "raw-html");
    m_writer->writeCDATA(rawCode);
    m_writer->writeEndElement(); // programlisting
    newLine();
}

void DocBookGenerator::writeXmlId(const QString &id)
{
    if (id.isEmpty())
        return;

    m_writer->writeAttribute("xml:id", registerRef(id, true));
}

void DocBookGenerator::writeXmlId(const Node *node)
{
    if (!node)
        return;

    // Specifically for nodes, do not use the same code path, as refForNode
    // calls registerRef in all cases. Calling registerRef a second time adds
    // a character to "disambiguate" the two IDs (the one returned by
    // refForNode, then the one that is written as xml:id).
    m_writer->writeAttribute("xml:id", Generator::cleanRef(refForNode(node), true));
}

void DocBookGenerator::startSectionBegin(const QString &id)
{
    m_hasSection = true;

    m_writer->writeStartElement(dbNamespace, "section");
    writeXmlId(id);
    newLine();
    m_writer->writeStartElement(dbNamespace, "title");
}

void DocBookGenerator::startSectionBegin(const Node *node)
{
    m_writer->writeStartElement(dbNamespace, "section");
    writeXmlId(node);
    newLine();
    m_writer->writeStartElement(dbNamespace, "title");
}

void DocBookGenerator::startSectionEnd()
{
    m_writer->writeEndElement(); // title
    newLine();
}

void DocBookGenerator::startSection(const QString &id, const QString &title)
{
    startSectionBegin(id);
    m_writer->writeCharacters(title);
    startSectionEnd();
}

void DocBookGenerator::startSection(const Node *node, const QString &title)
{
    startSectionBegin(node);
    m_writer->writeCharacters(title);
    startSectionEnd();
}

void DocBookGenerator::startSection(const QString &title)
{
    // No xml:id given: down the calls, "" is interpreted as "no ID".
    startSection("", title);
}

void DocBookGenerator::endSection()
{
    m_writer->writeEndElement(); // section
    newLine();
}

void DocBookGenerator::writeAnchor(const QString &id)
{
    if (id.isEmpty())
        return;

    m_writer->writeEmptyElement(dbNamespace, "anchor");
    writeXmlId(id);
    newLine();
}

/*!
  Initializes the DocBook output generator's data structures
  from the configuration (Config).
 */
void DocBookGenerator::initializeGenerator()
{
    // Excerpts from HtmlGenerator::initializeGenerator.
    Generator::initializeGenerator();
    m_config = &Config::instance();

    m_project = m_config->get(CONFIG_PROJECT).asString();

    m_projectDescription = m_config->get(CONFIG_DESCRIPTION).asString();
    if (m_projectDescription.isEmpty() && !m_project.isEmpty())
        m_projectDescription = m_project + QLatin1String(" Reference Documentation");

    m_naturalLanguage = m_config->get(CONFIG_NATURALLANGUAGE).asString();
    if (m_naturalLanguage.isEmpty())
        m_naturalLanguage = QLatin1String("en");

    m_buildVersion = m_config->get(CONFIG_BUILDVERSION).asString();
    m_useDocBook52 = m_config->get(CONFIG_DOCBOOKEXTENSIONS).asBool() ||
            m_config->get(format() + Config::dot + "usedocbookextensions").asBool();
    m_useITS = m_config->get(format() + Config::dot + "its").asBool();
}

QString DocBookGenerator::format()
{
    return "DocBook";
}

/*!
  Returns "xml" for this subclass of Generator.
 */
QString DocBookGenerator::fileExtension() const
{
    return "xml";
}

/*!
  Generate the documentation for \a relative. i.e. \a relative
  is the node that represents the entity where a qdoc comment
  was found, and \a text represents the qdoc comment.
 */
bool DocBookGenerator::generateText(const Text &text, const Node *relative)
{
    // From Generator::generateText.
    if (!text.firstAtom())
        return false;

    int numAtoms = 0;
    initializeTextOutput();
    generateAtomList(text.firstAtom(), relative, true, numAtoms);
    closeTextSections();
    return true;
}

/*!
  Generate the text for \a atom relatively to \a relative.
  \a generate indicates if output to \a writer is expected.
  The number of generated atoms is returned in the argument
  \a numAtoms. The returned value is the first atom that was not
  generated.
 */
const Atom *DocBookGenerator::generateAtomList(const Atom *atom, const Node *relative,
                                               bool generate, int &numAtoms)
{
    Q_ASSERT(m_writer);
    // From Generator::generateAtomList.
    while (atom) {
        switch (atom->type()) {
        case Atom::FormatIf: {
            int numAtoms0 = numAtoms;
            atom = generateAtomList(atom->next(), relative, generate, numAtoms);
            if (!atom)
                return nullptr;

            if (atom->type() == Atom::FormatElse) {
                ++numAtoms;
                atom = generateAtomList(atom->next(), relative, false, numAtoms);
                if (!atom)
                    return nullptr;
            }

            if (atom->type() == Atom::FormatEndif) {
                if (generate && numAtoms0 == numAtoms) {
                    relative->location().warning(QStringLiteral("Output format %1 not handled %2")
                                                         .arg(format(), outFileName()));
                    Atom unhandledFormatAtom(Atom::UnhandledFormat, format());
                    generateAtomList(&unhandledFormatAtom, relative, generate, numAtoms);
                }
                atom = atom->next();
            }
        } break;
        case Atom::FormatElse:
        case Atom::FormatEndif:
            return atom;
        default:
            int n = 1;
            if (generate) {
                n += generateAtom(atom, relative);
                numAtoms += n;
            }
            while (n-- > 0)
                atom = atom->next();
        }
    }
    return nullptr;
}

QString removeCodeMarkers(const QString& code) {
    QString rewritten = code;
    static const QRegularExpression re("(<@[^>&]*>)|(<\\/@[^&>]*>)");
    rewritten.replace(re, "");
    return rewritten;
}

/*!
  Generate DocBook from an instance of Atom.
 */
qsizetype DocBookGenerator::generateAtom(const Atom *atom, const Node *relative)
{
    Q_ASSERT(m_writer);
    // From HtmlGenerator::generateAtom, without warning generation.
    int idx = 0;
    int skipAhead = 0;
    Node::Genus genus = Node::DontCare;

    switch (atom->type()) {
    case Atom::AutoLink:
        // Allow auto-linking to nodes in API reference
        genus = Node::API;
        Q_FALLTHROUGH();
    case Atom::NavAutoLink:
        if (!m_inLink && !m_inContents && !m_inSectionHeading) {
            const Node *node = nullptr;
            QString link = getAutoLink(atom, relative, &node, genus);
            if (!link.isEmpty() && node && node->isDeprecated()
                && relative->parent() != node && !relative->isDeprecated()) {
                link.clear();
            }
            if (link.isEmpty()) {
                m_writer->writeCharacters(atom->string());
            } else {
                beginLink(link, node, relative);
                generateLink(atom);
                endLink();
            }
        } else {
            m_writer->writeCharacters(atom->string());
        }
        break;
    case Atom::BaseName:
        break;
    case Atom::BriefLeft:
        if (!hasBrief(relative)) {
            skipAhead = skipAtoms(atom, Atom::BriefRight);
            break;
        }
        m_writer->writeStartElement(dbNamespace, "para");
        m_inPara = true;
        rewritePropertyBrief(atom, relative);
        break;
    case Atom::BriefRight:
        if (hasBrief(relative)) {
            m_writer->writeEndElement(); // para
            m_inPara = false;
            newLine();
        }
        break;
    case Atom::C:
        // This may at one time have been used to mark up C++ code but it is
        // now widely used to write teletype text. As a result, text marked
        // with the \c command is not passed to a code marker.
        if (m_inTeletype)
            m_writer->writeCharacters(plainCode(atom->string()));
        else
            m_writer->writeTextElement(dbNamespace, "code", plainCode(atom->string()));
        break;
    case Atom::CaptionLeft:
        m_writer->writeStartElement(dbNamespace, "title");
        break;
    case Atom::CaptionRight:
        endLink();
        m_writer->writeEndElement(); // title
        newLine();
        break;
    case Atom::Qml:
        m_writer->writeStartElement(dbNamespace, "programlisting");
        m_writer->writeAttribute("language", "qml");
        if (m_useITS)
            m_writer->writeAttribute(itsNamespace, "translate", "no");
        m_writer->writeCharacters(removeCodeMarkers(atom->string()));
        m_writer->writeEndElement(); // programlisting
        newLine();
        break;
    case Atom::Code:
        m_writer->writeStartElement(dbNamespace, "programlisting");
        m_writer->writeAttribute("language", "cpp");
        if (m_useITS)
            m_writer->writeAttribute(itsNamespace, "translate", "no");
        m_writer->writeCharacters(removeCodeMarkers(atom->string()));
        m_writer->writeEndElement(); // programlisting
        newLine();
        break;
    case Atom::CodeBad:
        m_writer->writeStartElement(dbNamespace, "programlisting");
        m_writer->writeAttribute("language", "cpp");
        m_writer->writeAttribute("role", "bad");
        if (m_useITS)
            m_writer->writeAttribute(itsNamespace, "translate", "no");
        m_writer->writeCharacters(removeCodeMarkers(atom->string()));
        m_writer->writeEndElement(); // programlisting
        newLine();
        break;
    case Atom::DetailsLeft:
    case Atom::DetailsRight:
        break;
    case Atom::DivLeft:
    case Atom::DivRight:
        break;
    case Atom::FootnoteLeft:
        m_writer->writeStartElement(dbNamespace, "footnote");
        newLine();
        m_writer->writeStartElement(dbNamespace, "para");
        m_inPara = true;
        break;
    case Atom::FootnoteRight:
        m_writer->writeEndElement(); // para
        m_inPara = false;
        newLine();
        m_writer->writeEndElement(); // footnote
        break;
    case Atom::FormatElse:
    case Atom::FormatEndif:
    case Atom::FormatIf:
        break;
    case Atom::FormattingLeft:
        if (atom->string() == ATOM_FORMATTING_BOLD) {
            m_writer->writeStartElement(dbNamespace, "emphasis");
            m_writer->writeAttribute("role", "bold");
        } else if (atom->string() == ATOM_FORMATTING_ITALIC) {
            m_writer->writeStartElement(dbNamespace, "emphasis");
        } else if (atom->string() == ATOM_FORMATTING_UNDERLINE) {
            m_writer->writeStartElement(dbNamespace, "emphasis");
            m_writer->writeAttribute("role", "underline");
        } else if (atom->string() == ATOM_FORMATTING_SUBSCRIPT) {
            m_writer->writeStartElement(dbNamespace, "subscript");
        } else if (atom->string() == ATOM_FORMATTING_SUPERSCRIPT) {
            m_writer->writeStartElement(dbNamespace, "superscript");
        } else if (atom->string() == ATOM_FORMATTING_TELETYPE
                   || atom->string() == ATOM_FORMATTING_PARAMETER) {
            m_writer->writeStartElement(dbNamespace, "code");
            if (m_useITS)
                m_writer->writeAttribute(itsNamespace, "translate", "no");

            if (atom->string() == ATOM_FORMATTING_PARAMETER)
                m_writer->writeAttribute("role", "parameter");
            else // atom->string() == ATOM_FORMATTING_TELETYPE
                m_inTeletype = true;

            // For parameters, understand subscripts.
            if (atom->string() == ATOM_FORMATTING_PARAMETER) {
                if (atom->next() != nullptr && atom->next()->type() == Atom::String) {
                    static const QRegularExpression subscriptRegExp("^([a-z]+)_([0-9n])$");
                    auto match = subscriptRegExp.match(atom->next()->string());
                    if (match.hasMatch()) {
                        m_writer->writeCharacters(match.captured(1));
                        m_writer->writeStartElement(dbNamespace, "subscript");
                        m_writer->writeCharacters(match.captured(2));
                        m_writer->writeEndElement(); // subscript
                        skipAhead = 1;
                    }
                }
            }
        } else if (atom->string() == ATOM_FORMATTING_UICONTROL) {
            m_writer->writeStartElement(dbNamespace, "guilabel");
            if (m_useITS)
                m_writer->writeAttribute(itsNamespace, "translate", "no");
        } else {
            relative->location().warning(QStringLiteral("Unsupported formatting: %1").arg(atom->string()));
        }
        break;
    case Atom::FormattingRight:
        if (atom->string() == ATOM_FORMATTING_BOLD || atom->string() == ATOM_FORMATTING_ITALIC
            || atom->string() == ATOM_FORMATTING_UNDERLINE
            || atom->string() == ATOM_FORMATTING_SUBSCRIPT
            || atom->string() == ATOM_FORMATTING_SUPERSCRIPT
            || atom->string() == ATOM_FORMATTING_TELETYPE
            || atom->string() == ATOM_FORMATTING_PARAMETER
            || atom->string() == ATOM_FORMATTING_UICONTROL) {
            m_writer->writeEndElement();
        } else if (atom->string() == ATOM_FORMATTING_LINK) {
            if (atom->string() == ATOM_FORMATTING_TELETYPE)
                m_inTeletype = false;
            endLink();
        } else {
            relative->location().warning(QStringLiteral("Unsupported formatting: %1").arg(atom->string()));
        }
        break;
    case Atom::AnnotatedList:
        if (const CollectionNode *cn = m_qdb->getCollectionNode(atom->string(), Node::Group))
            generateList(cn, atom->string());
        break;
    case Atom::GeneratedList: {
        bool hasGeneratedSomething = false;
        if (atom->string() == QLatin1String("annotatedclasses")
            || atom->string() == QLatin1String("attributions")
            || atom->string() == QLatin1String("namespaces")) {
            const NodeMultiMap things = atom->string() == QLatin1String("annotatedclasses")
                    ? m_qdb->getCppClasses()
                    : atom->string() == QLatin1String("attributions") ? m_qdb->getAttributions()
                                                                      : m_qdb->getNamespaces();
            generateAnnotatedList(relative, things.values(), atom->string());
            hasGeneratedSomething = !things.isEmpty();
        } else if (atom->string() == QLatin1String("annotatedexamples")
                || atom->string() == QLatin1String("annotatedattributions")) {
            const NodeMultiMap things = atom->string() == QLatin1String("annotatedexamples")
                                        ? m_qdb->getAttributions()
                                        : m_qdb->getExamples();
            generateAnnotatedLists(relative, things, atom->string());
            hasGeneratedSomething = !things.isEmpty();
        } else if (atom->string() == QLatin1String("classes")
                   || atom->string() == QLatin1String("qmlbasictypes") // deprecated!
                   || atom->string() == QLatin1String("qmlvaluetypes")
                   || atom->string() == QLatin1String("qmltypes")) {
            const NodeMultiMap things = atom->string() == QLatin1String("classes")
                    ? m_qdb->getCppClasses()
                    : (atom->string() == QLatin1String("qmlvaluetypes")
                       || atom->string() == QLatin1String("qmlbasictypes"))
                    ? m_qdb->getQmlValueTypes()
                    : m_qdb->getQmlTypes();
            generateCompactList(relative, things, true, QString(), atom->string());
            hasGeneratedSomething = !things.isEmpty();
        } else if (atom->string().contains("classes ")) {
            QString rootName = atom->string().mid(atom->string().indexOf("classes") + 7).trimmed();
            NodeMultiMap things = m_qdb->getCppClasses();

            hasGeneratedSomething = !things.isEmpty();
            generateCompactList(relative, things, true, rootName, atom->string());
        } else if ((idx = atom->string().indexOf(QStringLiteral("bymodule"))) != -1) {
            QString moduleName = atom->string().mid(idx + 8).trimmed();
            Node::NodeType type = typeFromString(atom);
            QDocDatabase *qdb = QDocDatabase::qdocDB();
            if (const CollectionNode *cn = qdb->getCollectionNode(moduleName, type)) {
                if (type == Node::Module) {
                    NodeMap m;
                    cn->getMemberClasses(m);
                    if (!m.isEmpty())
                        generateAnnotatedList(relative, m.values(), atom->string());
                    hasGeneratedSomething = !m.isEmpty();
                } else {
                    generateAnnotatedList(relative, cn->members(), atom->string());
                    hasGeneratedSomething = !cn->members().isEmpty();
                }
            }
        } else if (atom->string() == QLatin1String("classhierarchy")) {
            generateClassHierarchy(relative, m_qdb->getCppClasses());
            hasGeneratedSomething = !m_qdb->getCppClasses().isEmpty();
        } else if (atom->string().startsWith("obsolete")) {
            QString prefix = atom->string().contains("cpp") ? QStringLiteral("Q") : QString();
            const NodeMultiMap &things = atom->string() == QLatin1String("obsoleteclasses")
                    ? m_qdb->getObsoleteClasses()
                    : atom->string() == QLatin1String("obsoleteqmltypes")
                    ? m_qdb->getObsoleteQmlTypes()
                    : atom->string() == QLatin1String("obsoletecppmembers")
                    ? m_qdb->getClassesWithObsoleteMembers()
                    : m_qdb->getQmlTypesWithObsoleteMembers();
            generateCompactList(relative, things, false, prefix, atom->string());
            hasGeneratedSomething = !things.isEmpty();
        } else if (atom->string() == QLatin1String("functionindex")) {
            generateFunctionIndex(relative);
            hasGeneratedSomething = !m_qdb->getFunctionIndex().isEmpty();
        } else if (atom->string() == QLatin1String("legalese")) {
            generateLegaleseList(relative);
            hasGeneratedSomething = !m_qdb->getLegaleseTexts().isEmpty();
        } else if (atom->string() == QLatin1String("overviews")
                   || atom->string() == QLatin1String("cpp-modules")
                   || atom->string() == QLatin1String("qml-modules")
                   || atom->string() == QLatin1String("related")) {
            generateList(relative, atom->string());
            hasGeneratedSomething = true; // Approximation, because there is
                                          // some nontrivial logic in generateList.
        } else if (const auto *cn = m_qdb->getCollectionNode(atom->string(), Node::Group); cn) {
            generateAnnotatedList(cn, cn->members(), atom->string(), ItemizedList);
            hasGeneratedSomething = true; // Approximation
        }

        // There must still be some content generated for the DocBook document
        // to be valid (except if already in a paragraph).
        if (!hasGeneratedSomething && !m_inPara) {
            m_writer->writeEmptyElement(dbNamespace, "para");
            newLine();
        }
    }
        break;
    case Atom::SinceList:
        // Table of contents, should automatically be generated by the DocBook processor.
        Q_FALLTHROUGH();
    case Atom::LineBreak:
    case Atom::BR:
    case Atom::HR:
        // Not supported in DocBook.
        break;
    case Atom::Image: // mediaobject
        // An Image atom is always followed by an ImageText atom,
        // containing the alternative text.
        // If no caption is present, we just output a <db:mediaobject>,
        // avoiding the wrapper as it is not required.
        // For bordered images, there is another atom before the
        // caption, DivRight (the corresponding DivLeft being just
        // before the image).

        if (atom->next() && matchAhead(atom->next(), Atom::DivRight) && atom->next()->next()
            && matchAhead(atom->next()->next(), Atom::CaptionLeft)) {
            // If there is a caption, there must be a <db:figure>
            // wrapper starting with the caption.
            Q_ASSERT(atom->next());
            Q_ASSERT(atom->next()->next());
            Q_ASSERT(atom->next()->next()->next());
            Q_ASSERT(atom->next()->next()->next()->next());
            Q_ASSERT(atom->next()->next()->next()->next()->next());

            m_writer->writeStartElement(dbNamespace, "figure");
            newLine();

            const Atom *current = atom->next()->next()->next();
            skipAhead += 2;

            Q_ASSERT(current->type() == Atom::CaptionLeft);
            generateAtom(current, relative);
            current = current->next();
            ++skipAhead;

            while (current->type() != Atom::CaptionRight) { // The actual caption.
                generateAtom(current, relative);
                current = current->next();
                ++skipAhead;
            }

            Q_ASSERT(current->type() == Atom::CaptionRight);
            generateAtom(current, relative);
            current = current->next();
            ++skipAhead;

            m_closeFigureWrapper = true;
        }

        if (atom->next() && matchAhead(atom->next(), Atom::CaptionLeft)) {
            // If there is a caption, there must be a <db:figure>
            // wrapper starting with the caption.
            Q_ASSERT(atom->next());
            Q_ASSERT(atom->next()->next());
            Q_ASSERT(atom->next()->next()->next());
            Q_ASSERT(atom->next()->next()->next()->next());

            m_writer->writeStartElement(dbNamespace, "figure");
            newLine();

            const Atom *current = atom->next()->next();
            ++skipAhead;

            Q_ASSERT(current->type() == Atom::CaptionLeft);
            generateAtom(current, relative);
            current = current->next();
            ++skipAhead;

            while (current->type() != Atom::CaptionRight) { // The actual caption.
                generateAtom(current, relative);
                current = current->next();
                ++skipAhead;
            }

            Q_ASSERT(current->type() == Atom::CaptionRight);
            generateAtom(current, relative);
            current = current->next();
            ++skipAhead;

            m_closeFigureWrapper = true;
        }

        Q_FALLTHROUGH();
    case Atom::InlineImage: { // inlinemediaobject
        // TODO: [generator-insufficient-structural-abstraction]
        // The structure of the computations for this part of the
        // docbook generation and the same parts in other format
        // generators is the same.
        //
        // The difference, instead, lies in what the generated output
        // is like. A correct abstraction for a generator would take
        // this structural equivalence into account and encapsulate it
        // into a driver for the format generators.
        //
        // This would avoid the replication of content, and the
        // subsequent friction for changes and desynchronization
        // between generators.
        //
        // Review all the generators routines and find the actual
        // skeleton that is shared between them, then consider it when
        // extracting the logic for the generation phase.
        QString tag = atom->type() == Atom::Image ? "mediaobject" : "inlinemediaobject";
        m_writer->writeStartElement(dbNamespace, tag);
        newLine();

        auto maybe_resolved_file{file_resolver.resolve(atom->string())};
        if (!maybe_resolved_file) {
            // TODO: [uncetnralized-admonition][failed-resolve-file]
            relative->location().warning(QStringLiteral("Missing image: %1").arg(atom->string()));

            m_writer->writeStartElement(dbNamespace, "textobject");
            newLine();
            m_writer->writeStartElement(dbNamespace, "para");
            m_writer->writeTextElement(dbNamespace, "emphasis",
                                       "[Missing image " + atom->string() + "]");
            m_writer->writeEndElement(); // para
            newLine();
            m_writer->writeEndElement(); // textobject
            newLine();
        } else {
            ResolvedFile file{*maybe_resolved_file};
            QString file_name{QFileInfo{file.get_path()}.fileName()};

            // TODO: [uncentralized-output-directory-structure]
            Config::copyFile(relative->doc().location(), file.get_path(), file_name, outputDir() + QLatin1String("/images"));

            if (atom->next() && !atom->next()->string().isEmpty()
                && atom->next()->type() == Atom::ImageText) {
                m_writer->writeTextElement(dbNamespace, "alt", atom->next()->string());
                newLine();
            }

            m_writer->writeStartElement(dbNamespace, "imageobject");
            newLine();
            m_writer->writeEmptyElement(dbNamespace, "imagedata");
            // TODO: [uncentralized-output-directory-structure]
            m_writer->writeAttribute("fileref", "images/" + file_name);
            newLine();
            m_writer->writeEndElement(); // imageobject
            newLine();

            // TODO: [uncentralized-output-directory-structure]
            setImageFileName(relative, "images/" + file_name);
        }

        m_writer->writeEndElement(); // [inline]mediaobject
        if (atom->type() == Atom::Image)
            newLine();

        if (m_closeFigureWrapper) {
            m_writer->writeEndElement(); // figure
            newLine();
            m_closeFigureWrapper = false;
        }
    } break;
    case Atom::ImageText:
        break;
    case Atom::ImportantLeft:
    case Atom::NoteLeft:
    case Atom::WarningLeft: {
        QString admonType = atom->typeString().toLower();
        // Remove 'Left' to get the admonition type
        admonType.chop(4);
        m_writer->writeStartElement(dbNamespace, admonType);
        newLine();
        m_writer->writeStartElement(dbNamespace, "para");
        m_inPara = true;
    } break;
    case Atom::ImportantRight:
    case Atom::NoteRight:
    case Atom::WarningRight:
        m_writer->writeEndElement(); // para
        m_inPara = false;
        newLine();
        m_writer->writeEndElement(); // note/important
        newLine();
        break;
    case Atom::LegaleseLeft:
    case Atom::LegaleseRight:
        break;
    case Atom::Link:
    case Atom::NavLink: {
        const Node *node = nullptr;
        QString link = getLink(atom, relative, &node);
        beginLink(link, node, relative); // Ended at Atom::FormattingRight
        skipAhead = 1;
    } break;
    case Atom::LinkNode: {
        const Node *node = CodeMarker::nodeForString(atom->string());
        beginLink(linkForNode(node, relative), node, relative);
        skipAhead = 1;
    } break;
    case Atom::ListLeft:
        if (m_inPara) {
            // The variable m_inPara is not set in a very smart way, because
            // it ignores nesting. This might in theory create false positives
            // here. A better solution would be to track the depth of
            // paragraphs the generator is in, but determining the right check
            // for this condition is far from trivial (think of nested lists).
            m_writer->writeEndElement(); // para
            newLine();
            m_inPara = false;
        }

        if (atom->string() == ATOM_LIST_BULLET) {
            m_writer->writeStartElement(dbNamespace, "itemizedlist");
            newLine();
        } else if (atom->string() == ATOM_LIST_TAG) {
            m_writer->writeStartElement(dbNamespace, "variablelist");
            newLine();
        } else if (atom->string() == ATOM_LIST_VALUE) {
            m_writer->writeStartElement(dbNamespace, "informaltable");
            newLine();
            m_writer->writeStartElement(dbNamespace, "thead");
            newLine();
            m_writer->writeStartElement(dbNamespace, "tr");
            newLine();
            m_writer->writeTextElement(dbNamespace, "th", "Constant");
            newLine();

            m_threeColumnEnumValueTable = isThreeColumnEnumValueTable(atom);
            if (m_threeColumnEnumValueTable && relative->nodeType() == Node::Enum) {
                // With three columns, if not in \enum topic, skip the value column
                m_writer->writeTextElement(dbNamespace, "th", "Value");
                newLine();
            }

            if (!isOneColumnValueTable(atom)) {
                m_writer->writeTextElement(dbNamespace, "th", "Description");
                newLine();
            }

            m_writer->writeEndElement(); // tr
            newLine();
            m_writer->writeEndElement(); // thead
            newLine();
        } else { // No recognized list type.
            m_writer->writeStartElement(dbNamespace, "orderedlist");

            if (atom->next() != nullptr && atom->next()->string().toInt() > 1)
                m_writer->writeAttribute("startingnumber", atom->next()->string());

            if (atom->string() == ATOM_LIST_UPPERALPHA)
                m_writer->writeAttribute("numeration", "upperalpha");
            else if (atom->string() == ATOM_LIST_LOWERALPHA)
                m_writer->writeAttribute("numeration", "loweralpha");
            else if (atom->string() == ATOM_LIST_UPPERROMAN)
                m_writer->writeAttribute("numeration", "upperroman");
            else if (atom->string() == ATOM_LIST_LOWERROMAN)
                m_writer->writeAttribute("numeration", "lowerroman");
            else // (atom->string() == ATOM_LIST_NUMERIC)
                m_writer->writeAttribute("numeration", "arabic");

            newLine();
        }
        m_inList++;
        break;
    case Atom::ListItemNumber:
        break;
    case Atom::ListTagLeft:
        if (atom->string() == ATOM_LIST_TAG) {
            m_writer->writeStartElement(dbNamespace, "varlistentry");
            newLine();
            m_writer->writeStartElement(dbNamespace, "item");
        } else { // (atom->string() == ATOM_LIST_VALUE)
            std::pair<QString, int> pair = getAtomListValue(atom);
            skipAhead = pair.second;

            m_writer->writeStartElement(dbNamespace, "tr");
            newLine();
            m_writer->writeStartElement(dbNamespace, "td");
            newLine();
            m_writer->writeStartElement(dbNamespace, "para");
            if (m_useITS)
                m_writer->writeAttribute(itsNamespace, "translate", "no");
            generateEnumValue(pair.first, relative);
            m_writer->writeEndElement(); // para
            newLine();
            m_writer->writeEndElement(); // td
            newLine();

            if (relative->nodeType() == Node::Enum) {
                const auto enume = static_cast<const EnumNode *>(relative);
                QString itemValue = enume->itemValue(atom->next()->string());

                m_writer->writeStartElement(dbNamespace, "td");
                if (itemValue.isEmpty())
                    m_writer->writeCharacters("?");
                else {
                    m_writer->writeStartElement(dbNamespace, "code");
                    if (m_useITS)
                        m_writer->writeAttribute(itsNamespace, "translate", "no");
                    m_writer->writeCharacters(itemValue);
                    m_writer->writeEndElement(); // code
                }
                m_writer->writeEndElement(); // td
                newLine();
            }
        }
        m_inList++;
        break;
    case Atom::SinceTagRight:
        if (atom->string() == ATOM_LIST_TAG) {
            m_writer->writeEndElement(); // item
            newLine();
        }
        break;
    case Atom::ListTagRight:
        if (m_inList > 0 && atom->string() == ATOM_LIST_TAG) {
            m_writer->writeEndElement(); // item
            newLine();
            m_inList = false;
        }
        break;
    case Atom::ListItemLeft:
        if (m_inList > 0) {
            m_inListItemLineOpen = false;
            if (atom->string() == ATOM_LIST_TAG) {
                m_writer->writeStartElement(dbNamespace, "listitem");
                newLine();
                m_writer->writeStartElement(dbNamespace, "para");
                m_inPara = true;
            } else if (atom->string() == ATOM_LIST_VALUE) {
                if (m_threeColumnEnumValueTable) {
                    if (matchAhead(atom, Atom::ListItemRight)) {
                        m_writer->writeEmptyElement(dbNamespace, "td");
                        newLine();
                        m_inListItemLineOpen = false;
                    } else {
                        m_writer->writeStartElement(dbNamespace, "td");
                        newLine();
                        m_inListItemLineOpen = true;
                    }
                }
            } else {
                m_writer->writeStartElement(dbNamespace, "listitem");
                newLine();
            }
            // Don't skip a paragraph, DocBook requires them within list items.
        }
        break;
    case Atom::ListItemRight:
        if (m_inList > 0) {
            if (atom->string() == ATOM_LIST_TAG) {
                m_writer->writeEndElement(); // para
                m_inPara = false;
                newLine();
                m_writer->writeEndElement(); // listitem
                newLine();
                m_writer->writeEndElement(); // varlistentry
                newLine();
            } else if (atom->string() == ATOM_LIST_VALUE) {
                if (m_inListItemLineOpen) {
                    m_writer->writeEndElement(); // td
                    newLine();
                    m_inListItemLineOpen = false;
                }
                m_writer->writeEndElement(); // tr
                newLine();
            } else {
                m_writer->writeEndElement(); // listitem
                newLine();
            }
        }
        break;
    case Atom::ListRight:
        // Depending on atom->string(), closing a different item:
        // - ATOM_LIST_BULLET: itemizedlist
        // - ATOM_LIST_TAG: variablelist
        // - ATOM_LIST_VALUE: informaltable
        // - ATOM_LIST_NUMERIC: orderedlist
        m_writer->writeEndElement();
        newLine();
        m_inList--;
        break;
    case Atom::Nop:
        break;
    case Atom::ParaLeft:
        m_writer->writeStartElement(dbNamespace, "para");
        m_inPara = true;
        break;
    case Atom::ParaRight:
        endLink();
        if (m_inPara) {
            m_writer->writeEndElement(); // para
            newLine();
            m_inPara = false;
        }
        break;
    case Atom::QuotationLeft:
        m_writer->writeStartElement(dbNamespace, "blockquote");
        m_inBlockquote = true;
        break;
    case Atom::QuotationRight:
        m_writer->writeEndElement(); // blockquote
        newLine();
        m_inBlockquote = false;
        break;
    case Atom::RawString: {
        // Many of these transformations are only useful when dealing with
        // older versions of Qt, with their idiosyncrasies. However, they
        // also make qdoc hardened against new problematic raw strings.
        bool hasRewrittenString = false;
        const QString &str = atom->string().trimmed();

        static QHash<QString, QString> entitiesMapping;
        if (entitiesMapping.isEmpty()) {
            // These mappings come from qtbase\doc\global\macros.qdocconf.
            entitiesMapping["&aacute;"] = "&#225;";
            entitiesMapping["&Aring;"] = "&#197;";
            entitiesMapping["&aring;"] = "&#229;";
            entitiesMapping["&Auml;"] = "&#196;";
            entitiesMapping["&copyright;"] = "&#169;";
            entitiesMapping["&eacute;"] = "&#233;";
            entitiesMapping["&iacute;"] = "&#237;";
            entitiesMapping["&oslash;"] = "&#248;";
            entitiesMapping["&ouml;"] = "&#246;";
            entitiesMapping["&rarrow;"] = "&#8594;";
            entitiesMapping["&uuml;"] = "&#252;";
            entitiesMapping["&mdash;"] = "&#8212;";
            entitiesMapping["&Pi;"] = "&#928;";
        }

        if (str.startsWith(R"(<link rel="stylesheet" type="text/css")")) {
            hasRewrittenString = true;
            m_writer->writeComment(str);
        } else if (str == "\\sup{*}") {
            hasRewrittenString = true;
            m_writer->writeTextElement(dbNamespace, "superscript", "*");
        } else if (str.startsWith("<sup>") && str.endsWith("</sup>")) {
            hasRewrittenString = true;
            m_writer->writeTextElement(dbNamespace, "superscript", str.mid(5, str.size() - 5 - 6));
        } else if (str.startsWith("<div class=\"video")) {
            hasRewrittenString = true;

            // Sequence of atoms:
            // - RawString (this one): <div class="video">\n<a href="https://www.youtube.com/watch/?v=
            // - String: video ID
            // - RawString: ">\n<img src="images/
            // - String: video ID, again (but with an associated image)
            // - RawString: .jpg" title="Click to play in a browser" /></a>\n</div>\n
            // TODO: No call to file resolver, like the other generators. Would it be required?
            //      auto maybe_resolved_file{file_resolver.resolve(atom->string())};
            Q_ASSERT(atom->next());
            Q_ASSERT(atom->next()->next());
            Q_ASSERT(atom->next()->next()->next());
            Q_ASSERT(atom->next()->next()->next()->next());
            Q_ASSERT(atom->next()->type() == Atom::String);
            Q_ASSERT(atom->next()->next()->next()->type() == Atom::String);
            skipAhead += 4;

            const QString &videoID = atom->next()->string();
            const QString &imageID = atom->next()->next()->next()->string();

            m_writer->writeStartElement(dbNamespace, "mediaobject");
            newLine();

            m_writer->writeStartElement(dbNamespace, "videoobject");
            newLine();
            m_writer->writeStartElement(dbNamespace, "videodata");
            m_writer->writeAttribute("fileref", videoID);
            newLine();
            m_writer->writeEndElement(); // videodata
            newLine();
            m_writer->writeEndElement(); // videoobject
            newLine();

            m_writer->writeStartElement(dbNamespace, "imageobject");
            newLine();
            m_writer->writeStartElement(dbNamespace, "imagedata");
            m_writer->writeAttribute("fileref", "images/" + imageID + ".jpg");
            newLine();
            m_writer->writeEndElement(); // imagedata
            newLine();
            m_writer->writeEndElement(); // imageobject
            newLine();

            m_writer->writeEndElement(); // mediaobject
            newLine();
        } else if (str.startsWith("<h") && str.size() >= 9) { // <hX></hX>: 9 characters.
            // If qdoc has just closed a section, suppose that the person
            // writing this RawString knows what they are doing generate a
            // section. Otherwise, create a bridgehead.
            bool hasJustClosedASection = !m_writer->device()->isSequential() &&
                m_writer->device()->readAll().trimmed().endsWith("</db:section>");

            // Parse the raw string. If nothing matches, no title is found,
            // and no rewriting is performed.
            QChar level = str[2];
            QString title {""};
            QString id {""};

            if (str.startsWith("<h" + level + ">") && str.endsWith("</h" + level + ">")) {
                title = str.mid(4, str.size() - 9);
            } else if (str.startsWith("<h" + level + " id=") && str.endsWith("</h" + level + ">")) {
                // <hX id=: 7 characters.
                QString idToEndTag = str.mid(8, str.size() - 7 - 5);
                id = idToEndTag.split("\"")[0];
                title = idToEndTag.remove(0, id.size() + 2).chopped(1);
            }

            // Output the DocBook equivalent.
            if (!title.isEmpty()) {
                hasRewrittenString = true;

                if (hasJustClosedASection) {
                    startSection(id, title);
                    m_closeSectionAfterRawTitle = true;
                } else {
                    m_writer->writeStartElement(dbNamespace, "bridgehead");
                    m_writer->writeAttribute("renderas", "sect" + level);
                    writeXmlId(id);
                    m_writer->writeCharacters(title);
                    m_writer->writeEndElement(); // bridgehead
                }

                // If there is an anchor just after with the same ID, skip it.
                if (matchAhead(atom, Atom::Target)
                    && Utilities::asAsciiPrintable(atom->next()->string()) == id) {
                    ++skipAhead;
                }
            } else {
                // The formatting is not recognized: it starts with a tittle,
                // then some unknown stuff. It's highly likely some qdoc
                // example: output that as raw HTML in DocBook too.
                writeRawHtml(str);
                hasRewrittenString = true;
            }
        } else if (// Formatting of images.
                str.startsWith(R"(<div class="table"><table style="background:transparent; border:0px">)") ||
                str.startsWith(R"(</td><td style="border:0px">)") ||
                str.simplified().startsWith("</td></tr> </table></div>") ||
                str.startsWith(R"(<br style="clear: both" />)") ||
                str.startsWith(R"(<div style="float: left; margin-right: 2em">)") ||
                str.startsWith(R"(<div style="float: right; margin-left: 2em">)") ||
                str.startsWith("</div>") ||
                str.startsWith("<span></span>") ||
                str.simplified().startsWith("</td></tr> </table></div>") ||
                str.startsWith(R"(<br style="clear: both" />)") ||
                // Other formatting, only for QMake.
                str == "<br />") {
            // Ignore this part, as it's only for formatting of images.
            hasRewrittenString = true;
        } else if (str.startsWith(R"(<div style="padding:10px;color:#fff;background)") &&
                matchAhead(atom, Atom::String) && matchAhead(atom->next(), Atom::RawString) &&
                matchAhead(atom->next()->next(), Atom::String) &&
                matchAhead(atom->next()->next()->next(), Atom::RawString) &&
                matchAhead(atom->next()->next()->next()->next(), Atom::String) &&
                matchAhead(atom->next()->next()->next()->next()->next(), Atom::RawString)) {
            hasRewrittenString = true;
            skipAhead += 6;

            const QString color = atom->next()->string(); // == atom->next()->next()->next()->string()
            const QString text = atom->next()->next()->next()->next()->next()->string();

            m_writer->writeStartElement(dbNamespace, "phrase");
            m_writer->writeAttribute("role", "color:" + color);
            m_writer->writeCharacters(color);
            m_writer->writeCharacters(" ");
            if (text.isEmpty())
                m_writer->writeCharacters(text);
            else
                m_writer->writeCharacters("&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;");
            m_writer->writeEndElement(); // phrase
        }
        // The following two cases handle some specificities of the documentation of Qt Quick
        // Controls 2. A small subset of pages is involved, as of Qt 6.4.0:
        // qtquickcontrols2-imagine, qtquickcontrols2-macos, qtquickcontrols2-material,
        // qtquickcontrols2-universal, qtquickcontrols2-windows. The string to rewrite looks like
        // the following, with XML comments to indicate the start of atoms:
        //      <!--RawString--><table class="alignedsummary"><tbody><tr><td class="memItemLeft
        //      rightAlign topAlign"> Import Statement:</td><td class="memItemRight bottomAlign">
        //      import
        //      <!--String-->QtQuick.Controls.Imagine 2.12
        //      <!--RawString--></td></tr><tr><td class="memItemLeft rightAlign topAlign">
        //      Since:</td> <td class="memItemRight bottomAlign">
        //      <!--String-->Qt 5.10
        //      <!--RawString--></td></tr></tbody></table>
        // The structure being fixed, a rigid but simple solution is implemented: don't parse the
        // table, simply output the expected set of tags. An alternative would be to parse the HTML
        // table and to replicate the tags. The output is identical to
        // DocBookGenerator::generateQmlRequisites.
        else if (
                str.startsWith(
                        R"(<table class="alignedsummary"><tbody><tr><td class="memItemLeft rightAlign topAlign"> Import Statement:)")
                && matchAhead(atom, Atom::String) && matchAhead(atom->next(), Atom::RawString)
                && matchAhead(atom->next()->next(), Atom::String)
                && matchAhead(atom->next()->next()->next(), Atom::RawString)) {
            m_rewritingCustomQmlModuleSummary = true;
            hasRewrittenString = true;

            m_writer->writeStartElement(dbNamespace, "variablelist");
            newLine();

            generateStartRequisite("Import Statement");
            m_writer->writeCharacters("import ");
        } else if (m_rewritingCustomQmlModuleSummary) {
            if (str.startsWith(
                        R"(</td></tr><tr><td class="memItemLeft rightAlign topAlign"> Since:)")) {
                generateEndRequisite();
                generateStartRequisite("Since");

                hasRewrittenString = true;
            } else if (str.startsWith(R"(</td></tr></tbody></table>)")) {
                m_rewritingCustomQmlModuleSummary = false;
                hasRewrittenString = true;

                generateEndRequisite();
                m_writer->writeEndElement(); // variablelist
                newLine();
            }
        }
        // Another idiosyncrasy for this module:
        //      <!--RawString--><div class="qmlproto"><table class="qmlname"><tbody><tr valign="top"
        //      class="odd" id="
        //      <!--String-->universal-accent-attached-prop
        //      <!--RawString-->"><td class="tblQmlPropNode"><p><span class="name">
        //      <!--String-->Universal.accent
        //      <!--RawString--></span> : <span class="type">
        //      <!--String-->color
        //      <!--RawString--></span></p></td></tr></tbody></table></div>
        // Several variants of this template exist, with more than three String between the
        // RawString. They are defined in
        // qtdeclarative\src\quickcontrols2\doc\qtquickcontrols.qdocconf.
        else if (
                str.startsWith(
                        R"(<div class="qmlproto"><table class="qmlname"><tbody><tr valign="top" class="odd" id=")")
                && matchAhead(atom, Atom::String) && matchAhead(atom->next(), Atom::RawString)
                && matchAhead(atom->next()->next(), Atom::String)
                && matchAhead(atom->next()->next()->next(), Atom::RawString)) {
            hasRewrittenString = true;
            m_hasSection = true;

            // Determine which case occurs (property or method).
            const bool isStyleProperty = atom->next()->next()->string().startsWith(
                    R"("><td class="tblQmlPropNode"><p><span class="name">)");
            const bool isStyleMethod =
                    !isStyleProperty; // atom->next()->next()->string().startsWith(R"("><td
                                      // class="tblQmlFuncNode"><p><span class="type">)")

            // Parse the sequence of atoms.
            const Atom *nextStringAtom =
                    atom->next(); // Invariant: ->type() == Atom::String (except after parsing).
            const QString id = nextStringAtom->string();
            skipAhead += 2;
            QString name;
            QString type;
            QString arg1;
            QString type1;
            QString arg2;
            QString type2;

            if (isStyleProperty) {
                nextStringAtom = nextStringAtom->next()->next();
                name = nextStringAtom->string();
                skipAhead += 2;

                nextStringAtom = nextStringAtom->next()->next();
                type = nextStringAtom->string();
                skipAhead += 2;
            } else if (isStyleMethod) {
                nextStringAtom = nextStringAtom->next()->next();
                type = nextStringAtom->string();
                skipAhead += 2;

                nextStringAtom = nextStringAtom->next()->next();
                type = nextStringAtom->string();
                skipAhead += 2;

                nextStringAtom = nextStringAtom->next()->next();
                arg1 = nextStringAtom->string();
                skipAhead += 2;

                nextStringAtom = nextStringAtom->next()->next();
                type1 = nextStringAtom->string();
                skipAhead += 2;

                if (matchAhead(nextStringAtom, Atom::RawString)
                    && matchAhead(nextStringAtom->next(), Atom::String)
                    && matchAhead(nextStringAtom->next()->next(), Atom::RawString)
                    && matchAhead(nextStringAtom->next()->next()->next(), Atom::String)
                    && matchAhead(nextStringAtom->next()->next()->next()->next(),
                                  Atom::RawString)) {
                    nextStringAtom = nextStringAtom->next()->next();
                    arg2 = nextStringAtom->string();
                    skipAhead += 2;

                    nextStringAtom = nextStringAtom->next()->next();
                    type2 = nextStringAtom->string();
                    skipAhead += 2;
                }

                // For now (Qt 6.4.0), the macro is only defined up to two arguments: \stylemethod
                // and \stylemethod2.
            }

            // Write the corresponding DocBook.
            // This should be wrapped in a section, but there is no mechanism to check for
            // \endstyleproperty or \endstylemethod within qdoc (it must be done at the macro
            // level), hence the bridgehead.
            QString title;
            if (isStyleProperty) {
                title = name + " : " + type;
            } else if (isStyleMethod) {
                title = type + " " + name;
            }

            m_writer->writeStartElement(dbNamespace, "bridgehead");
            m_writer->writeAttribute("renderas", "sect2");
            writeXmlId(id);
            m_writer->writeCharacters(title);
            m_writer->writeEndElement(); // bridgehead
            newLine();

            if (m_useDocBook52) {
                if (isStyleProperty) {
                    m_writer->writeStartElement(dbNamespace, "fieldsynopsis");

                    m_writer->writeTextElement(dbNamespace, "type", type);
                    newLine();
                    m_writer->writeTextElement(dbNamespace, "varname", name);
                    newLine();

                    m_writer->writeEndElement(); // fieldsynopsis
                } else if (isStyleMethod) {
                    m_writer->writeStartElement(dbNamespace, "methodsynopsis");

                    m_writer->writeTextElement(dbNamespace, "type", type);
                    newLine();
                    m_writer->writeTextElement(dbNamespace, "methodname", name);
                    newLine();

                    if (!arg1.isEmpty() && !type1.isEmpty()) {
                        m_writer->writeStartElement(dbNamespace, "methodparam");
                        newLine();
                        m_writer->writeTextElement(dbNamespace, "type", type1);
                        newLine();
                        m_writer->writeTextElement(dbNamespace, "parameter", arg1);
                        newLine();
                        m_writer->writeEndElement(); // methodparam
                        newLine();
                    }
                    if (!arg2.isEmpty() && !type2.isEmpty()) {
                        m_writer->writeStartElement(dbNamespace, "methodparam");
                        newLine();
                        m_writer->writeTextElement(dbNamespace, "type", type2);
                        newLine();
                        m_writer->writeTextElement(dbNamespace, "parameter", arg2);
                        newLine();
                        m_writer->writeEndElement(); // methodparam
                        newLine();
                    }

                    m_writer->writeEndElement(); // methodsynopsis
                }
            }
        }
        // This time, a specificity of Qt Virtual Keyboard to embed SVG images. Typically, there are
        // several images at once with the same encoding.
        else if (
                str.startsWith(
                        R"(<div align="center"><figure><svg xmlns:svg="http://www.w3.org/2000/svg" xmlns="http://www.w3.org/2000/svg")")) {
            const QStringList images = str.split("</div>", Qt::SkipEmptyParts, Qt::CaseInsensitive);

            for (const QString& image : images) {
                // Find the caption.
                const QStringList parts = image.split("</svg>");
                const QString svgImage = "<svg" + parts[0].split("<svg")[1] + "</svg>";
                const QString caption = parts[1].split("<figcaption>")[1].split("</figcaption>")[0];

                // Output the DocBook equivalent.
                m_writer->writeStartElement(dbNamespace, "figure");
                newLine();
                m_writer->writeStartElement(dbNamespace, "title");
                m_writer->writeCharacters(caption);
                m_writer->writeEndElement(); // title
                newLine();
                m_writer->writeStartElement(dbNamespace, "mediaobject");
                newLine();
                m_writer->writeStartElement(dbNamespace, "imageobject");
                newLine();
                m_writer->writeStartElement(dbNamespace, "imagedata");
                newLine();
                m_writer->device()->write(svgImage.toUtf8()); // SVG image as raw XML.
                m_writer->writeEndElement(); // imagedata
                newLine();
                m_writer->writeEndElement(); // imageobject
                newLine();
                m_writer->writeEndElement(); // mediaobject
                newLine();
                m_writer->writeEndElement(); // figure
                newLine();
            }

            hasRewrittenString = true;
        }
        // This time, a specificity of Qt Virtual Keyboard to embed SVG images. Typically, there are
        // several images at once with the same encoding.
        else if (
                str.startsWith(
                        R"(<div align="center"><figure><svg xmlns:svg="http://www.w3.org/2000/svg" xmlns="http://www.w3.org/2000/svg")")) {
            const QStringList images = str.split("</div>", Qt::SkipEmptyParts, Qt::CaseInsensitive);

            for (const QString &image : images) {
                // Find the caption.
                const QStringList parts = image.split("</svg>");
                const QString svgImage = "<svg" + parts[0].split("<svg")[1] + "</svg>";
                const QString caption = parts[1].split("<figcaption>")[1].split("</figcaption>")[0];

                // Output the DocBook equivalent.
                m_writer->writeStartElement(dbNamespace, "figure");
                newLine();
                m_writer->writeStartElement(dbNamespace, "title");
                m_writer->writeCharacters(caption);
                m_writer->writeEndElement(); // title
                newLine();
                m_writer->writeStartElement(dbNamespace, "mediaobject");
                newLine();
                m_writer->writeStartElement(dbNamespace, "imageobject");
                newLine();
                m_writer->writeStartElement(dbNamespace, "imagedata");
                newLine();
                m_writer->device()->write(svgImage.toUtf8()); // SVG image as raw XML.
                m_writer->writeEndElement(); // imagedata
                newLine();
                m_writer->writeEndElement(); // imageobject
                newLine();
                m_writer->writeEndElement(); // mediaobject
                newLine();
                m_writer->writeEndElement(); // figure
                newLine();
            }

            hasRewrittenString = true;
        }
        // For ActiveQt, there is some raw HTML that has no meaningful
        // translation into DocBook.
        else if (str.trimmed().toLower().startsWith(R"(<script language="javascript">)")
                 || str.trimmed().toLower().startsWith(R"(<script language="vbscript">)")
                 || str.trimmed().toLower().startsWith("<object id=")) {
            writeRawHtml(str);
            hasRewrittenString = true;
        }
        // Raw HTML encoding of some tables. Perform some basic soundness
        // checks to ensure the conversion has some chance of success.
        else if (str.startsWith("<table ") &&
                str.count("<tr") == str.count("</tr") &&
                str.count("<td") == str.count("</td") &&
                str.count("<tr") > 0 && str.count("</td") > 0) {
            QStringList tables = str.split("</table>", Qt::SkipEmptyParts);
            for (QString table : tables) {
                // Several changes:
                // - name spaces for each element
                // - use of informaltable (no caption) instead of table
                // - DocBook-compliant encoding of the background cell color.
                //   In case the background color is given by value instead of
                //   name (like "#d0d0d0" instead of "gray"), remove the sharp
                //   so that the output class is still allowed by CSS.
                // - use of emphasis instead of HTML b or i, DocBook link
                //   instead of HTML a, DocBook para instead of HTML p
                // - removal of the "nowrap" and "align" attributes (no DocBook
                //   encoding), correction of rowspan and colspan attributes
                // - adding a </tbody> in case it is opened and never closed
                // - encoding of images and titles (as bridgeheads)
                // - remove line feeds
                table = table.replace("</", "</db:");
                table = table.replace("<", "<db:");
                table = table.replace("<db:/db:", "</db:");

                table = table.replace("<db:table", "<db:informaltable");

                table = table.replace("<db:b>", R"(<db:emphasis role="bold">)");
                table = table.replace("</db:b>", "</db:emphasis>");
                table = table.replace("<db:i>", "<db:emphasis>");
                table = table.replace("</db:i>", "</db:emphasis>");

                table = table.replace("<db:a href=", "<db:link xlink:href=");
                table = table.replace("</db:a>", "</db:link>");

                table = table.replace("<db:p>", "<db:para>");
                table = table.replace("</db:p>", "</db:para>");

                table = table.replace("<db:br />", QString());
                table = table.replace("<db:br/>", QString());

                static const QRegularExpression re1(R"regex(<db:h(\d).*)>(.*)</db:h(\d)>)regex");
                table.replace(re1,
                    R"xml(<db:bridgehead renderas="sect\1">\2</bridgehead>)xml");
                // Expecting \1 == \3.

                table = table.replace(R"( nowrap="nowrap")", QString());
                table = table.replace(R"( align="center")", QString());
                static const QRegularExpression re2(R"regex((row|col)span="\s+(.*)")regex");
                table.replace(re2,
                    R"(\1span="\2")");

                static const QRegularExpression re3(R"regex(<db:td (.*)bgcolor="#(.*)"(.*)>(.*)</db:td>)regex");
                table.replace(re3,
                    R"xml(<db:td \1 class="bgcolor-\2" \3><?dbhtml bgcolor="\2" ?><?dbfo bgcolor="\2" ?>\4</db:td>)xml");
                static const QRegularExpression re4(R"regex(<db:td (.*)bgcolor="(.*)"(.*)>(.*)</db:td>)regex");
                table.replace(re4,
                    R"xml(<db:td \1 class="bgcolor-\2" \3><?dbhtml bgcolor="\2" ?><?dbfo bgcolor="\2" ?>\4</db:td>)xml");
                static const QRegularExpression re5(R"regex(<db:tr (.*)bgcolor="#(.*)"(.*)>)regex");
                table.replace(re5,
                    R"xml(<db:tr \1 class="bgcolor-\2" \3><?dbhtml bgcolor="\2" ?><?dbfo bgcolor="\2" ?>)xml");
                static const QRegularExpression re6(R"regex(<db:tr (.*)bgcolor="(.*)"(.*)>³)regex");
                table.replace(re6,
                    R"xml(<db:tr \1 class="bgcolor-\2" \3><?dbhtml bgcolor="\2" ?><?dbfo bgcolor="\2" ?>)xml");

                static const QRegularExpression re7(R"regex(<db:img src="(.*)" alt="(.*)"\s*/>)regex");
                table.replace(re7,
                    R"xml(<db:figure>
<db:title>\2</db:title>
<db:mediaobject>
<db:imageobject>
<db:imagedata fileref="\1"/>
</db:imageobject>
</db:mediaobject>
</db:figure>)xml");

                m_writer->device()->write(table.toUtf8());

                // Finalize the table by writing the end tags.
                // m_writer->writeEndElement cannot be used, as the opening
                // tags are output directly through m_writer->device().
                if (table.contains("<db:tbody") && !table.contains("</db:tbody")) {
                    m_writer->device()->write("</db:tbody>\n");
                }
                m_writer->device()->write("</db:informaltable>\n");
            }

            hasRewrittenString = true;
        }

        // No rewriting worked: for blockquotes, this is likely a qdoc example.
        // Use some programlisting to encode this raw HTML.
        if (!hasRewrittenString && m_inBlockquote) {
            writeRawHtml(atom->string());
            hasRewrittenString = true;
        } else {
            // Deal with some HTML entities to convert into XML.
            // This implementation complements the entities in qtbase\doc\global\macros.qdocconf,
            // because this code focuses on the RawString atom, while the configuration only works
            // for macros that generate HTML/XML entities.
            auto end = entitiesMapping.keyEnd();
            for (auto it = entitiesMapping.keyBegin(); it != end; ++it) {
                if (const QString &entity = *it; str.startsWith(entity)) {
                    QString rewrittenString = str;
                    rewrittenString.replace(entity, entitiesMapping.value(entity));

                    m_writer->device()->write(rewrittenString.toUtf8());

                    hasRewrittenString = true;
                }
            }
        }

        // The RawString may be a macro specialized for DocBook, in which case no escaping is expected.
        // QXmlStreamWriter always write UTF-8 contents.
        if (!hasRewrittenString)
            m_writer->device()->write(atom->string().toUtf8()); // str has been trimmed.
    }
        break;
    case Atom::SectionLeft:
        m_hasSection = true;

        currentSectionLevel = atom->string().toInt() + hOffset(relative);
        // Level 1 is dealt with at the header level (info tag).
        if (currentSectionLevel > 1) {
            // Unfortunately, SectionRight corresponds to the end of any section,
            // i.e. going to a new section, even deeper.
            while (!sectionLevels.empty() && sectionLevels.top() >= currentSectionLevel) {
                sectionLevels.pop();
                m_writer->writeEndElement(); // section
                newLine();
            }

            sectionLevels.push(currentSectionLevel);

            m_writer->writeStartElement(dbNamespace, "section");
            writeXmlId(Utilities::asAsciiPrintable(Text::sectionHeading(atom).toString()));
            newLine();
            // Unlike startSectionBegin, don't start a title here.
        }

        if (matchAhead(atom, Atom::SectionHeadingLeft) &&
                matchAhead(atom->next(), Atom::String) &&
                matchAhead(atom->next()->next(), Atom::SectionHeadingRight) &&
                matchAhead(atom->next()->next()->next(), Atom::SectionRight) &&
                !atom->next()->next()->next()->next()->next()) {
            // A lonely section at the end of the document indicates that a
            // generated list of some sort should be within this section.
            // Close this section later on, in generateFooter().
            generateAtom(atom->next(), relative);
            generateAtom(atom->next()->next(), relative);
            generateAtom(atom->next()->next()->next(), relative);

            m_closeSectionAfterGeneratedList = true;
            skipAhead += 4;
            sectionLevels.pop();
        }

        if (!matchAhead(atom, Atom::SectionHeadingLeft)) {
            // No section title afterwards, make one up. This likely indicates a problem in the original documentation.
            m_writer->writeTextElement(dbNamespace, "title", "");
        }
        break;
    case Atom::SectionRight:
        // All the logic about closing sections is done in the SectionLeft case
        // and generateFooter() for the end of the page.
        break;
    case Atom::SectionHeadingLeft:
        // Level 1 is dealt with at the header level (info tag).
        if (currentSectionLevel > 1) {
            m_writer->writeStartElement(dbNamespace, "title");
            m_inSectionHeading = true;
        }
        break;
    case Atom::SectionHeadingRight:
        // Level 1 is dealt with at the header level (info tag).
        if (currentSectionLevel > 1) {
            m_writer->writeEndElement(); // title
            newLine();
            m_inSectionHeading = false;
        }
        break;
    case Atom::SidebarLeft:
        m_writer->writeStartElement(dbNamespace, "sidebar");
        break;
    case Atom::SidebarRight:
        m_writer->writeEndElement(); // sidebar
        newLine();
        break;
    case Atom::String:
        if (m_inLink && !m_inContents && !m_inSectionHeading)
            generateLink(atom);
        else
            m_writer->writeCharacters(atom->string());
        break;
    case Atom::TableLeft: {
        std::pair<QString, QString> pair = getTableWidthAttr(atom);
        QString attr = pair.second;
        QString width = pair.first;

        if (m_inPara) {
            m_writer->writeEndElement(); // para or blockquote
            newLine();
            m_inPara = false;
        }

        m_tableHeaderAlreadyOutput = false;

        m_writer->writeStartElement(dbNamespace, "informaltable");
        m_writer->writeAttribute("style", attr);
        if (!width.isEmpty())
            m_writer->writeAttribute("width", width);
        newLine();
    } break;
    case Atom::TableRight:
        m_tableWidthAttr = {"", ""};
        m_writer->writeEndElement(); // table
        newLine();
        break;
    case Atom::TableHeaderLeft: {
        if (matchAhead(atom, Atom::TableHeaderRight)) {
            ++skipAhead;
            break;
        }

        if (m_tableHeaderAlreadyOutput) {
            // Headers are only allowed at the beginning of the table: close
            // the table and reopen one.
            m_writer->writeEndElement(); // table
            newLine();

            const QString &attr = m_tableWidthAttr.second;
            const QString &width = m_tableWidthAttr.first;

            m_writer->writeStartElement(dbNamespace, "informaltable");
            m_writer->writeAttribute("style", attr);
            if (!width.isEmpty())
                m_writer->writeAttribute("width", width);
            newLine();
        } else {
            m_tableHeaderAlreadyOutput = true;
        }

        const Atom *next = atom->next();
        QString id{""};
        if (matchAhead(atom, Atom::Target)) {
            id = Utilities::asAsciiPrintable(next->string());
            next = next->next();
            ++skipAhead;
        }

        m_writer->writeStartElement(dbNamespace, "thead");
        newLine();
        m_writer->writeStartElement(dbNamespace, "tr");
        writeXmlId(id);
        newLine();
        m_inTableHeader = true;

        if (!matchAhead(atom, Atom::TableItemLeft)) {
            m_closeTableCell = true;
            m_writer->writeStartElement(dbNamespace, "td");
            newLine();
        }
    }
        break;
    case Atom::TableHeaderRight:
        if (m_closeTableCell) {
            m_closeTableCell = false;
            m_writer->writeEndElement(); // td
            newLine();
        }

        m_writer->writeEndElement(); // tr
        newLine();
        if (matchAhead(atom, Atom::TableHeaderLeft)) {
            skipAhead = 1;
            m_writer->writeStartElement(dbNamespace, "tr");
            newLine();
        } else {
            m_writer->writeEndElement(); // thead
            newLine();
            m_inTableHeader = false;
        }
        break;
    case Atom::TableRowLeft: {
        if (matchAhead(atom, Atom::TableRowRight)) {
            skipAhead = 1;
            break;
        }

        QString id{""};
        bool hasTarget {false};
        if (matchAhead(atom, Atom::Target)) {
            id = Utilities::asAsciiPrintable(atom->next()->string());
            ++skipAhead;
            hasTarget = true;
        }

        m_writer->writeStartElement(dbNamespace, "tr");
        writeXmlId(id);

        if (atom->string().isEmpty()) {
            m_writer->writeAttribute("valign", "top");
        } else {
            // Basic parsing of attributes, should be enough. The input string (atom->string())
            // looks like:
            //      arg1="val1" arg2="val2"
            QStringList args = atom->string().split("\"", Qt::SkipEmptyParts);
            //      arg1=, val1, arg2=, val2,
            //      \-- 1st --/  \-- 2nd --/  \-- remainder
            const int nArgs = args.size();

            if (nArgs % 2) {
                // Problem...
                relative->doc().location().warning(
                        QStringLiteral("Error when parsing attributes for the table: got \"%1\"")
                                .arg(atom->string()));
            }
            for (int i = 0; i + 1 < nArgs; i += 2) {
                // args.at(i): name of the attribute being set.
                // args.at(i + 1): value of the said attribute.
                const QString &attr = args.at(i).chopped(1);
                if (attr == "id") { // Too bad if there is an anchor later on
                    // (currently never happens).
                    writeXmlId(args.at(i + 1));
                } else {
                    m_writer->writeAttribute(attr, args.at(i + 1));
                }
            }
        }
        newLine();

        // If there is nothing in this row, close it right now. There might be keywords before the row contents.
        bool isRowEmpty = hasTarget ? !matchAhead(atom->next(), Atom::TableItemLeft) : !matchAhead(atom, Atom::TableItemLeft);
        if (isRowEmpty && matchAhead(atom, Atom::Keyword)) {
            const Atom* next = atom->next();
            while (matchAhead(next, Atom::Keyword))
                next = next->next();
            isRowEmpty = !matchAhead(next, Atom::TableItemLeft);
        }

        if (isRowEmpty) {
            m_closeTableRow = true;
            m_writer->writeEndElement(); // td
            newLine();
        }
    }
        break;
    case Atom::TableRowRight:
        if (m_closeTableRow) {
            m_closeTableRow = false;
            m_writer->writeEndElement(); // td
            newLine();
        }

        m_writer->writeEndElement(); // tr
        newLine();
        break;
    case Atom::TableItemLeft:
        m_writer->writeStartElement(dbNamespace, m_inTableHeader ? "th" : "td");

        for (int i = 0; i < atom->count(); ++i) {
            const QString &p = atom->string(i);
            if (p.contains('=')) {
                QStringList lp = p.split(QLatin1Char('='));
                m_writer->writeAttribute(lp.at(0), lp.at(1));
            } else {
                QStringList spans = p.split(QLatin1Char(','));
                if (spans.size() == 2) {
                    if (spans.at(0) != "1")
                        m_writer->writeAttribute("colspan", spans.at(0).trimmed());
                    if (spans.at(1) != "1")
                        m_writer->writeAttribute("rowspan", spans.at(1).trimmed());
                }
            }
        }
        newLine();
        // No skipahead, as opposed to HTML: in DocBook, the text must be wrapped in paragraphs.
        break;
    case Atom::TableItemRight:
        m_writer->writeEndElement(); // th if m_inTableHeader, otherwise td
        newLine();
        break;
    case Atom::TableOfContents:
        Q_FALLTHROUGH();
    case Atom::Keyword:
        break;
    case Atom::Target:
        // Sometimes, there is a \target just before a section title with the same ID. Only outut one xml:id.
        if (matchAhead(atom, Atom::SectionRight) && matchAhead(atom->next(), Atom::SectionLeft)) {
            QString nextId = Utilities::asAsciiPrintable(
                    Text::sectionHeading(atom->next()->next()).toString());
            QString ownId = Utilities::asAsciiPrintable(atom->string());
            if (nextId == ownId)
                break;
        }

        writeAnchor(Utilities::asAsciiPrintable(atom->string()));
        break;
    case Atom::UnhandledFormat:
        m_writer->writeStartElement(dbNamespace, "emphasis");
        m_writer->writeAttribute("role", "bold");
        m_writer->writeCharacters("<Missing DocBook>");
        m_writer->writeEndElement(); // emphasis
        break;
    case Atom::UnknownCommand:
        m_writer->writeStartElement(dbNamespace, "emphasis");
        m_writer->writeAttribute("role", "bold");
        if (m_useITS)
            m_writer->writeAttribute(itsNamespace, "translate", "no");
        m_writer->writeCharacters("<Unknown command>");
        m_writer->writeStartElement(dbNamespace, "code");
        m_writer->writeCharacters(atom->string());
        m_writer->writeEndElement(); // code
        m_writer->writeEndElement(); // emphasis
        break;
    case Atom::CodeQuoteArgument:
    case Atom::CodeQuoteCommand:
    case Atom::SnippetCommand:
    case Atom::SnippetIdentifier:
    case Atom::SnippetLocation:
        // No output (ignore).
        break;
    default:
        unknownAtom(atom);
    }
    return skipAhead;
}

void DocBookGenerator::generateClassHierarchy(const Node *relative, NodeMultiMap &classMap)
{
    // From HtmlGenerator::generateClassHierarchy.
    if (classMap.isEmpty())
        return;

    std::function<void(ClassNode *)> generateClassAndChildren
        = [this, &relative, &generateClassAndChildren](ClassNode * classe) {
            m_writer->writeStartElement(dbNamespace, "listitem");
            newLine();

            // This class.
            m_writer->writeStartElement(dbNamespace, "para");
            generateFullName(classe, relative);
            m_writer->writeEndElement(); // para
            newLine();

            // Children, if any.
            bool hasChild = false;
            for (const RelatedClass &relatedClass : classe->derivedClasses()) {
                if (relatedClass.m_node && relatedClass.m_node->isInAPI()) {
                    hasChild = true;
                    break;
                }
            }

            if (hasChild) {
                m_writer->writeStartElement(dbNamespace, "itemizedlist");
                newLine();

                for (const RelatedClass &relatedClass: classe->derivedClasses()) {
                    if (relatedClass.m_node && relatedClass.m_node->isInAPI()) {
                        generateClassAndChildren(relatedClass.m_node);
                    }
                }

                m_writer->writeEndElement(); // itemizedlist
                newLine();
            }

            // End this class.
            m_writer->writeEndElement(); // listitem
            newLine();
        };

    m_writer->writeStartElement(dbNamespace, "itemizedlist");
    newLine();

    for (const auto &it : classMap) {
        auto *classe = static_cast<ClassNode *>(it);
        if (classe->baseClasses().isEmpty())
            generateClassAndChildren(classe);
    }

    m_writer->writeEndElement(); // itemizedlist
    newLine();
}

void DocBookGenerator::generateLink(const Atom *atom)
{
    Q_ASSERT(m_inLink);

    // From HtmlGenerator::generateLink.
    if (m_linkNode && m_linkNode->isFunction()) {
        auto match = XmlGenerator::m_funcLeftParen.match(atom->string());
        if (match.hasMatch()) {
            // C++: move () outside of link
            qsizetype leftParenLoc = match.capturedStart(1);
            m_writer->writeCharacters(atom->string().left(leftParenLoc));
            endLink();
            m_writer->writeCharacters(atom->string().mid(leftParenLoc));
            return;
        }
    }
    m_writer->writeCharacters(atom->string());
}

/*!
  This version of the function is called when the \a link is known
  to be correct.
 */
void DocBookGenerator::beginLink(const QString &link, const Node *node, const Node *relative)
{
    // From HtmlGenerator::beginLink.
    m_writer->writeStartElement(dbNamespace, "link");
    m_writer->writeAttribute(xlinkNamespace, "href", link);
    if (node && !(relative && node->status() == relative->status())
        && node->isDeprecated())
        m_writer->writeAttribute("role", "deprecated");
    m_inLink = true;
    m_linkNode = node;
}

void DocBookGenerator::endLink()
{
    // From HtmlGenerator::endLink.
    if (m_inLink)
        m_writer->writeEndElement(); // link
    m_inLink = false;
    m_linkNode = nullptr;
}

void DocBookGenerator::generateList(const Node *relative, const QString &selector)
{
    // From HtmlGenerator::generateList, without warnings, changing prototype.
    CNMap cnm;
    Node::NodeType type = Node::NoType;
    if (selector == QLatin1String("overviews"))
        type = Node::Group;
    else if (selector == QLatin1String("cpp-modules"))
        type = Node::Module;
    else if (selector == QLatin1String("qml-modules"))
        type = Node::QmlModule;

    if (type != Node::NoType) {
        NodeList nodeList;
        m_qdb->mergeCollections(type, cnm, relative);
        const QList<CollectionNode *> collectionList = cnm.values();
        nodeList.reserve(collectionList.size());
        for (auto *collectionNode : collectionList)
            nodeList.append(collectionNode);
        generateAnnotatedList(relative, nodeList, selector);
    } else {
        /*
          \generatelist {selector} is only allowed in a comment where
          the topic is \group, \module, or \qmlmodule.
        */
        Node *n = const_cast<Node *>(relative);
        auto *cn = static_cast<CollectionNode *>(n);
        m_qdb->mergeCollections(cn);
        generateAnnotatedList(cn, cn->members(), selector);
    }
}

/*!
  Outputs an annotated list of the nodes in \a nodeList.
  A two-column table is output.
 */
void DocBookGenerator::generateAnnotatedList(const Node *relative, const NodeList &nodeList,
                                             const QString &selector, GeneratedListType type)
{
    if (nodeList.isEmpty())
        return;

    // Do nothing if all items are internal or obsolete.
    if (std::all_of(nodeList.cbegin(), nodeList.cend(), [](const Node *n) {
        return n->isInternal() || n->isDeprecated(); })) {
        return;
    }

    // Detect if there is a need for a variablelist (i.e. titles mapped to
    // descriptions) or a regular itemizedlist (only titles).
    bool noItemsHaveTitle =
            type == ItemizedList || std::all_of(nodeList.begin(), nodeList.end(),
                    [](const Node* node) {
                        return node->doc().briefText().toString().isEmpty();
                    });

    // Wrap the list in a section if needed.
    if (type == AutoSection && m_hasSection)
        startSection("", "Contents");

    // From WebXMLGenerator::generateAnnotatedList.
    if (!nodeList.isEmpty()) {
        m_writer->writeStartElement(dbNamespace, "variablelist");
        m_writer->writeAttribute("role", selector);
        newLine();

        NodeList members{nodeList};
        std::sort(members.begin(), members.end(), Node::nodeNameLessThan);
        for (const auto &node : std::as_const(members)) {
            if (node->isInternal() || node->isDeprecated())
                continue;

            if (noItemsHaveTitle) {
                m_writer->writeStartElement(dbNamespace, "listitem");
                newLine();
                m_writer->writeStartElement(dbNamespace, "para");
            } else {
                m_writer->writeStartElement(dbNamespace, "varlistentry");
                newLine();
                m_writer->writeStartElement(dbNamespace, "term");
            }
            generateFullName(node, relative);
            if (noItemsHaveTitle) {
                m_writer->writeEndElement(); // para
                newLine();
                m_writer->writeEndElement(); // listitem
            } else {
                m_writer->writeEndElement(); // term
                newLine();
                m_writer->writeStartElement(dbNamespace, "listitem");
                newLine();
                m_writer->writeStartElement(dbNamespace, "para");
                m_writer->writeCharacters(node->doc().briefText().toString());
                m_writer->writeEndElement(); // para
                newLine();
                m_writer->writeEndElement(); // listitem
                newLine();
                m_writer->writeEndElement(); // varlistentry
            }
            newLine();
        }

        m_writer->writeEndElement(); // itemizedlist or variablelist
        newLine();
    }

    if (type == AutoSection && m_hasSection)
        endSection();
}

/*!
  Outputs a series of annotated lists from the nodes in \a nmm,
  divided into sections based by the key names in the multimap.
 */
void DocBookGenerator::generateAnnotatedLists(const Node *relative, const NodeMultiMap &nmm,
                                              const QString &selector)
{
    // From HtmlGenerator::generateAnnotatedLists.
    for (const QString &name : nmm.uniqueKeys()) {
        if (!name.isEmpty())
            startSection(name.toLower(), name);
        generateAnnotatedList(relative, nmm.values(name), selector);
        if (!name.isEmpty())
            endSection();
    }
}

/*!
  This function finds the common prefix of the names of all
  the classes in the class map \a nmm and then generates a
  compact list of the class names alphabetized on the part
  of the name not including the common prefix. You can tell
  the function to use \a comonPrefix as the common prefix,
  but normally you let it figure it out itself by looking at
  the name of the first and last classes in the class map
  \a nmm.
 */
void DocBookGenerator::generateCompactList(const Node *relative, const NodeMultiMap &nmm,
                                           bool includeAlphabet, const QString &commonPrefix,
                                           const QString &selector)
{
    // From HtmlGenerator::generateCompactList. No more "includeAlphabet", this should be handled by
    // the DocBook toolchain afterwards.
    // TODO: In DocBook, probably no need for this method: this is purely presentational, i.e. to be
    // fully handled by the DocBook toolchain.

    if (nmm.isEmpty())
        return;

    const int NumParagraphs = 37; // '0' to '9', 'A' to 'Z', '_'
    qsizetype commonPrefixLen = commonPrefix.size();

    /*
      Divide the data into 37 paragraphs: 0, ..., 9, A, ..., Z,
      underscore (_). QAccel will fall in paragraph 10 (A) and
      QXtWidget in paragraph 33 (X). This is the only place where we
      assume that NumParagraphs is 37. Each paragraph is a NodeMultiMap.
    */
    NodeMultiMap paragraph[NumParagraphs + 1];
    QString paragraphName[NumParagraphs + 1];
    QSet<char> usedParagraphNames;

    for (auto c = nmm.constBegin(); c != nmm.constEnd(); ++c) {
        QStringList pieces = c.key().split("::");
        int idx = commonPrefixLen;
        if (idx > 0 && !pieces.last().startsWith(commonPrefix, Qt::CaseInsensitive))
            idx = 0;
        QString last = pieces.last().toLower();
        QString key = last.mid(idx);

        int paragraphNr = NumParagraphs - 1;

        if (key[0].digitValue() != -1) {
            paragraphNr = key[0].digitValue();
        } else if (key[0] >= QLatin1Char('a') && key[0] <= QLatin1Char('z')) {
            paragraphNr = 10 + key[0].unicode() - 'a';
        }

        paragraphName[paragraphNr] = key[0].toUpper();
        usedParagraphNames.insert(key[0].toLower().cell());
        paragraph[paragraphNr].insert(last, c.value());
    }

    /*
      Each paragraph j has a size: paragraph[j].count(). In the
      discussion, we will assume paragraphs 0 to 5 will have sizes
      3, 1, 4, 1, 5, 9.

      We now want to compute the paragraph offset. Paragraphs 0 to 6
      start at offsets 0, 3, 4, 8, 9, 14, 23.
    */
    int paragraphOffset[NumParagraphs + 1]; // 37 + 1
    paragraphOffset[0] = 0;
    for (int i = 0; i < NumParagraphs; i++) // i = 0..36
        paragraphOffset[i + 1] = paragraphOffset[i] + paragraph[i].size();

    // Output the alphabet as a row of links.
    if (includeAlphabet && !usedParagraphNames.isEmpty()) {
        m_writer->writeStartElement(dbNamespace, "simplelist");
        newLine();

        for (int i = 0; i < 26; i++) {
            QChar ch('a' + i);
            if (usedParagraphNames.contains(char('a' + i))) {
                m_writer->writeStartElement(dbNamespace, "member");
                generateSimpleLink(ch, ch.toUpper());
                m_writer->writeEndElement(); // member
                newLine();
            }
        }

        m_writer->writeEndElement(); // simplelist
        newLine();
    }

    // Actual output.
    int curParNr = 0;
    int curParOffset = 0;
    QString previousName;
    bool multipleOccurrences = false;

    m_writer->writeStartElement(dbNamespace, "variablelist");
    m_writer->writeAttribute("role", selector);
    newLine();

    for (int i = 0; i < nmm.size(); i++) {
        while ((curParNr < NumParagraphs) && (curParOffset == paragraph[curParNr].size())) {

            ++curParNr;
            curParOffset = 0;
        }

        // Starting a new paragraph means starting a new varlistentry.
        if (curParOffset == 0) {
            if (i > 0) {
                m_writer->writeEndElement(); // itemizedlist
                newLine();
                m_writer->writeEndElement(); // listitem
                newLine();
                m_writer->writeEndElement(); // varlistentry
                newLine();
            }

            m_writer->writeStartElement(dbNamespace, "varlistentry");
            if (includeAlphabet)
                writeXmlId(paragraphName[curParNr][0].toLower());
            newLine();

            m_writer->writeStartElement(dbNamespace, "term");
            m_writer->writeStartElement(dbNamespace, "emphasis");
            m_writer->writeAttribute("role", "bold");
            m_writer->writeCharacters(paragraphName[curParNr]);
            m_writer->writeEndElement(); // emphasis
            m_writer->writeEndElement(); // term
            newLine();

            m_writer->writeStartElement(dbNamespace, "listitem");
            newLine();
            m_writer->writeStartElement(dbNamespace, "itemizedlist");
            newLine();
        }

        // Output a listitem for the current offset in the current paragraph.
        m_writer->writeStartElement(dbNamespace, "listitem");
        newLine();
        m_writer->writeStartElement(dbNamespace, "para");

        if ((curParNr < NumParagraphs) && !paragraphName[curParNr].isEmpty()) {
            NodeMultiMap::Iterator it;
            NodeMultiMap::Iterator next;
            it = paragraph[curParNr].begin();
            for (int j = 0; j < curParOffset; j++)
                ++it;

            // Cut the name into pieces to determine whether it is simple (one piece) or complex
            // (more than one piece).
            QStringList pieces;
            if (it.value()->isQmlType()) {
                QString name = it.value()->name();
                next = it;
                ++next;
                if (name != previousName)
                    multipleOccurrences = false;
                if ((next != paragraph[curParNr].end()) && (name == next.value()->name())) {
                    multipleOccurrences = true;
                    previousName = name;
                }
                if (multipleOccurrences)
                    name += ": " + it.value()->tree()->camelCaseModuleName();
                pieces << name;
            } else
                pieces = it.value()->fullName(relative).split("::");

            // Write the link to the element, which is identical if the element is obsolete or not.
            m_writer->writeStartElement(dbNamespace, "link");
            m_writer->writeAttribute(xlinkNamespace, "href", linkForNode(*it, relative));
            if (const QString type = targetType(it.value()); !type.isEmpty())
                m_writer->writeAttribute("role", type);
            m_writer->writeCharacters(pieces.last());
            m_writer->writeEndElement(); // link

            // Outside the link, give the full name of the node if it is complex.
            if (pieces.size() > 1) {
                m_writer->writeCharacters(" (");
                generateFullName(it.value()->parent(), relative);
                m_writer->writeCharacters(")");
            }
        }

        m_writer->writeEndElement(); // para
        newLine();
        m_writer->writeEndElement(); // listitem
        newLine();

        curParOffset++;
    }
    m_writer->writeEndElement(); // itemizedlist
    newLine();
    m_writer->writeEndElement(); // listitem
    newLine();
    m_writer->writeEndElement(); // varlistentry
    newLine();

    m_writer->writeEndElement(); // variablelist
    newLine();
}

void DocBookGenerator::generateFunctionIndex(const Node *relative)
{
    // From HtmlGenerator::generateFunctionIndex.

    // First list: links to parts of the second list, one item per letter.
    m_writer->writeStartElement(dbNamespace, "simplelist");
    m_writer->writeAttribute("role", "functionIndex");
    newLine();
    for (int i = 0; i < 26; i++) {
        QChar ch('a' + i);
        m_writer->writeStartElement(dbNamespace, "member");
        m_writer->writeAttribute(xlinkNamespace, "href", QString("#") + ch);
        m_writer->writeCharacters(ch.toUpper());
        m_writer->writeEndElement(); // member
        newLine();
    }
    m_writer->writeEndElement(); // simplelist
    newLine();

    // Second list: the actual list of functions, sorted by alphabetical
    // order. One entry of the list per letter.
    if (m_qdb->getFunctionIndex().isEmpty())
        return;
    char nextLetter = 'a';
    char currentLetter;

    m_writer->writeStartElement(dbNamespace, "itemizedlist");
    newLine();

    NodeMapMap &funcIndex = m_qdb->getFunctionIndex();
    QMap<QString, NodeMap>::ConstIterator f = funcIndex.constBegin();
    while (f != funcIndex.constEnd()) {
        m_writer->writeStartElement(dbNamespace, "listitem");
        newLine();
        m_writer->writeStartElement(dbNamespace, "para");
        m_writer->writeCharacters(f.key() + ": ");

        currentLetter = f.key()[0].unicode();
        while (islower(currentLetter) && currentLetter >= nextLetter) {
            writeAnchor(QString(nextLetter));
            nextLetter++;
        }

        NodeMap::ConstIterator s = (*f).constBegin();
        while (s != (*f).constEnd()) {
            m_writer->writeCharacters(" ");
            generateFullName((*s)->parent(), relative);
            ++s;
        }

        m_writer->writeEndElement(); // para
        newLine();
        m_writer->writeEndElement(); // listitem
        newLine();
        ++f;
    }
    m_writer->writeEndElement(); // itemizedlist
    newLine();
}

void DocBookGenerator::generateLegaleseList(const Node *relative)
{
    // From HtmlGenerator::generateLegaleseList.
    TextToNodeMap &legaleseTexts = m_qdb->getLegaleseTexts();
    for (auto it = legaleseTexts.cbegin(), end = legaleseTexts.cend(); it != end; ++it) {
        Text text = it.key();
        generateText(text, relative);
        m_writer->writeStartElement(dbNamespace, "itemizedlist");
        newLine();
        do {
            m_writer->writeStartElement(dbNamespace, "listitem");
            newLine();
            m_writer->writeStartElement(dbNamespace, "para");
            generateFullName(it.value(), relative);
            m_writer->writeEndElement(); // para
            newLine();
            m_writer->writeEndElement(); // listitem
            newLine();
            ++it;
        } while (it != legaleseTexts.constEnd() && it.key() == text);
        m_writer->writeEndElement(); // itemizedlist
        newLine();
    }
}

void DocBookGenerator::generateBrief(const Node *node)
{
    // From HtmlGenerator::generateBrief. Also see generateHeader, which is specifically dealing
    // with the DocBook header (and thus wraps the brief in an abstract).
    Text brief = node->doc().briefText();

    if (!brief.isEmpty()) {
        if (!brief.lastAtom()->string().endsWith('.'))
            brief << Atom(Atom::String, ".");

        m_writer->writeStartElement(dbNamespace, "para");
        generateText(brief, node);
        m_writer->writeEndElement(); // para
        newLine();
    }
}

bool DocBookGenerator::generateSince(const Node *node)
{
    // From Generator::generateSince.
    if (!node->since().isEmpty()) {
        m_writer->writeStartElement(dbNamespace, "para");
        m_writer->writeCharacters("This " + typeString(node) + " was introduced in ");
        m_writer->writeCharacters(formatSince(node) + ".");
        m_writer->writeEndElement(); // para
        newLine();

        return true;
    }

    return false;
}

/*!
    Generate the DocBook header for the file, including the abstract.
    Equivalent to calling generateTitle and generateBrief in HTML.
*/
void DocBookGenerator::generateHeader(const QString &title, const QString &subTitle,
                                      const Node *node)
{
    refMap.clear();

    // Output the DocBook header.
    m_writer->writeStartElement(dbNamespace, "info");
    newLine();
    m_writer->writeStartElement(dbNamespace, "title");
    if (node->genus() & Node::API && m_useITS)
        m_writer->writeAttribute(itsNamespace, "translate", "no");
    m_writer->writeCharacters(title);
    m_writer->writeEndElement(); // title
    newLine();

    if (!subTitle.isEmpty()) {
        m_writer->writeStartElement(dbNamespace, "subtitle");
        if (node->genus() & Node::API && m_useITS)
            m_writer->writeAttribute(itsNamespace, "translate", "no");
        m_writer->writeCharacters(subTitle);
        m_writer->writeEndElement(); // subtitle
        newLine();
    }

    if (!m_project.isEmpty()) {
        m_writer->writeTextElement(dbNamespace, "productname", m_project);
        newLine();
    }

    if (!m_buildVersion.isEmpty()) {
        m_writer->writeTextElement(dbNamespace, "edition", m_buildVersion);
        newLine();
    }

    if (!m_projectDescription.isEmpty()) {
        m_writer->writeTextElement(dbNamespace, "titleabbrev", m_projectDescription);
        newLine();
    }

    // Deal with links.
    // Adapted from HtmlGenerator::generateHeader (output part: no need to update a navigationLinks
    // or useSeparator field, as this content is only output in the info tag, not in the main
    // content).
    if (node && !node->links().empty()) {
        std::pair<QString, QString> linkPair;
        std::pair<QString, QString> anchorPair;
        const Node *linkNode;

        if (node->links().contains(Node::PreviousLink)) {
            linkPair = node->links()[Node::PreviousLink];
            linkNode = m_qdb->findNodeForTarget(linkPair.first, node);
            if (!linkNode || linkNode == node)
                anchorPair = linkPair;
            else
                anchorPair = anchorForNode(linkNode);

            m_writer->writeStartElement(dbNamespace, "extendedlink");
            m_writer->writeAttribute(xlinkNamespace, "type", "extended");
            m_writer->writeEmptyElement(dbNamespace, "link");
            m_writer->writeAttribute(xlinkNamespace, "to", anchorPair.first);
            m_writer->writeAttribute(xlinkNamespace, "type", "arc");
            m_writer->writeAttribute(xlinkNamespace, "arcrole", "prev");
            if (linkPair.first == linkPair.second && !anchorPair.second.isEmpty())
                m_writer->writeAttribute(xlinkNamespace, "title", anchorPair.second);
            else
                m_writer->writeAttribute(xlinkNamespace, "title", linkPair.second);
            m_writer->writeEndElement(); // extendedlink
            newLine();
        }
        if (node->links().contains(Node::NextLink)) {
            linkPair = node->links()[Node::NextLink];
            linkNode = m_qdb->findNodeForTarget(linkPair.first, node);
            if (!linkNode || linkNode == node)
                anchorPair = linkPair;
            else
                anchorPair = anchorForNode(linkNode);

            m_writer->writeStartElement(dbNamespace, "extendedlink");
            m_writer->writeAttribute(xlinkNamespace, "type", "extended");
            m_writer->writeEmptyElement(dbNamespace, "link");
            m_writer->writeAttribute(xlinkNamespace, "to", anchorPair.first);
            m_writer->writeAttribute(xlinkNamespace, "type", "arc");
            m_writer->writeAttribute(xlinkNamespace, "arcrole", "next");
            if (linkPair.first == linkPair.second && !anchorPair.second.isEmpty())
                m_writer->writeAttribute(xlinkNamespace, "title", anchorPair.second);
            else
                m_writer->writeAttribute(xlinkNamespace, "title", linkPair.second);
            m_writer->writeEndElement(); // extendedlink
            newLine();
        }
        if (node->links().contains(Node::StartLink)) {
            linkPair = node->links()[Node::StartLink];
            linkNode = m_qdb->findNodeForTarget(linkPair.first, node);
            if (!linkNode || linkNode == node)
                anchorPair = linkPair;
            else
                anchorPair = anchorForNode(linkNode);

            m_writer->writeStartElement(dbNamespace, "extendedlink");
            m_writer->writeAttribute(xlinkNamespace, "type", "extended");
            m_writer->writeEmptyElement(dbNamespace, "link");
            m_writer->writeAttribute(xlinkNamespace, "to", anchorPair.first);
            m_writer->writeAttribute(xlinkNamespace, "type", "arc");
            m_writer->writeAttribute(xlinkNamespace, "arcrole", "start");
            if (linkPair.first == linkPair.second && !anchorPair.second.isEmpty())
                m_writer->writeAttribute(xlinkNamespace, "title", anchorPair.second);
            else
                m_writer->writeAttribute(xlinkNamespace, "title", linkPair.second);
            m_writer->writeEndElement(); // extendedlink
            newLine();
        }
    }

    // Deal with the abstract (what qdoc calls brief).
    if (node) {
        // Adapted from HtmlGenerator::generateBrief, without extraction marks. The parameter
        // addLink is always false. Factoring this function out is not as easy as in HtmlGenerator:
        // abstracts only happen in the header (info tag), slightly different tags must be used at
        // other places. Also includes code from HtmlGenerator::generateCppReferencePage to handle
        // the name spaces.
        m_writer->writeStartElement(dbNamespace, "abstract");
        newLine();

        bool generatedSomething = false;

        Text brief;
        const NamespaceNode *ns =
                node->isNamespace() ? static_cast<const NamespaceNode *>(node) : nullptr;
        if (ns && !ns->hasDoc() && ns->docNode()) {
            NamespaceNode *NS = ns->docNode();
            brief << "The " << ns->name()
                  << " namespace includes the following elements from module "
                  << ns->tree()->camelCaseModuleName() << ". The full namespace is "
                  << "documented in module " << NS->tree()->camelCaseModuleName()
                  << Atom(Atom::LinkNode, fullDocumentLocation(NS))
                  << Atom(Atom::FormattingLeft, ATOM_FORMATTING_LINK)
                  << Atom(Atom::String, " here.")
                  << Atom(Atom::FormattingRight, ATOM_FORMATTING_LINK);
        } else {
            brief = node->doc().briefText();
        }

        if (!brief.isEmpty()) {
            if (!brief.lastAtom()->string().endsWith('.'))
                brief << Atom(Atom::String, ".");

            m_writer->writeStartElement(dbNamespace, "para");
            generateText(brief, node);
            m_writer->writeEndElement(); // para
            newLine();

            generatedSomething = true;
        }

        // Generate other paragraphs that should go into the abstract.
        generatedSomething |= generateStatus(node);
        generatedSomething |= generateSince(node);
        generatedSomething |= generateThreadSafeness(node);

        // An abstract cannot be empty, hence use the project description.
        if (!generatedSomething)
            m_writer->writeTextElement(dbNamespace, "para", m_projectDescription + ".");

        m_writer->writeEndElement(); // abstract
        newLine();
    }

    // End of the DocBook header.
    m_writer->writeEndElement(); // info
    newLine();
}

void DocBookGenerator::closeTextSections()
{
    while (!sectionLevels.isEmpty()) {
        sectionLevels.pop();
        endSection();
    }
}

void DocBookGenerator::generateFooter()
{
    if (m_closeSectionAfterGeneratedList) {
        m_closeSectionAfterGeneratedList = false;
        endSection();
    }
    if (m_closeSectionAfterRawTitle) {
        m_closeSectionAfterRawTitle = false;
        endSection();
    }

    closeTextSections();
    m_writer->writeEndElement(); // article
}

void DocBookGenerator::generateSimpleLink(const QString &href, const QString &text)
{
    m_writer->writeStartElement(dbNamespace, "link");
    m_writer->writeAttribute(xlinkNamespace, "href", href);
    m_writer->writeCharacters(text);
    m_writer->writeEndElement(); // link
}

void DocBookGenerator::generateObsoleteMembers(const Sections &sections)
{
    // From HtmlGenerator::generateObsoleteMembersFile.
    SectionPtrVector summary_spv; // Summaries are ignored in DocBook (table of contents).
    SectionPtrVector details_spv;
    if (!sections.hasObsoleteMembers(&summary_spv, &details_spv))
        return;

    Aggregate *aggregate = sections.aggregate();
    startSection("obsolete", "Obsolete Members for " + aggregate->name());

    m_writer->writeStartElement(dbNamespace, "para");
    m_writer->writeStartElement(dbNamespace, "emphasis");
    m_writer->writeAttribute("role", "bold");
    m_writer->writeCharacters("The following members of class ");
    generateSimpleLink(linkForNode(aggregate, nullptr), aggregate->name());
    m_writer->writeCharacters(" are deprecated.");
    m_writer->writeEndElement(); // emphasis bold
    m_writer->writeCharacters(" We strongly advise against using them in new code.");
    m_writer->writeEndElement(); // para
    newLine();

    for (const Section *section : details_spv) {
        const QString &title = "Obsolete " + section->title();
        startSection(title.toLower(), title);

        const NodeVector &members = section->obsoleteMembers();
        NodeVector::ConstIterator m = members.constBegin();
        while (m != members.constEnd()) {
            if ((*m)->access() != Access::Private)
                generateDetailedMember(*m, aggregate);
            ++m;
        }

        endSection();
    }

    endSection();
}

/*!
  Generates a separate section where obsolete members of the QML
  type \a qcn are listed. The \a marker is used to generate
  the section lists, which are then traversed and output here.

  Note that this function currently only handles correctly the
  case where \a status is \c {Section::Deprecated}.
 */
void DocBookGenerator::generateObsoleteQmlMembers(const Sections &sections)
{
    // From HtmlGenerator::generateObsoleteQmlMembersFile.
    SectionPtrVector summary_spv; // Summaries are not useful in DocBook.
    SectionPtrVector details_spv;
    if (!sections.hasObsoleteMembers(&summary_spv, &details_spv))
        return;

    Aggregate *aggregate = sections.aggregate();
    startSection("obsolete", "Obsolete Members for " + aggregate->name());

    m_writer->writeStartElement(dbNamespace, "para");
    m_writer->writeStartElement(dbNamespace, "emphasis");
    m_writer->writeAttribute("role", "bold");
    m_writer->writeCharacters("The following members of QML type ");
    generateSimpleLink(linkForNode(aggregate, nullptr), aggregate->name());
    m_writer->writeCharacters(" are deprecated.");
    m_writer->writeEndElement(); // emphasis bold
    m_writer->writeCharacters(" We strongly advise against using them in new code.");
    m_writer->writeEndElement(); // para
    newLine();

    for (const auto *section : details_spv) {
        const QString &title = "Obsolete " + section->title();
        startSection(title.toLower(), title);

        const NodeVector &members = section->obsoleteMembers();
        NodeVector::ConstIterator m = members.constBegin();
        while (m != members.constEnd()) {
            if ((*m)->access() != Access::Private)
                generateDetailedQmlMember(*m, aggregate);
            ++m;
        }

        endSection();
    }

    endSection();
}

static QString nodeToSynopsisTag(const Node *node)
{
    // Order from Node::nodeTypeString.
    if (node->isClass() || node->isQmlType())
        return QStringLiteral("classsynopsis");
    if (node->isNamespace())
        return QStringLiteral("packagesynopsis");
    if (node->isPageNode()) {
        node->doc().location().warning("Unexpected document node in nodeToSynopsisTag");
        return QString();
    }
    if (node->isEnumType())
        return QStringLiteral("enumsynopsis");
    if (node->isTypedef())
        return QStringLiteral("typedefsynopsis");
    if (node->isFunction()) {
        // Signals are also encoded as functions (including QML ones).
        const auto fn = static_cast<const FunctionNode *>(node);
        if (fn->isCtor() || fn->isCCtor() || fn->isMCtor())
            return QStringLiteral("constructorsynopsis");
        if (fn->isDtor())
            return QStringLiteral("destructorsynopsis");
        return QStringLiteral("methodsynopsis");
    }
    if (node->isProperty() || node->isVariable() || node->isQmlProperty())
        return QStringLiteral("fieldsynopsis");

    node->doc().location().warning(QString("Unknown node tag %1").arg(node->nodeTypeString()));
    return QStringLiteral("synopsis");
}

void DocBookGenerator::generateStartRequisite(const QString &description)
{
    m_writer->writeStartElement(dbNamespace, "varlistentry");
    newLine();
    m_writer->writeTextElement(dbNamespace, "term", description);
    newLine();
    m_writer->writeStartElement(dbNamespace, "listitem");
    newLine();
    m_writer->writeStartElement(dbNamespace, "para");
    m_inPara = true;
}

void DocBookGenerator::generateEndRequisite()
{
    m_writer->writeEndElement(); // para
    m_inPara = false;
    newLine();
    m_writer->writeEndElement(); // listitem
    newLine();
    m_writer->writeEndElement(); // varlistentry
    newLine();
}

void DocBookGenerator::generateRequisite(const QString &description, const QString &value)
{
    generateStartRequisite(description);
    m_writer->writeCharacters(value);
    generateEndRequisite();
}

/*!
 * \internal
 * Generates the CMake (\a description) requisites
 */
void DocBookGenerator::generateCMakeRequisite(const QStringList &values)
{
    const QString description("CMake");
    generateStartRequisite(description);
    m_writer->writeCharacters(values.first());
    m_writer->writeEndElement(); // para
    newLine();

    m_writer->writeStartElement(dbNamespace, "para");
    m_writer->writeCharacters(values.last());
    generateEndRequisite();
}

void DocBookGenerator::generateSortedNames(const ClassNode *cn, const QList<RelatedClass> &rc)
{
    // From Generator::appendSortedNames.
    QMap<QString, ClassNode *> classMap;
    QList<RelatedClass>::ConstIterator r = rc.constBegin();
    while (r != rc.constEnd()) {
        ClassNode *rcn = (*r).m_node;
        if (rcn && rcn->access() == Access::Public && rcn->status() != Node::Internal
            && !rcn->doc().isEmpty()) {
            classMap[rcn->plainFullName(cn).toLower()] = rcn;
        }
        ++r;
    }

    QStringList classNames = classMap.keys();
    classNames.sort();

    int index = 0;
    for (const QString &className : classNames) {
        generateFullName(classMap.value(className), cn);
        m_writer->writeCharacters(Utilities::comma(index++, classNames.size()));
    }
}

void DocBookGenerator::generateSortedQmlNames(const Node *base, const NodeList &subs)
{
    // From Generator::appendSortedQmlNames.
    QMap<QString, Node *> classMap;
    int index = 0;

    for (auto sub : subs)
        if (!base->isQtQuickNode() || !sub->isQtQuickNode()
            || (base->logicalModuleName() == sub->logicalModuleName()))
            classMap[sub->plainFullName(base).toLower()] = sub;

    QStringList names = classMap.keys();
    names.sort();

    for (const QString &name : names) {
        generateFullName(classMap.value(name), base);
        m_writer->writeCharacters(Utilities::comma(index++, names.size()));
    }
}

/*!
  Lists the required imports and includes.
*/
void DocBookGenerator::generateRequisites(const Aggregate *aggregate)
{
    // Adapted from HtmlGenerator::generateRequisites, but simplified: no need to store all the
    // elements, they can be produced one by one.

    // Generate the requisites first separately: if some of them are generated, output them in a wrapper.
    // This complexity is required to ensure the DocBook file is valid: an empty list is not valid. It is not easy
    // to write a truly comprehensive condition.
    QXmlStreamWriter* oldWriter = m_writer;
    QString output;
    m_writer = new QXmlStreamWriter(&output);

    // Includes.
    if (aggregate->includeFile()) generateRequisite("Header", *aggregate->includeFile());

    // Since and project.
    if (!aggregate->since().isEmpty())
        generateRequisite("Since", formatSince(aggregate));

    if (aggregate->isClassNode() || aggregate->isNamespace()) {
        // CMake and QT variable.
        const CollectionNode *cn =
                m_qdb->getCollectionNode(aggregate->physicalModuleName(), Node::Module);
        if (cn && !cn->qtCMakeComponent().isEmpty()) {
            const QString qtComponent = "Qt" + QString::number(QT_VERSION_MAJOR);
            const QString findpackageText = "find_package(" + qtComponent
                    + " REQUIRED COMPONENTS " + cn->qtCMakeComponent() + ")";
            const QString targetLinkLibrariesText = "target_link_libraries(mytarget PRIVATE "
                    + qtComponent + "::" + cn->qtCMakeComponent() + ")";
            const QStringList cmakeInfo { findpackageText, targetLinkLibrariesText };
            generateCMakeRequisite(cmakeInfo);
        }
        if (cn && !cn->qtVariable().isEmpty())
            generateRequisite("qmake", "QT += " + cn->qtVariable());
    }

    if (aggregate->nodeType() == Node::Class) {
        // Instantiated by.
        auto *classe = const_cast<ClassNode *>(static_cast<const ClassNode *>(aggregate));
        if (classe->qmlElement() != nullptr && classe->status() != Node::Internal) {
            generateStartRequisite("Inherited By");
            generateSortedNames(classe, classe->derivedClasses());
            generateEndRequisite();
            generateRequisite("Instantiated By", fullDocumentLocation(classe->qmlElement()));
        }

        // Inherits.
        QList<RelatedClass>::ConstIterator r;
        if (!classe->baseClasses().isEmpty()) {
            generateStartRequisite("Inherits");

            r = classe->baseClasses().constBegin();
            int index = 0;
            while (r != classe->baseClasses().constEnd()) {
                if ((*r).m_node) {
                    generateFullName((*r).m_node, classe);

                    if ((*r).m_access == Access::Protected)
                        m_writer->writeCharacters(" (protected)");
                    else if ((*r).m_access == Access::Private)
                        m_writer->writeCharacters(" (private)");
                    m_writer->writeCharacters(
                            Utilities::comma(index++, classe->baseClasses().size()));
                }
                ++r;
            }

            generateEndRequisite();
        }

        // Inherited by.
        if (!classe->derivedClasses().isEmpty()) {
            generateStartRequisite("Inherited By");
            generateSortedNames(classe, classe->derivedClasses());
            generateEndRequisite();
        }
    }

    // Group.
    if (!aggregate->groupNames().empty()) {
        generateStartRequisite("Group");
        generateGroupReferenceText(aggregate);
        generateEndRequisite();
    }

    // Status.
    if (auto status = formatStatus(aggregate, m_qdb); status)
        generateRequisite("Status", status.value());

    // Write the elements as a list if not empty.
    delete m_writer;
    m_writer = oldWriter;

    if (!output.isEmpty()) {
        // Namespaces are mangled in this output, because QXmlStreamWriter doesn't know about them. (Letting it know
        // would imply generating the xmlns declaration one more time.)
        static const QRegularExpression xmlTag(R"(<(/?)n\d+:)"); // Only for DocBook tags.
        static const QRegularExpression xmlnsDocBookDefinition(R"( xmlns:n\d+=")" + QString{dbNamespace} + "\"");
        static const QRegularExpression xmlnsXLinkDefinition(R"( xmlns:n\d+=")" + QString{xlinkNamespace} + "\"");
        static const QRegularExpression xmlAttr(R"( n\d+:)"); // Only for XLink attributes.
        // Space at the beginning!
        const QString cleanOutput = output.replace(xmlTag, R"(<\1db:)")
            .replace(xmlnsDocBookDefinition, "")
            .replace(xmlnsXLinkDefinition, "")
            .replace(xmlAttr, " xlink:");

        m_writer->writeStartElement(dbNamespace, "variablelist");
        if (m_useITS)
            m_writer->writeAttribute(itsNamespace, "translate", "no");
        newLine();

        m_writer->device()->write(cleanOutput.toUtf8());

        m_writer->writeEndElement(); // variablelist
        newLine();
    }
}

/*!
  Lists the required imports and includes.
*/
void DocBookGenerator::generateQmlRequisites(const QmlTypeNode *qcn)
{
    // From HtmlGenerator::generateQmlRequisites, but simplified: no need to store all the elements,
    // they can be produced one by one.
    if (!qcn)
        return;

    const CollectionNode *collection = qcn->logicalModule();

    NodeList subs;
    QmlTypeNode::subclasses(qcn, subs);

    QmlTypeNode *base = qcn->qmlBaseNode();
    while (base && base->isInternal()) {
        base = base->qmlBaseNode();
    }

    // Skip import statement for \internal collections
    const bool generate_import_statement = !qcn->logicalModuleName().isEmpty() && (!collection || !collection->isInternal() || m_showInternal);
    // Detect if anything is generated in this method. If not, exit early to avoid having an empty list.
    const bool generates_something = generate_import_statement || !qcn->since().isEmpty() || !subs.isEmpty() || base;

    if (!generates_something)
        return;

    // Start writing the elements as a list.
    m_writer->writeStartElement(dbNamespace, "variablelist");
    if (m_useITS)
        m_writer->writeAttribute(itsNamespace, "translate", "no");
    newLine();

    if (generate_import_statement) {
        QStringList parts = QStringList() << "import" << qcn->logicalModuleName() << qcn->logicalModuleVersion();
        generateRequisite("Import Statement", parts.join(' ').trimmed());
    }

    // Since and project.
    if (!qcn->since().isEmpty())
        generateRequisite("Since:", formatSince(qcn));

    // Inherited by.
    if (!subs.isEmpty()) {
        generateStartRequisite("Inherited By:");
        generateSortedQmlNames(qcn, subs);
        generateEndRequisite();
    }

    // Inherits.
    if (base) {
        const Node *otherNode = nullptr;
        Atom a = Atom(Atom::LinkNode, CodeMarker::stringForNode(base));
        QString link = getAutoLink(&a, qcn, &otherNode);

        generateStartRequisite("Inherits:");
        generateSimpleLink(link, base->name());
        generateEndRequisite();
    }

    // Instantiates.
    ClassNode *cn = (const_cast<QmlTypeNode *>(qcn))->classNode();
    if (cn && (cn->status() != Node::Internal)) {
        Atom a = Atom(Atom::LinkNode, CodeMarker::stringForNode(qcn));

        generateStartRequisite("Instantiates:");
        generateSimpleLink(fullDocumentLocation(cn), cn->name());
        generateEndRequisite();
    }

    // Group.
    if (!qcn->groupNames().empty()) {
        generateStartRequisite("Group");
        generateGroupReferenceText(qcn);
        generateEndRequisite();
    }

    // Status.
    if (auto status = formatStatus(qcn, m_qdb); status)
        generateRequisite("Status:", status.value());

    m_writer->writeEndElement(); // variablelist
    newLine();
}

bool DocBookGenerator::generateStatus(const Node *node)
{
    // From Generator::generateStatus.
    switch (node->status()) {
    case Node::Active:
        // Output the module 'state' description if set.
        if (node->isModule() || node->isQmlModule()) {
            const QString &state = static_cast<const CollectionNode*>(node)->state();
            if (!state.isEmpty()) {
                m_writer->writeStartElement(dbNamespace, "para");
                m_writer->writeCharacters("This " + typeString(node) + " is in ");
                m_writer->writeStartElement(dbNamespace, "emphasis");
                m_writer->writeCharacters(state);
                m_writer->writeEndElement(); // emphasis
                m_writer->writeCharacters(" state.");
                m_writer->writeEndElement(); // para
                newLine();
                return true;
            }
        }
        return false;
    case Node::Preliminary:
        m_writer->writeStartElement(dbNamespace, "para");
        m_writer->writeStartElement(dbNamespace, "emphasis");
        m_writer->writeAttribute("role", "bold");
        m_writer->writeCharacters("This " + typeString(node)
                                  + " is under development and is subject to change.");
        m_writer->writeEndElement(); // emphasis
        m_writer->writeEndElement(); // para
        newLine();
        return true;
    case Node::Deprecated:
        m_writer->writeStartElement(dbNamespace, "para");
        if (node->isAggregate()) {
            m_writer->writeStartElement(dbNamespace, "emphasis");
            m_writer->writeAttribute("role", "bold");
        }
        m_writer->writeCharacters("This " + typeString(node) + " is deprecated");
        if (const QString &version = node->deprecatedSince(); !version.isEmpty())
            m_writer->writeCharacters(" since " + version);
        m_writer->writeCharacters(". We strongly advise against using it in new code.");
        if (node->isAggregate())
            m_writer->writeEndElement(); // emphasis
        m_writer->writeEndElement(); // para
        newLine();
        return true;
    case Node::Internal:
    default:
        return false;
    }
}

/*!
  Generate a list of function signatures. The function nodes
  are in \a nodes.
 */
void DocBookGenerator::generateSignatureList(const NodeList &nodes)
{
    // From Generator::signatureList and Generator::appendSignature.
    m_writer->writeStartElement(dbNamespace, "itemizedlist");
    newLine();

    NodeList::ConstIterator n = nodes.constBegin();
    while (n != nodes.constEnd()) {
        m_writer->writeStartElement(dbNamespace, "listitem");
        newLine();
        m_writer->writeStartElement(dbNamespace, "para");

        generateSimpleLink(currentGenerator()->fullDocumentLocation(*n),
                           (*n)->signature(Node::SignaturePlain));

        m_writer->writeEndElement(); // para
        newLine();
        m_writer->writeEndElement(); // itemizedlist
        newLine();
        ++n;
    }

    m_writer->writeEndElement(); // itemizedlist
    newLine();
}

/*!
 * Return a string representing a text that exposes information about
 * the groups that the \a node is part of.
 */
void DocBookGenerator::generateGroupReferenceText(const Node* node)
{
    // From HtmlGenerator::groupReferenceText

    if (!node->isAggregate())
        return;
    const auto aggregate = static_cast<const Aggregate *>(node);

    const QStringList &groups_names{aggregate->groupNames()};
    if (!groups_names.empty()) {
        m_writer->writeStartElement(dbNamespace, "para");
        m_writer->writeCharacters(aggregate->name() + " is part of ");
        m_writer->writeStartElement(dbNamespace, "simplelist");

        for (qsizetype index{0}; index < groups_names.size(); ++index) {
            CollectionNode* group{m_qdb->groups()[groups_names[index]]};
            m_qdb->mergeCollections(group);

            m_writer->writeStartElement(dbNamespace, "member");
            if (QString target{linkForNode(group, nullptr)}; !target.isEmpty())
                generateSimpleLink(target, group->fullTitle());
            else
                m_writer->writeCharacters(group->name());
            m_writer->writeEndElement(); // member
        }

        m_writer->writeEndElement(); // simplelist
        m_writer->writeEndElement(); // para
        newLine();
    }
}

/*!
  Generates text that explains how threadsafe and/or reentrant
  \a node is.
 */
bool DocBookGenerator::generateThreadSafeness(const Node *node)
{
    // From Generator::generateThreadSafeness
    Node::ThreadSafeness ts = node->threadSafeness();

    const Node *reentrantNode;
    Atom reentrantAtom = Atom(Atom::Link, "reentrant");
    QString linkReentrant = getAutoLink(&reentrantAtom, node, &reentrantNode);
    const Node *threadSafeNode;
    Atom threadSafeAtom = Atom(Atom::Link, "thread-safe");
    QString linkThreadSafe = getAutoLink(&threadSafeAtom, node, &threadSafeNode);

    if (ts == Node::NonReentrant) {
        m_writer->writeStartElement(dbNamespace, "warning");
        newLine();
        m_writer->writeStartElement(dbNamespace, "para");
        m_writer->writeCharacters("This " + typeString(node) + " is not ");
        generateSimpleLink(linkReentrant, "reentrant");
        m_writer->writeCharacters(".");
        m_writer->writeEndElement(); // para
        newLine();
        m_writer->writeEndElement(); // warning

        return true;
    } else if (ts == Node::Reentrant || ts == Node::ThreadSafe) {
        m_writer->writeStartElement(dbNamespace, "note");
        newLine();
        m_writer->writeStartElement(dbNamespace, "para");

        if (node->isAggregate()) {
            m_writer->writeCharacters("All functions in this " + typeString(node) + " are ");
            if (ts == Node::ThreadSafe)
                generateSimpleLink(linkThreadSafe, "thread-safe");
            else
                generateSimpleLink(linkReentrant, "reentrant");

            NodeList reentrant;
            NodeList threadsafe;
            NodeList nonreentrant;
            bool exceptions = hasExceptions(node, reentrant, threadsafe, nonreentrant);
            if (!exceptions || (ts == Node::Reentrant && !threadsafe.isEmpty())) {
                m_writer->writeCharacters(".");
                m_writer->writeEndElement(); // para
                newLine();
            } else {
                m_writer->writeCharacters(" with the following exceptions:");
                m_writer->writeEndElement(); // para
                newLine();
                m_writer->writeStartElement(dbNamespace, "para");

                if (ts == Node::Reentrant) {
                    if (!nonreentrant.isEmpty()) {
                        m_writer->writeCharacters("These functions are not ");
                        generateSimpleLink(linkReentrant, "reentrant");
                        m_writer->writeCharacters(":");
                        m_writer->writeEndElement(); // para
                        newLine();
                        generateSignatureList(nonreentrant);
                    }
                    if (!threadsafe.isEmpty()) {
                        m_writer->writeCharacters("These functions are also ");
                        generateSimpleLink(linkThreadSafe, "thread-safe");
                        m_writer->writeCharacters(":");
                        m_writer->writeEndElement(); // para
                        newLine();
                        generateSignatureList(threadsafe);
                    }
                } else { // thread-safe
                    if (!reentrant.isEmpty()) {
                        m_writer->writeCharacters("These functions are only ");
                        generateSimpleLink(linkReentrant, "reentrant");
                        m_writer->writeCharacters(":");
                        m_writer->writeEndElement(); // para
                        newLine();
                        generateSignatureList(reentrant);
                    }
                    if (!nonreentrant.isEmpty()) {
                        m_writer->writeCharacters("These functions are not ");
                        generateSimpleLink(linkReentrant, "reentrant");
                        m_writer->writeCharacters(":");
                        m_writer->writeEndElement(); // para
                        newLine();
                        generateSignatureList(nonreentrant);
                    }
                }
            }
        } else {
            m_writer->writeCharacters("This " + typeString(node) + " is ");
            if (ts == Node::ThreadSafe)
                generateSimpleLink(linkThreadSafe, "thread-safe");
            else
                generateSimpleLink(linkReentrant, "reentrant");
            m_writer->writeCharacters(".");
            m_writer->writeEndElement(); // para
            newLine();
        }
        m_writer->writeEndElement(); // note
        newLine();

        return true;
    }

    return false;
}

/*!
  Generate the body of the documentation from the qdoc comment
  found with the entity represented by the \a node.
 */
void DocBookGenerator::generateBody(const Node *node)
{
    // From Generator::generateBody, without warnings.
    const FunctionNode *fn = node->isFunction() ? static_cast<const FunctionNode *>(node) : nullptr;

    if (!node->hasDoc() && !node->hasSharedDoc()) {
        /*
          Test for special function, like a destructor or copy constructor,
          that has no documentation.
        */
        if (fn) {
            QString t;
            if (fn->isDtor()) {
                t = "Destroys the instance of " + fn->parent()->name() + ".";
                if (fn->isVirtual())
                    t += " The destructor is virtual.";
            } else if (fn->isCtor()) {
                t = "Default constructs an instance of " + fn->parent()->name() + ".";
            } else if (fn->isCCtor()) {
                t = "Copy constructor.";
            } else if (fn->isMCtor()) {
                t = "Move-copy constructor.";
            } else if (fn->isCAssign()) {
                t = "Copy-assignment constructor.";
            } else if (fn->isMAssign()) {
                t = "Move-assignment constructor.";
            }

            if (!t.isEmpty())
                m_writer->writeTextElement(dbNamespace, "para", t);
        }
    } else if (!node->isSharingComment()) {
        // Reimplements clause and type alias info precede body text
        if (fn && !fn->overridesThis().isEmpty())
            generateReimplementsClause(fn);
        else if (node->isProperty()) {
            if (static_cast<const PropertyNode *>(node)->propertyType() != PropertyNode::PropertyType::StandardProperty)
                generateAddendum(node, BindableProperty, nullptr, false);
        }

        // Generate the body.
        if (!generateText(node->doc().body(), node)) {
            if (node->isMarkedReimp())
                return;
        }

        // Output what is after the main body.
        if (fn) {
            if (fn->isQmlSignal())
                generateAddendum(node, QmlSignalHandler, nullptr, true);
            if (fn->isPrivateSignal())
                generateAddendum(node, PrivateSignal, nullptr, true);
            if (fn->isInvokable())
                generateAddendum(node, Invokable, nullptr, true);
            if (fn->hasAssociatedProperties())
                generateAddendum(node, AssociatedProperties, nullptr, true);
        }

        // Warning generation skipped with respect to Generator::generateBody.
    }

    generateRequiredLinks(node);
}

/*!
  Generates either a link to the project folder for example \a node, or a list
  of links files/images if 'url.examples config' variable is not defined.

  Does nothing for non-example nodes.
*/
void DocBookGenerator::generateRequiredLinks(const Node *node)
{
    // From Generator::generateRequiredLinks.
    if (!node->isExample())
        return;

    const auto en = static_cast<const ExampleNode *>(node);
    QString exampleUrl{Config::instance().get(CONFIG_URL + Config::dot + CONFIG_EXAMPLES).asString()};

    if (exampleUrl.isEmpty()) {
        if (!en->noAutoList()) {
            generateFileList(en, false); // files
            generateFileList(en, true); // images
        }
    } else {
        generateLinkToExample(en, exampleUrl);
    }
}

/*!
  The path to the example replaces a placeholder '\1' character if
  one is found in the \a baseUrl string.  If no such placeholder is found,
  the path is appended to \a baseUrl, after a '/' character if \a baseUrl did
  not already end in one.
*/
void DocBookGenerator::generateLinkToExample(const ExampleNode *en, const QString &baseUrl)
{
    // From Generator::generateLinkToExample.
    QString exampleUrl(baseUrl);
    QString link;
#ifndef QT_BOOTSTRAPPED
    link = QUrl(exampleUrl).host();
#endif
    if (!link.isEmpty())
        link.prepend(" @ ");
    link.prepend("Example project");

    const QLatin1Char separator('/');
    const QLatin1Char placeholder('\1');
    if (!exampleUrl.contains(placeholder)) {
        if (!exampleUrl.endsWith(separator))
            exampleUrl += separator;
        exampleUrl += placeholder;
    }

    // Construct a path to the example; <install path>/<example name>
    QStringList path = QStringList()
            << Config::instance().get(CONFIG_EXAMPLESINSTALLPATH).asString() << en->name();
    path.removeAll(QString());

    // Write the link to the example. Typically, this link comes after sections, hence
    // wrap it in a section too.
    startSection("Example project");

    m_writer->writeStartElement(dbNamespace, "para");
    generateSimpleLink(exampleUrl.replace(placeholder, path.join(separator)), link);
    m_writer->writeEndElement(); // para
    newLine();

    endSection();
}

// TODO: [multi-purpose-function-with-flag][generate-file-list]

/*!
  This function is called when the documentation for an example is
  being formatted. It outputs a list of files for the example, which
  can be the example's source files or the list of images used by the
  example. The images are copied into a subtree of
  \c{...doc/html/images/used-in-examples/...}
*/
void DocBookGenerator::generateFileList(const ExampleNode *en, bool images)
{
    // TODO: [possibly-stale-duplicate-code][generator-insufficient-structural-abstraction]
    // Review and compare this code with
    // Generator::generateFileList.
    // Some subtle changes that might be semantically equivalent are
    // present between the two.
    // Supposedly, this version is to be considered stale compared to
    // Generator's one and it might be possible to remove it in favor
    // of that as long as the difference in output are taken into consideration.

    // From Generator::generateFileList
    QString tag;
    QStringList paths;
    if (images) {
        paths = en->images();
        tag = "Images:";
    } else { // files
        paths = en->files();
        tag = "Files:";
    }
    std::sort(paths.begin(), paths.end(), Generator::comparePaths);

    if (paths.isEmpty())
        return;

    startSection("", "List of Files");

    m_writer->writeStartElement(dbNamespace, "para");
    m_writer->writeCharacters(tag);
    m_writer->writeEndElement(); // para
    newLine();

    startSection("List of Files");

    m_writer->writeStartElement(dbNamespace, "itemizedlist");
    newLine();

    for (const auto &path : std::as_const(paths)) {
        auto maybe_resolved_file{file_resolver.resolve(path)};
        if (!maybe_resolved_file) {
            // TODO: [uncentralized-admonition][failed-resolve-file]
            QString details = std::transform_reduce(
                file_resolver.get_search_directories().cbegin(),
                file_resolver.get_search_directories().cend(),
                u"Searched directories:"_s,
                std::plus(),
                [](const DirectoryPath &directory_path) -> QString { return u' ' + directory_path.value(); }
            );

            en->location().warning(u"Cannot find file to quote from: %1"_s.arg(path), details);

            continue;
        }

        auto file{*maybe_resolved_file};
        if (images) addImageToCopy(en, file);
        else        generateExampleFilePage(en, file);

        m_writer->writeStartElement(dbNamespace, "listitem");
        newLine();
        m_writer->writeStartElement(dbNamespace, "para");
        generateSimpleLink(file.get_query(), file.get_query());
        m_writer->writeEndElement(); // para
        m_writer->writeEndElement(); // listitem
        newLine();
    }

    m_writer->writeEndElement(); // itemizedlist
    newLine();

    endSection();
}

/*!
  Generate a file with the contents of a C++ or QML source file.
 */
void DocBookGenerator::generateExampleFilePage(const Node *node, ResolvedFile resolved_file, CodeMarker*)
{
    // TODO: [generator-insufficient-structural-abstraction]

    // From HtmlGenerator::generateExampleFilePage.
    if (!node->isExample())
        return;

    // TODO: Understand if this is safe.
    const auto en = static_cast<const ExampleNode *>(node);

    // Store current (active) writer
    QXmlStreamWriter *currentWriter = m_writer;
    m_writer = startDocument(en, resolved_file.get_path());
    generateHeader(en->fullTitle(), en->subtitle(), en);

    Text text;
    Quoter quoter;
    Doc::quoteFromFile(en->doc().location(), quoter, resolved_file);
    QString code = quoter.quoteTo(en->location(), QString(), QString());
    CodeMarker *codeMarker = CodeMarker::markerForFileName(resolved_file.get_path());
    text << Atom(codeMarker->atomType(), code);
    Atom a(codeMarker->atomType(), code);
    generateText(text, en);

    endDocument(); // Delete m_writer.
    m_writer = currentWriter; // Restore writer.
}

void DocBookGenerator::generateReimplementsClause(const FunctionNode *fn)
{
    // From Generator::generateReimplementsClause, without warning generation.
    if (fn->overridesThis().isEmpty() || !fn->parent()->isClassNode())
        return;

    auto cn = static_cast<ClassNode *>(fn->parent());

    if (const FunctionNode *overrides = cn->findOverriddenFunction(fn);
        overrides && !overrides->isPrivate() && !overrides->parent()->isPrivate()) {
        if (overrides->hasDoc()) {
            m_writer->writeStartElement(dbNamespace, "para");
            m_writer->writeCharacters("Reimplements: ");
            QString fullName =
                    overrides->parent()->name() + "::" + overrides->signature(Node::SignaturePlain);
            generateFullName(overrides->parent(), fullName, overrides);
            m_writer->writeCharacters(".");
            m_writer->writeEndElement(); // para
            newLine();
            return;
        }
    }

    if (const PropertyNode *sameName = cn->findOverriddenProperty(fn); sameName && sameName->hasDoc()) {
        m_writer->writeStartElement(dbNamespace, "para");
        m_writer->writeCharacters("Reimplements an access function for property: ");
        QString fullName = sameName->parent()->name() + "::" + sameName->name();
        generateFullName(sameName->parent(), fullName, sameName);
        m_writer->writeCharacters(".");
        m_writer->writeEndElement(); // para
        newLine();
        return;
    }
}

void DocBookGenerator::generateAlsoList(const Node *node)
{
    // From Generator::generateAlsoList.
    QList<Text> alsoList = node->doc().alsoList();
    supplementAlsoList(node, alsoList);

    if (!alsoList.isEmpty()) {
        startSection("See Also");

        m_writer->writeStartElement(dbNamespace, "para");
        m_writer->writeStartElement(dbNamespace, "emphasis");
        m_writer->writeCharacters("See also ");
        m_writer->writeEndElement(); // emphasis
        newLine();

        m_writer->writeStartElement(dbNamespace, "simplelist");
        m_writer->writeAttribute("type", "vert");
        m_writer->writeAttribute("role", "see-also");
        newLine();

        for (const Text &text : alsoList) {
            m_writer->writeStartElement(dbNamespace, "member");
            generateText(text, node);
            m_writer->writeEndElement(); // member
            newLine();
        }

        m_writer->writeEndElement(); // simplelist
        newLine();

        m_writer->writeEndElement(); // para
        newLine();

        endSection();
    }
}

/*!
  Open a new file to write XML contents, including the DocBook
  opening tag.
 */
QXmlStreamWriter *DocBookGenerator::startGenericDocument(const Node *node, const QString &fileName)
{
    QFile *outFile = openSubPageFile(node, fileName);
    m_writer = new QXmlStreamWriter(outFile);
    m_writer->setAutoFormatting(false); // We need a precise handling of line feeds.

    m_writer->writeStartDocument();
    newLine();
    m_writer->writeNamespace(dbNamespace, "db");
    m_writer->writeNamespace(xlinkNamespace, "xlink");
    if (m_useITS)
        m_writer->writeNamespace(itsNamespace, "its");
    m_writer->writeStartElement(dbNamespace, "article");
    m_writer->writeAttribute("version", "5.2");
    if (!m_naturalLanguage.isEmpty())
        m_writer->writeAttribute("xml:lang", m_naturalLanguage);
    newLine();

    // Reset the state for the new document.
    sectionLevels.resize(0);
    m_inPara = false;
    m_inList = 0;

    return m_writer;
}

QXmlStreamWriter *DocBookGenerator::startDocument(const Node *node)
{
    m_hasSection = false;
    refMap.clear();

    QString fileName = Generator::fileName(node, fileExtension());
    return startGenericDocument(node, fileName);
}

QXmlStreamWriter *DocBookGenerator::startDocument(const ExampleNode *en, const QString &file)
{
    m_hasSection = false;

    QString fileName = linkForExampleFile(file);
    return startGenericDocument(en, fileName);
}

void DocBookGenerator::endDocument()
{
    m_writer->writeEndElement(); // article
    m_writer->writeEndDocument();

    m_writer->device()->close();
    delete m_writer->device();
    delete m_writer;
    m_writer = nullptr;
}

/*!
  Generate a reference page for the C++ class, namespace, or
  header file documented in \a node.
 */
void DocBookGenerator::generateCppReferencePage(Node *node)
{
    // Based on HtmlGenerator::generateCppReferencePage.
    Q_ASSERT(node->isAggregate());
    const auto aggregate = static_cast<const Aggregate *>(node);

    QString title;
    QString rawTitle;
    QString fullTitle;
    if (aggregate->isNamespace()) {
        rawTitle = aggregate->plainName();
        fullTitle = aggregate->plainFullName();
        title = rawTitle + " Namespace";
    } else if (aggregate->isClass()) {
        rawTitle = aggregate->plainName();
        QString templateDecl = node->templateDecl();
        if (!templateDecl.isEmpty())
            fullTitle = QString("%1 %2 ").arg(templateDecl, aggregate->typeWord(false));
        fullTitle += aggregate->plainFullName();
        title = rawTitle + QLatin1Char(' ') + aggregate->typeWord(true);
    } else if (aggregate->isHeader()) {
        title = fullTitle = rawTitle = aggregate->fullTitle();
    }

    QString subtitleText;
    if (rawTitle != fullTitle)
        subtitleText = fullTitle;

    // Start producing the DocBook file.
    m_writer = startDocument(node);

    // Info container.
    generateHeader(title, subtitleText, aggregate);

    generateRequisites(aggregate);
    generateStatus(aggregate);

    // Element synopsis.
    generateDocBookSynopsis(node);

    // Actual content.
    if (!aggregate->doc().isEmpty()) {
        startSection("details", "Detailed Description");

        generateBody(aggregate);
        generateAlsoList(aggregate);

        endSection();
    }

    Sections sections(const_cast<Aggregate *>(aggregate));
    SectionVector sectionVector =
            (aggregate->isNamespace() || aggregate->isHeader()) ?
                    sections.stdDetailsSections() :
                    sections.stdCppClassDetailsSections();
    for (const Section &section : sectionVector) {
        if (section.members().isEmpty())
            continue;

        startSection(section.title().toLower(), section.title());

        for (const Node *member : section.members()) {
            if (member->access() == Access::Private) // ### check necessary?
                continue;

            if (member->nodeType() != Node::Class) {
                // This function starts its own section.
                generateDetailedMember(member, aggregate);
            } else {
                startSectionBegin();
                m_writer->writeCharacters("class ");
                generateFullName(member, aggregate);
                startSectionEnd();

                generateBrief(member);

                endSection();
            }
        }

        endSection();
    }

    generateObsoleteMembers(sections);

    endDocument();
}

void DocBookGenerator::generateSynopsisInfo(const QString &key, const QString &value)
{
    m_writer->writeStartElement(dbNamespace, "synopsisinfo");
    m_writer->writeAttribute("role", key);
    m_writer->writeCharacters(value);
    m_writer->writeEndElement(); // synopsisinfo
    newLine();
}

void DocBookGenerator::generateModifier(const QString &value)
{
    m_writer->writeTextElement(dbNamespace, "modifier", value);
    newLine();
}

/*!
  Generate the metadata for the given \a node in DocBook.
 */
void DocBookGenerator::generateDocBookSynopsis(const Node *node)
{
    if (!node)
        return;

    // From Generator::generateStatus, HtmlGenerator::generateRequisites,
    // Generator::generateThreadSafeness, QDocIndexFiles::generateIndexSection.

    // This function is the major place where DocBook extensions are used.
    if (!m_useDocBook52)
        return;

    // Nothing to export in some cases. Note that isSharedCommentNode() returns
    // true also for QML property groups.
    if (node->isGroup() || node->isSharedCommentNode() || node->isModule() || node->isQmlModule() || node->isPageNode())
        return;

    // Cast the node to several subtypes (null pointer if the node is not of the required type).
    const Aggregate *aggregate =
            node->isAggregate() ? static_cast<const Aggregate *>(node) : nullptr;
    const ClassNode *classNode = node->isClass() ? static_cast<const ClassNode *>(node) : nullptr;
    const FunctionNode *functionNode =
            node->isFunction() ? static_cast<const FunctionNode *>(node) : nullptr;
    const PropertyNode *propertyNode =
            node->isProperty() ? static_cast<const PropertyNode *>(node) : nullptr;
    const VariableNode *variableNode =
            node->isVariable() ? static_cast<const VariableNode *>(node) : nullptr;
    const EnumNode *enumNode = node->isEnumType() ? static_cast<const EnumNode *>(node) : nullptr;
    const QmlPropertyNode *qpn =
            node->isQmlProperty() ? static_cast<const QmlPropertyNode *>(node) : nullptr;
    const QmlTypeNode *qcn = node->isQmlType() ? static_cast<const QmlTypeNode *>(node) : nullptr;
    // Typedefs are ignored, as they correspond to enums.
    // Groups and modules are ignored.
    // Documents are ignored, they have no interesting metadata.

    // Start the synopsis tag.
    QString synopsisTag = nodeToSynopsisTag(node);
    m_writer->writeStartElement(dbNamespace, synopsisTag);
    newLine();

    // Name and basic properties of each tag (like types and parameters).
    if (node->isClass()) {
        m_writer->writeStartElement(dbNamespace, "ooclass");
        m_writer->writeTextElement(dbNamespace, "classname", node->plainName());
        m_writer->writeEndElement(); // ooclass
        newLine();
    } else if (node->isNamespace()) {
        m_writer->writeTextElement(dbNamespace, "namespacename", node->plainName());
        newLine();
    } else if (node->isQmlType()) {
        m_writer->writeStartElement(dbNamespace, "ooclass");
        m_writer->writeTextElement(dbNamespace, "classname", node->plainName());
        m_writer->writeEndElement(); // ooclass
        newLine();
        if (!qcn->groupNames().isEmpty())
            m_writer->writeAttribute("groups", qcn->groupNames().join(QLatin1Char(',')));
    } else if (node->isProperty()) {
        m_writer->writeTextElement(dbNamespace, "modifier", "(Qt property)");
        newLine();
        m_writer->writeTextElement(dbNamespace, "type", propertyNode->dataType());
        newLine();
        m_writer->writeTextElement(dbNamespace, "varname", node->plainName());
        newLine();
    } else if (node->isVariable()) {
        if (variableNode->isStatic()) {
            m_writer->writeTextElement(dbNamespace, "modifier", "static");
            newLine();
        }
        m_writer->writeTextElement(dbNamespace, "type", variableNode->dataType());
        newLine();
        m_writer->writeTextElement(dbNamespace, "varname", node->plainName());
        newLine();
    } else if (node->isEnumType()) {
        m_writer->writeTextElement(dbNamespace, "enumname", node->plainName());
        newLine();
    } else if (node->isQmlProperty()) {
        QString name = node->name();
        if (qpn->isAttached())
            name.prepend(qpn->element() + QLatin1Char('.'));

        m_writer->writeTextElement(dbNamespace, "type", qpn->dataType());
        newLine();
        m_writer->writeTextElement(dbNamespace, "varname", name);
        newLine();

        if (qpn->isAttached()) {
            m_writer->writeTextElement(dbNamespace, "modifier", "attached");
            newLine();
        }
        if (!(const_cast<QmlPropertyNode *>(qpn))->isReadOnly()) {
            m_writer->writeTextElement(dbNamespace, "modifier", "writable");
            newLine();
        }
        if ((const_cast<QmlPropertyNode *>(qpn))->isRequired()) {
            m_writer->writeTextElement(dbNamespace, "modifier", "required");
            newLine();
        }
        if (qpn->isReadOnly()) {
            generateModifier("[read-only]");
            newLine();
        }
        if (qpn->isDefault()) {
            generateModifier("[default]");
            newLine();
        }
    } else if (node->isFunction()) {
        if (functionNode->virtualness() != "non")
            generateModifier("virtual");
        if (functionNode->isConst())
            generateModifier("const");
        if (functionNode->isStatic())
            generateModifier("static");

        if (!functionNode->isMacro() && !functionNode->isCtor() &&
                !functionNode->isCCtor() && !functionNode->isMCtor()
                && !functionNode->isDtor()) {
            if (functionNode->returnType() == "void")
                m_writer->writeEmptyElement(dbNamespace, "void");
            else
                m_writer->writeTextElement(dbNamespace, "type", functionNode->returnType());
            newLine();
        }
        // Remove two characters from the plain name to only get the name
        // of the method without parentheses (only for functions, not macros).
        QString name = node->plainName();
        if (name.endsWith("()"))
            name.chop(2);
        m_writer->writeTextElement(dbNamespace, "methodname", name);
        newLine();

        if (functionNode->parameters().isEmpty()) {
            m_writer->writeEmptyElement(dbNamespace, "void");
            newLine();
        }

        const Parameters &lp = functionNode->parameters();
        for (int i = 0; i < lp.count(); ++i) {
            const Parameter &parameter = lp.at(i);
            m_writer->writeStartElement(dbNamespace, "methodparam");
            newLine();
            m_writer->writeTextElement(dbNamespace, "type", parameter.type());
            newLine();
            m_writer->writeTextElement(dbNamespace, "parameter", parameter.name());
            newLine();
            if (!parameter.defaultValue().isEmpty()) {
                m_writer->writeTextElement(dbNamespace, "initializer", parameter.defaultValue());
                newLine();
            }
            m_writer->writeEndElement(); // methodparam
            newLine();
        }

        if (functionNode->isDefault())
            generateModifier("default");
        if (functionNode->isFinal())
            generateModifier("final");
        if (functionNode->isOverride())
            generateModifier("override");
    } else if (node->isTypedef()) {
        m_writer->writeTextElement(dbNamespace, "typedefname", node->plainName());
        newLine();
    } else {
        node->doc().location().warning(
            QStringLiteral("Unexpected node type in generateDocBookSynopsis: %1")
                .arg(node->nodeTypeString()));
        newLine();
    }

    // Enums and typedefs.
    if (enumNode) {
        for (const EnumItem &item : enumNode->items()) {
            m_writer->writeStartElement(dbNamespace, "enumitem");
            newLine();
            m_writer->writeTextElement(dbNamespace, "enumidentifier", item.name());
            newLine();
            m_writer->writeTextElement(dbNamespace, "enumvalue", item.value());
            newLine();
            m_writer->writeEndElement(); // enumitem
            newLine();
        }

        if (enumNode->items().isEmpty()) {
            // If the enumeration is empty (really rare case), still produce
            // something for the DocBook document to be valid.
            m_writer->writeStartElement(dbNamespace, "enumitem");
            newLine();
            m_writer->writeEmptyElement(dbNamespace, "enumidentifier");
            newLine();
            m_writer->writeEndElement(); // enumitem
            newLine();
        }
    }

    // Below: only synopsisinfo within synopsisTag. These elements must be at
    // the end of the tag, as per DocBook grammar.

    // Information for functions that could not be output previously
    // (synopsisinfo).
    if (node->isFunction()) {
        generateSynopsisInfo("meta", functionNode->metanessString());

        if (functionNode->isOverload()) {
            generateSynopsisInfo("overload", "overload");
            generateSynopsisInfo("overload-number",
                                 QString::number(functionNode->overloadNumber()));
        }

        if (functionNode->isRef())
            generateSynopsisInfo("refness", QString::number(1));
        else if (functionNode->isRefRef())
            generateSynopsisInfo("refness", QString::number(2));

        if (functionNode->hasAssociatedProperties()) {
            QStringList associatedProperties;
            const auto &nodes = functionNode->associatedProperties();
            for (const Node *n : nodes) {
                const auto pn = static_cast<const PropertyNode *>(n);
                associatedProperties << pn->name();
            }
            associatedProperties.sort();
            generateSynopsisInfo("associated-property",
                                 associatedProperties.join(QLatin1Char(',')));
        }

        QString signature = functionNode->signature(Node::SignatureReturnType);
        // 'const' is already part of FunctionNode::signature()
        if (functionNode->isFinal())
            signature += " final";
        if (functionNode->isOverride())
            signature += " override";
        if (functionNode->isPureVirtual())
            signature += " = 0";
        else if (functionNode->isDefault())
            signature += " = default";
        generateSynopsisInfo("signature", signature);
    }

    // Accessibility status.
    if (!node->isPageNode() && !node->isCollectionNode()) {
        switch (node->access()) {
        case Access::Public:
            generateSynopsisInfo("access", "public");
            break;
        case Access::Protected:
            generateSynopsisInfo("access", "protected");
            break;
        case Access::Private:
            generateSynopsisInfo("access", "private");
            break;
        default:
            break;
        }
        if (node->isAbstract())
            generateSynopsisInfo("abstract", "true");
    }

    // Status.
    switch (node->status()) {
    case Node::Active:
        generateSynopsisInfo("status", "active");
        break;
    case Node::Preliminary:
        generateSynopsisInfo("status", "preliminary");
        break;
    case Node::Deprecated:
        generateSynopsisInfo("status", "deprecated");
        break;
    case Node::Internal:
        generateSynopsisInfo("status", "internal");
        break;
    default:
        generateSynopsisInfo("status", "main");
        break;
    }

    // C++ classes and name spaces.
    if (aggregate) {
        // Includes.
        if (aggregate->includeFile()) generateSynopsisInfo("headers", *aggregate->includeFile());

        // Since and project.
        if (!aggregate->since().isEmpty())
            generateSynopsisInfo("since", formatSince(aggregate));

        if (aggregate->nodeType() == Node::Class || aggregate->nodeType() == Node::Namespace) {
            // CMake and QT variable.
            if (!aggregate->physicalModuleName().isEmpty()) {
                const CollectionNode *cn =
                        m_qdb->getCollectionNode(aggregate->physicalModuleName(), Node::Module);
                if (cn && !cn->qtCMakeComponent().isEmpty()) {
                    const QString qtComponent = "Qt" + QString::number(QT_VERSION_MAJOR);
                    const QString findpackageText = "find_package(" + qtComponent
                            + " REQUIRED COMPONENTS " + cn->qtCMakeComponent() + ")";
                    const QString targetLinkLibrariesText =
                            "target_link_libraries(mytarget PRIVATE " + qtComponent + "::" + cn->qtCMakeComponent()
                            + ")";
                    generateSynopsisInfo("cmake-find-package", findpackageText);
                    generateSynopsisInfo("cmake-target-link-libraries", targetLinkLibrariesText);
                }
                if (cn && !cn->qtVariable().isEmpty())
                    generateSynopsisInfo("qmake", "QT += " + cn->qtVariable());
            }
        }

        if (aggregate->nodeType() == Node::Class) {
            // Instantiated by.
            auto *classe = const_cast<ClassNode *>(static_cast<const ClassNode *>(aggregate));
            if (classe->qmlElement() != nullptr && classe->status() != Node::Internal) {
                const Node *otherNode = nullptr;
                Atom a = Atom(Atom::LinkNode, CodeMarker::stringForNode(classe->qmlElement()));
                QString link = getAutoLink(&a, aggregate, &otherNode);

                m_writer->writeStartElement(dbNamespace, "synopsisinfo");
                m_writer->writeAttribute("role", "instantiatedBy");
                generateSimpleLink(link, classe->qmlElement()->name());
                m_writer->writeEndElement(); // synopsisinfo
                newLine();
            }

            // Inherits.
            QList<RelatedClass>::ConstIterator r;
            if (!classe->baseClasses().isEmpty()) {
                m_writer->writeStartElement(dbNamespace, "synopsisinfo");
                m_writer->writeAttribute("role", "inherits");

                r = classe->baseClasses().constBegin();
                int index = 0;
                while (r != classe->baseClasses().constEnd()) {
                    if ((*r).m_node) {
                        generateFullName((*r).m_node, classe);

                        if ((*r).m_access == Access::Protected) {
                            m_writer->writeCharacters(" (protected)");
                        } else if ((*r).m_access == Access::Private) {
                            m_writer->writeCharacters(" (private)");
                        }
                        m_writer->writeCharacters(
                                Utilities::comma(index++, classe->baseClasses().size()));
                    }
                    ++r;
                }

                m_writer->writeEndElement(); // synopsisinfo
                newLine();
            }

            // Inherited by.
            if (!classe->derivedClasses().isEmpty()) {
                m_writer->writeStartElement(dbNamespace, "synopsisinfo");
                m_writer->writeAttribute("role", "inheritedBy");
                generateSortedNames(classe, classe->derivedClasses());
                m_writer->writeEndElement(); // synopsisinfo
                newLine();
            }
        }
    }

    // QML types.
    if (qcn) {
        // Module name and version (i.e. import).
        QString logicalModuleVersion;
        const CollectionNode *collection =
                m_qdb->getCollectionNode(qcn->logicalModuleName(), qcn->nodeType());
        if (collection)
            logicalModuleVersion = collection->logicalModuleVersion();
        else
            logicalModuleVersion = qcn->logicalModuleVersion();

        QStringList importText;
        importText << "import " + qcn->logicalModuleName();
        if (!logicalModuleVersion.isEmpty())
            importText << logicalModuleVersion;
        generateSynopsisInfo("import", importText.join(' '));

        // Since and project.
        if (!qcn->since().isEmpty())
            generateSynopsisInfo("since", formatSince(qcn));

        // Inherited by.
        NodeList subs;
        QmlTypeNode::subclasses(qcn, subs);
        if (!subs.isEmpty()) {
            m_writer->writeTextElement(dbNamespace, "synopsisinfo");
            m_writer->writeAttribute("role", "inheritedBy");
            generateSortedQmlNames(qcn, subs);
            m_writer->writeEndElement(); // synopsisinfo
            newLine();
        }

        // Inherits.
        QmlTypeNode *base = qcn->qmlBaseNode();
        while (base && base->isInternal())
            base = base->qmlBaseNode();
        if (base) {
            const Node *otherNode = nullptr;
            Atom a = Atom(Atom::LinkNode, CodeMarker::stringForNode(base));
            QString link = getAutoLink(&a, base, &otherNode);

            m_writer->writeTextElement(dbNamespace, "synopsisinfo");
            m_writer->writeAttribute("role", "inherits");
            generateSimpleLink(link, base->name());
            m_writer->writeEndElement(); // synopsisinfo
            newLine();
        }

        // Instantiates.
        ClassNode *cn = (const_cast<QmlTypeNode *>(qcn))->classNode();
        if (cn && (cn->status() != Node::Internal)) {
            const Node *otherNode = nullptr;
            Atom a = Atom(Atom::LinkNode, CodeMarker::stringForNode(qcn));
            QString link = getAutoLink(&a, cn, &otherNode);

            m_writer->writeTextElement(dbNamespace, "synopsisinfo");
            m_writer->writeAttribute("role", "instantiates");
            generateSimpleLink(link, cn->name());
            m_writer->writeEndElement(); // synopsisinfo
            newLine();
        }
    }

    // Thread safeness.
    switch (node->threadSafeness()) {
    case Node::UnspecifiedSafeness:
        generateSynopsisInfo("threadsafeness", "unspecified");
        break;
    case Node::NonReentrant:
        generateSynopsisInfo("threadsafeness", "non-reentrant");
        break;
    case Node::Reentrant:
        generateSynopsisInfo("threadsafeness", "reentrant");
        break;
    case Node::ThreadSafe:
        generateSynopsisInfo("threadsafeness", "thread safe");
        break;
    default:
        generateSynopsisInfo("threadsafeness", "unspecified");
        break;
    }

    // Module.
    if (!node->physicalModuleName().isEmpty())
        generateSynopsisInfo("module", node->physicalModuleName());

    // Group.
    if (classNode && !classNode->groupNames().isEmpty()) {
        generateSynopsisInfo("groups", classNode->groupNames().join(QLatin1Char(',')));
    } else if (qcn && !qcn->groupNames().isEmpty()) {
        generateSynopsisInfo("groups", qcn->groupNames().join(QLatin1Char(',')));
    }

    // Properties.
    if (propertyNode) {
        for (const Node *fnNode : propertyNode->getters()) {
            if (fnNode) {
                const auto funcNode = static_cast<const FunctionNode *>(fnNode);
                generateSynopsisInfo("getter", funcNode->name());
            }
        }
        for (const Node *fnNode : propertyNode->setters()) {
            if (fnNode) {
                const auto funcNode = static_cast<const FunctionNode *>(fnNode);
                generateSynopsisInfo("setter", funcNode->name());
            }
        }
        for (const Node *fnNode : propertyNode->resetters()) {
            if (fnNode) {
                const auto funcNode = static_cast<const FunctionNode *>(fnNode);
                generateSynopsisInfo("resetter", funcNode->name());
            }
        }
        for (const Node *fnNode : propertyNode->notifiers()) {
            if (fnNode) {
                const auto funcNode = static_cast<const FunctionNode *>(fnNode);
                generateSynopsisInfo("notifier", funcNode->name());
            }
        }
    }

    m_writer->writeEndElement(); // nodeToSynopsisTag (like classsynopsis)
    newLine();

    // The typedef associated to this enum. It is output *after* the main tag,
    // i.e. it must be after the synopsisinfo.
    if (enumNode && enumNode->flagsType()) {
        m_writer->writeStartElement(dbNamespace, "typedefsynopsis");
        newLine();

        m_writer->writeTextElement(dbNamespace, "typedefname",
                                   enumNode->flagsType()->fullDocumentName());
        newLine();

        m_writer->writeEndElement(); // typedefsynopsis
        newLine();
    }
}

QString taggedNode(const Node *node)
{
    // From CodeMarker::taggedNode, but without the tag part (i.e. only the QML specific case
    // remaining).
    // TODO: find a better name for this.
    if (node->nodeType() == Node::QmlType && node->name().startsWith(QLatin1String("QML:")))
        return node->name().mid(4);
    return node->name();
}

/*!
  Parses a string with method/variable name and (return) type
  to include type tags.
 */
void DocBookGenerator::typified(const QString &string, const Node *relative, bool trailingSpace,
                                bool generateType)
{
    // Adapted from CodeMarker::typified and HtmlGenerator::highlightedCode.
    // Note: CppCodeMarker::markedUpIncludes is not needed for DocBook, as this part is natively
    // generated as DocBook. Hence, there is no need to reimplement <@headerfile> from
    // HtmlGenerator::highlightedCode.
    QString result;
    QString pendingWord;

    for (int i = 0; i <= string.size(); ++i) {
        QChar ch;
        if (i != string.size())
            ch = string.at(i);

        QChar lower = ch.toLower();
        if ((lower >= QLatin1Char('a') && lower <= QLatin1Char('z')) || ch.digitValue() >= 0
            || ch == QLatin1Char('_') || ch == QLatin1Char(':')) {
            pendingWord += ch;
        } else {
            if (!pendingWord.isEmpty()) {
                bool isProbablyType = (pendingWord != QLatin1String("const"));
                if (generateType && isProbablyType) {
                    // Flush the current buffer.
                    m_writer->writeCharacters(result);
                    result.truncate(0);

                    // Add the link, logic from HtmlGenerator::highlightedCode.
                    const Node *n = m_qdb->findTypeNode(pendingWord, relative, Node::DontCare);
                    QString href;
                    if (!(n && n->isQmlBasicType())
                        || (relative
                            && (relative->genus() == n->genus() || Node::DontCare == n->genus()))) {
                        href = linkForNode(n, relative);
                    }

                    m_writer->writeStartElement(dbNamespace, "type");
                    if (href.isEmpty())
                        m_writer->writeCharacters(pendingWord);
                    else
                        generateSimpleLink(href, pendingWord);
                    m_writer->writeEndElement(); // type
                } else {
                    result += pendingWord;
                }
            }
            pendingWord.clear();

            if (ch.unicode() != '\0')
                result += ch;
        }
    }

    if (trailingSpace && string.size()) {
        if (!string.endsWith(QLatin1Char('*')) && !string.endsWith(QLatin1Char('&')))
            result += QLatin1Char(' ');
    }

    m_writer->writeCharacters(result);
}

void DocBookGenerator::generateSynopsisName(const Node *node, const Node *relative,
                                            bool generateNameLink)
{
    // Implements the rewriting of <@link> from HtmlGenerator::highlightedCode, only due to calls to
    // CodeMarker::linkTag in CppCodeMarker::markedUpSynopsis.
    QString name = taggedNode(node);

    if (!generateNameLink) {
        m_writer->writeCharacters(name);
        return;
    }

    m_writer->writeStartElement(dbNamespace, "emphasis");
    m_writer->writeAttribute("role", "bold");
    generateSimpleLink(linkForNode(node, relative), name);
    m_writer->writeEndElement(); // emphasis
}

void DocBookGenerator::generateParameter(const Parameter &parameter, const Node *relative,
                                         bool generateExtra, bool generateType)
{
    const QString &pname = parameter.name();
    const QString &ptype = parameter.type();
    QString paramName;
    if (!pname.isEmpty()) {
        typified(ptype, relative, true, generateType);
        paramName = pname;
    } else {
        paramName = ptype;
    }
    if (generateExtra || pname.isEmpty()) {
        // Look for the _ character in the member name followed by a number (or n):
        // this is intended to be rendered as a subscript.
        static const QRegularExpression sub("([a-z]+)_([0-9]+|n)");

        m_writer->writeStartElement(dbNamespace, "emphasis");
        auto match = sub.match(paramName);
        if (match.hasMatch()) {
            m_writer->writeCharacters(match.captured(0));
            m_writer->writeStartElement(dbNamespace, "subscript");
            m_writer->writeCharacters(match.captured(1));
            m_writer->writeEndElement(); // subscript
        } else {
            m_writer->writeCharacters(paramName);
        }
        m_writer->writeEndElement(); // emphasis
    }

    const QString &pvalue = parameter.defaultValue();
    if (generateExtra && !pvalue.isEmpty())
        m_writer->writeCharacters(" = " + pvalue);
}

void DocBookGenerator::generateSynopsis(const Node *node, const Node *relative,
                                        Section::Style style)
{
    // From HtmlGenerator::generateSynopsis (conditions written as booleans).
    const bool generateExtra = style != Section::AllMembers;
    const bool generateType = style != Section::Details;
    const bool generateNameLink = style != Section::Details;

    // From CppCodeMarker::markedUpSynopsis, reversed the generation of "extra" and "synopsis".
    const int MaxEnumValues = 6;

    if (generateExtra)
        m_writer->writeCharacters(CodeMarker::extraSynopsis(node, style));

    // Then generate the synopsis.
    QString namePrefix {};
    if (style == Section::Details) {
        if (!node->isRelatedNonmember() && !node->isProxyNode() && !node->parent()->name().isEmpty()
            && !node->parent()->isHeader() && !node->isProperty() && !node->isQmlNode()) {
            namePrefix = taggedNode(node->parent()) + "::";
        }
    }

    switch (node->nodeType()) {
    case Node::Namespace:
        m_writer->writeCharacters("namespace ");
        m_writer->writeCharacters(namePrefix);
        generateSynopsisName(node, relative, generateNameLink);
        break;
    case Node::Class:
        m_writer->writeCharacters("class ");
        m_writer->writeCharacters(namePrefix);
        generateSynopsisName(node, relative, generateNameLink);
        break;
    case Node::Function: {
        const auto func = (const FunctionNode *)node;

        // First, the part coming before the name.
        if (style == Section::Summary || style == Section::Accessors) {
            if (!func->isNonvirtual())
                m_writer->writeCharacters(QStringLiteral("virtual "));
        }

        // Name and parameters.
        if (style != Section::AllMembers && !func->returnType().isEmpty())
            typified(func->returnType(), relative, true, generateType);
        m_writer->writeCharacters(namePrefix);
        generateSynopsisName(node, relative, generateNameLink);

        if (!func->isMacroWithoutParams()) {
            m_writer->writeCharacters(QStringLiteral("("));
            if (!func->parameters().isEmpty()) {
                const Parameters &parameters = func->parameters();
                for (int i = 0; i < parameters.count(); i++) {
                    if (i > 0)
                        m_writer->writeCharacters(QStringLiteral(", "));
                    generateParameter(parameters.at(i), relative, generateExtra, generateType);
                }
            }
            m_writer->writeCharacters(QStringLiteral(")"));
        }

        if (func->isConst())
            m_writer->writeCharacters(QStringLiteral(" const"));

        if (style == Section::Summary || style == Section::Accessors) {
            // virtual is prepended, if needed.
            QString synopsis;
            if (func->isFinal())
                synopsis += QStringLiteral(" final");
            if (func->isOverride())
                synopsis += QStringLiteral(" override");
            if (func->isPureVirtual())
                synopsis += QStringLiteral(" = 0");
            if (func->isRef())
                synopsis += QStringLiteral(" &");
            else if (func->isRefRef())
                synopsis += QStringLiteral(" &&");
            m_writer->writeCharacters(synopsis);
        } else if (style == Section::AllMembers) {
            if (!func->returnType().isEmpty() && func->returnType() != "void") {
                m_writer->writeCharacters(QStringLiteral(" : "));
                typified(func->returnType(), relative, false, generateType);
            }
        } else {
            QString synopsis;
            if (func->isRef())
                synopsis += QStringLiteral(" &");
            else if (func->isRefRef())
                synopsis += QStringLiteral(" &&");
            m_writer->writeCharacters(synopsis);
        }
    } break;
    case Node::Enum: {
        const auto enume = static_cast<const EnumNode *>(node);
        m_writer->writeCharacters(QStringLiteral("enum "));
        m_writer->writeCharacters(namePrefix);
        generateSynopsisName(node, relative, generateNameLink);

        QString synopsis;
        if (style == Section::Summary) {
            synopsis += " { ";

            QStringList documentedItems = enume->doc().enumItemNames();
            if (documentedItems.isEmpty()) {
                const auto &enumItems = enume->items();
                for (const auto &item : enumItems)
                    documentedItems << item.name();
            }
            const QStringList omitItems = enume->doc().omitEnumItemNames();
            for (const auto &item : omitItems)
                documentedItems.removeAll(item);

            if (documentedItems.size() > MaxEnumValues) {
                // Take the last element and keep it safe, then elide the surplus.
                const QString last = documentedItems.last();
                documentedItems = documentedItems.mid(0, MaxEnumValues - 1);
                documentedItems += "&#x2026;"; // Ellipsis: in HTML, &hellip;.
                documentedItems += last;
            }
            synopsis += documentedItems.join(QLatin1String(", "));

            if (!documentedItems.isEmpty())
                synopsis += QLatin1Char(' ');
            synopsis += QLatin1Char('}');
        }
        m_writer->writeCharacters(synopsis);
    } break;
    case Node::TypeAlias: {
        if (style == Section::Details) {
            const QString& templateDecl = node->templateDecl();
            if (!templateDecl.isEmpty())
                m_writer->writeCharacters(templateDecl + QLatin1Char(' '));
        }
        m_writer->writeCharacters(namePrefix);
        generateSynopsisName(node, relative, generateNameLink);
    } break;
    case Node::Typedef: {
        if (static_cast<const TypedefNode *>(node)->associatedEnum())
            m_writer->writeCharacters("flags ");
        m_writer->writeCharacters(namePrefix);
        generateSynopsisName(node, relative, generateNameLink);
    } break;
    case Node::Property: {
        const auto property = static_cast<const PropertyNode *>(node);
        m_writer->writeCharacters(namePrefix);
        generateSynopsisName(node, relative, generateNameLink);
        m_writer->writeCharacters(" : ");
        typified(property->qualifiedDataType(), relative, false, generateType);
    } break;
    case Node::Variable: {
        const auto variable = static_cast<const VariableNode *>(node);
        if (style == Section::AllMembers) {
            generateSynopsisName(node, relative, generateNameLink);
            m_writer->writeCharacters(" : ");
            typified(variable->dataType(), relative, false, generateType);
        } else {
            typified(variable->leftType(), relative, false, generateType);
            m_writer->writeCharacters(" ");
            m_writer->writeCharacters(namePrefix);
            generateSynopsisName(node, relative, generateNameLink);
            m_writer->writeCharacters(variable->rightType());
        }
    } break;
    default:
        m_writer->writeCharacters(namePrefix);
        generateSynopsisName(node, relative, generateNameLink);
    }
}

void DocBookGenerator::generateEnumValue(const QString &enumValue, const Node *relative)
{
    // From CppCodeMarker::markedUpEnumValue, simplifications from Generator::plainCode (removing
    // <@op>). With respect to CppCodeMarker::markedUpEnumValue, the order of generation of parents
    // must be reversed so that they are processed in the order
    if (!relative->isEnumType()) {
        m_writer->writeCharacters(enumValue);
        return;
    }

    QList<const Node *> parents;
    const Node *node = relative->parent();
    while (!node->isHeader() && node->parent()) {
        parents.prepend(node);
        if (node->parent() == relative || node->parent()->name().isEmpty())
            break;
        node = node->parent();
    }
    if (static_cast<const EnumNode *>(relative)->isScoped())
        parents << relative;

    m_writer->writeStartElement(dbNamespace, "code");
    for (auto parent : parents) {
        generateSynopsisName(parent, relative, true);
        m_writer->writeCharacters("::");
    }

    m_writer->writeCharacters(enumValue);
    m_writer->writeEndElement(); // code
}

/*!
  If the node is an overloaded signal, and a node with an
  example on how to connect to it

  Someone didn't finish writing this comment, and I don't know what this
  function is supposed to do, so I have not tried to complete the comment
  yet.
 */
void DocBookGenerator::generateOverloadedSignal(const Node *node)
{
    // From Generator::generateOverloadedSignal.
    QString code = getOverloadedSignalCode(node);
    if (code.isEmpty())
        return;

    m_writer->writeStartElement(dbNamespace, "note");
    newLine();
    m_writer->writeStartElement(dbNamespace, "para");
    m_writer->writeCharacters("Signal ");
    m_writer->writeTextElement(dbNamespace, "emphasis", node->name());
    m_writer->writeCharacters(" is overloaded in this class. To connect to this "
                              "signal by using the function pointer syntax, Qt "
                              "provides a convenient helper for obtaining the "
                              "function pointer as shown in this example:");
    m_writer->writeTextElement(dbNamespace, "code", code);
    m_writer->writeEndElement(); // para
    newLine();
    m_writer->writeEndElement(); // note
    newLine();
}

/*!
  Generates an addendum note of type \a type for \a node. \a marker
  is unused in this generator.
*/
void DocBookGenerator::generateAddendum(const Node *node, Addendum type, CodeMarker *marker,
                                        bool generateNote)
{
    Q_UNUSED(marker)
    Q_ASSERT(node && !node->name().isEmpty());
    if (generateNote) {
        m_writer->writeStartElement(dbNamespace, "note");
        newLine();
    }
    switch (type) {
    case Invokable:
        m_writer->writeStartElement(dbNamespace, "para");
        m_writer->writeCharacters(
                "This function can be invoked via the meta-object system and from QML. See ");
        generateSimpleLink(node->url(), "Q_INVOKABLE");
        m_writer->writeCharacters(".");
        m_writer->writeEndElement(); // para
        newLine();
        break;
    case PrivateSignal:
        m_writer->writeTextElement(
                dbNamespace, "para",
                "This is a private signal. It can be used in signal connections but "
                "cannot be emitted by the user.");
        break;
    case QmlSignalHandler:
    {
        QString handler(node->name());
        int prefixLocation = handler.lastIndexOf('.', -2) + 1;
        handler[prefixLocation] = handler[prefixLocation].toTitleCase();
        handler.insert(prefixLocation, QLatin1String("on"));
        m_writer->writeStartElement(dbNamespace, "para");
        m_writer->writeCharacters("The corresponding handler is ");
        m_writer->writeTextElement(dbNamespace, "code", handler);
        m_writer->writeCharacters(".");
        m_writer->writeEndElement(); // para
        newLine();
        break;
    }
    case AssociatedProperties:
    {
        if (!node->isFunction())
            return;
        const auto *fn = static_cast<const FunctionNode *>(node);
        auto propertyNodes = fn->associatedProperties();
        if (propertyNodes.isEmpty())
            return;
        std::sort(propertyNodes.begin(), propertyNodes.end(), Node::nodeNameLessThan);
        for (const auto propertyNode : std::as_const(propertyNodes)) {
            QString msg;
            const auto pn = static_cast<const PropertyNode *>(propertyNode);
            switch (pn->role(fn)) {
            case PropertyNode::FunctionRole::Getter:
                msg = QStringLiteral("Getter function");
                break;
            case PropertyNode::FunctionRole::Setter:
                msg = QStringLiteral("Setter function");
                break;
            case PropertyNode::FunctionRole::Resetter:
                msg = QStringLiteral("Resetter function");
                break;
            case PropertyNode::FunctionRole::Notifier:
                msg = QStringLiteral("Notifier signal");
                break;
            default:
                continue;
            }
            m_writer->writeStartElement(dbNamespace, "para");
            m_writer->writeCharacters(msg + " for property ");
            generateSimpleLink(linkForNode(pn, nullptr), pn->name());
            m_writer->writeCharacters(". ");
            m_writer->writeEndElement(); // para
            newLine();
        }
        break;
    }
    case BindableProperty:
    {
        const Node *linkNode;
        Atom linkAtom = Atom(Atom::Link, "QProperty");
        QString link = getAutoLink(&linkAtom, node, &linkNode);
        m_writer->writeStartElement(dbNamespace, "para");
        m_writer->writeCharacters("This property supports ");
        generateSimpleLink(link, "QProperty");
        m_writer->writeCharacters(" bindings.");
        m_writer->writeEndElement(); // para
        newLine();
        break;
    }
    default:
        break;
    }

    if (generateNote) {
        m_writer->writeEndElement(); // note
        newLine();
    }
}

void DocBookGenerator::generateDetailedMember(const Node *node, const PageNode *relative)
{
    // From HtmlGenerator::generateDetailedMember.
    bool closeSupplementarySection = false;

    if (node->isSharedCommentNode()) {
        const auto *scn = reinterpret_cast<const SharedCommentNode *>(node);
        const QList<Node *> &collective = scn->collective();

        bool firstFunction = true;
        for (const auto *sharedNode : collective) {
            if (firstFunction) {
                startSectionBegin(sharedNode);
            } else {
                m_writer->writeStartElement(dbNamespace, "bridgehead");
                m_writer->writeAttribute("renderas", "sect2");
                writeXmlId(sharedNode);
            }
            if (m_useITS)
                m_writer->writeAttribute(itsNamespace, "translate", "no");

            generateSynopsis(sharedNode, relative, Section::Details);

            if (firstFunction) {
                startSectionEnd();
                firstFunction = false;
            } else {
                m_writer->writeEndElement(); // bridgehead
                newLine();
            }
        }
    } else {
        const EnumNode *etn;
        if (node->isEnumType() && (etn = static_cast<const EnumNode *>(node))->flagsType()) {
            startSectionBegin(node);
            if (m_useITS)
                m_writer->writeAttribute(itsNamespace, "translate", "no");
            generateSynopsis(etn, relative, Section::Details);
            startSectionEnd();

            m_writer->writeStartElement(dbNamespace, "bridgehead");
            m_writer->writeAttribute("renderas", "sect2");
            generateSynopsis(etn->flagsType(), relative, Section::Details);
            m_writer->writeEndElement(); // bridgehead
            newLine();
        } else {
            startSectionBegin(node);
            if (m_useITS)
                m_writer->writeAttribute(itsNamespace, "translate", "no");
            generateSynopsis(node, relative, Section::Details);
            startSectionEnd();
        }
    }
    Q_ASSERT(m_hasSection);

    generateDocBookSynopsis(node);

    generateStatus(node);
    generateBody(node);

    // If the body ends with a section, the rest of the description must be wrapped in a section too.
    if (node->hasDoc() && node->doc().body().firstAtom() && node->doc().body().lastAtom()->type() == Atom::SectionRight) {
        closeSupplementarySection = true;
        startSection("", "Notes");
    }

    generateOverloadedSignal(node);
    generateThreadSafeness(node);
    generateSince(node);

    if (node->isProperty()) {
        const auto property = static_cast<const PropertyNode *>(node);
        if (property->propertyType() == PropertyNode::PropertyType::StandardProperty) {
            Section section("", "", "", "", Section::Accessors);

            section.appendMembers(property->getters().toVector());
            section.appendMembers(property->setters().toVector());
            section.appendMembers(property->resetters().toVector());

            if (!section.members().isEmpty()) {
                m_writer->writeStartElement(dbNamespace, "para");
                newLine();
                m_writer->writeStartElement(dbNamespace, "emphasis");
                m_writer->writeAttribute("role", "bold");
                m_writer->writeCharacters("Access functions:");
                newLine();
                m_writer->writeEndElement(); // emphasis
                newLine();
                m_writer->writeEndElement(); // para
                newLine();
                generateSectionList(section, node);
            }

            Section notifiers("", "", "", "", Section::Accessors);
            notifiers.appendMembers(property->notifiers().toVector());

            if (!notifiers.members().isEmpty()) {
                m_writer->writeStartElement(dbNamespace, "para");
                newLine();
                m_writer->writeStartElement(dbNamespace, "emphasis");
                m_writer->writeAttribute("role", "bold");
                m_writer->writeCharacters("Notifier signal:");
                newLine();
                m_writer->writeEndElement(); // emphasis
                newLine();
                m_writer->writeEndElement(); // para
                newLine();
                generateSectionList(notifiers, node);
            }
        }
    } else if (node->isEnumType()) {
        const auto en = static_cast<const EnumNode *>(node);

        if (m_qflagsHref.isEmpty()) {
            Node *qflags = m_qdb->findClassNode(QStringList("QFlags"));
            if (qflags)
                m_qflagsHref = linkForNode(qflags, nullptr);
        }

        if (en->flagsType()) {
            m_writer->writeStartElement(dbNamespace, "para");
            m_writer->writeCharacters("The ");
            m_writer->writeStartElement(dbNamespace, "code");
            m_writer->writeCharacters(en->flagsType()->name());
            m_writer->writeEndElement(); // code
            m_writer->writeCharacters(" type is a typedef for ");
            m_writer->writeStartElement(dbNamespace, "code");
            generateSimpleLink(m_qflagsHref, "QFlags");
            m_writer->writeCharacters("<" + en->name() + ">. ");
            m_writer->writeEndElement(); // code
            m_writer->writeCharacters("It stores an OR combination of ");
            m_writer->writeStartElement(dbNamespace, "code");
            m_writer->writeCharacters(en->name());
            m_writer->writeEndElement(); // code
            m_writer->writeCharacters(" values.");
            m_writer->writeEndElement(); // para
            newLine();
        }
    }

    if (closeSupplementarySection)
        endSection();

    // The list of linked pages is always in its own section.
    generateAlsoList(node);

    // Close the section for this member.
    endSection(); // section
}

void DocBookGenerator::generateSectionList(const Section &section, const Node *relative,
                                           bool useObsoleteMembers)
{
    // From HtmlGenerator::generateSectionList, just generating a list (not tables).
    const NodeVector &members =
            (useObsoleteMembers ? section.obsoleteMembers() : section.members());
    if (!members.isEmpty()) {
        bool hasPrivateSignals = false;
        bool isInvokable = false;

        m_writer->writeStartElement(dbNamespace, "itemizedlist");
        if (m_useITS)
            m_writer->writeAttribute(itsNamespace, "translate", "no");
        newLine();

        NodeVector::ConstIterator m = members.constBegin();
        while (m != members.constEnd()) {
            if ((*m)->access() == Access::Private) {
                ++m;
                continue;
            }

            m_writer->writeStartElement(dbNamespace, "listitem");
            newLine();
            m_writer->writeStartElement(dbNamespace, "para");

            // prefix no more needed.
            generateSynopsis(*m, relative, section.style());
            if ((*m)->isFunction()) {
                const auto fn = static_cast<const FunctionNode *>(*m);
                if (fn->isPrivateSignal())
                    hasPrivateSignals = true;
                else if (fn->isInvokable())
                    isInvokable = true;
            }

            m_writer->writeEndElement(); // para
            newLine();
            m_writer->writeEndElement(); // listitem
            newLine();

            ++m;
        }

        m_writer->writeEndElement(); // itemizedlist
        newLine();

        if (hasPrivateSignals)
            generateAddendum(relative, Generator::PrivateSignal, nullptr, true);
        if (isInvokable)
            generateAddendum(relative, Generator::Invokable, nullptr, true);
    }

    if (!useObsoleteMembers && section.style() == Section::Summary
        && !section.inheritedMembers().isEmpty()) {
        m_writer->writeStartElement(dbNamespace, "itemizedlist");
        if (m_useITS)
            m_writer->writeAttribute(itsNamespace, "translate", "no");
        newLine();

        generateSectionInheritedList(section, relative);

        m_writer->writeEndElement(); // itemizedlist
        newLine();
    }
}

void DocBookGenerator::generateSectionInheritedList(const Section &section, const Node *relative)
{
    // From HtmlGenerator::generateSectionInheritedList.
    QList<std::pair<Aggregate *, int>>::ConstIterator p = section.inheritedMembers().constBegin();
    while (p != section.inheritedMembers().constEnd()) {
        m_writer->writeStartElement(dbNamespace, "listitem");
        m_writer->writeCharacters(QString::number((*p).second) + u' ');
        if ((*p).second == 1)
            m_writer->writeCharacters(section.singular());
        else
            m_writer->writeCharacters(section.plural());
        m_writer->writeCharacters(" inherited from ");
        generateSimpleLink(fileName((*p).first) + '#'
                                   + Generator::cleanRef(section.title().toLower()),
                           (*p).first->plainFullName(relative));
        ++p;
    }
}

/*!
  Generate the DocBook page for an entity that doesn't map
  to any underlying parsable C++ or QML element.
 */
void DocBookGenerator::generatePageNode(PageNode *pn)
{
    // From HtmlGenerator::generatePageNode, remove anything related to TOCs.
    Q_ASSERT(m_writer == nullptr);
    m_writer = startDocument(pn);

    generateHeader(pn->fullTitle(), pn->subtitle(), pn);
    generateBody(pn);
    generateAlsoList(pn);
    generateFooter();

    endDocument();
}

/*!
  Generate the DocBook page for a QML type. \qcn is the QML type.
 */
void DocBookGenerator::generateQmlTypePage(QmlTypeNode *qcn)
{
    // From HtmlGenerator::generateQmlTypePage.
    // Start producing the DocBook file.
    Q_ASSERT(m_writer == nullptr);
    m_writer = startDocument(qcn);

    Generator::setQmlTypeContext(qcn);
    QString title = qcn->fullTitle();
    if (qcn->isQmlBasicType())
        title.append(" QML Value Type");
    else
        title.append(" QML Type");
    // TODO: for ITS attribute, only apply translate="no" on qcn->fullTitle(),
    // not its suffix (which should be translated). generateHeader doesn't
    // allow this kind of input, the title isn't supposed to be structured.
    // Ideally, do the same in HTML.

    generateHeader(title, qcn->subtitle(), qcn);
    generateQmlRequisites(qcn);

    startSection("details", "Detailed Description");
    generateBody(qcn);

    generateAlsoList(qcn);

    endSection();

    Sections sections(qcn);
    for (const auto &section : sections.stdQmlTypeDetailsSections()) {
        if (!section.isEmpty()) {
            startSection(section.title().toLower(), section.title());

            for (const auto &member : section.members())
                generateDetailedQmlMember(member, qcn);

            endSection();
        }
    }

    generateObsoleteQmlMembers(sections);

    generateFooter();
    Generator::setQmlTypeContext(nullptr);

    endDocument();
}

/*!
  Outputs the DocBook detailed documentation for a section
  on a QML element reference page.
 */
void DocBookGenerator::generateDetailedQmlMember(Node *node, const Aggregate *relative)
{
    // From HtmlGenerator::generateDetailedQmlMember, with elements from
    // CppCodeMarker::markedUpQmlItem and HtmlGenerator::generateQmlItem.
    std::function<QString(QmlPropertyNode *)> getQmlPropertyTitle = [&](QmlPropertyNode *n) {
        QString title;
        QStringList extra;
        if (n->isDefault())
            extra << "default";
        else if (n->isReadOnly())
            extra << "read-only";
        else if (n->isRequired())
            extra << "required";
        else if (!n->defaultValue().isEmpty())
            extra << "default: " + n->defaultValue();

        if (!n->since().isEmpty()) {
            if (!extra.isEmpty())
                extra.last().append(',');
            extra << "since " + n->since();
        }
        if (!extra.isEmpty())
            title = QString("[%1] ").arg(extra.join(QLatin1Char(' ')));

        // Finalise generation of name, as per CppCodeMarker::markedUpQmlItem.
        if (n->isAttached())
            title += n->element() + QLatin1Char('.');
        title += n->name() + " : " + n->dataType();

        return title;
    };

    std::function<void(Node *)> generateQmlMethodTitle = [&](Node *node) {
        generateSynopsis(node, relative, Section::Details);
    };

    if (node->isPropertyGroup()) {
        const auto *scn = static_cast<const SharedCommentNode *>(node);

        QString heading;
        if (!scn->name().isEmpty())
            heading = scn->name() + " group";
        else
            heading = node->name();
        startSection(scn, heading);
        // This last call creates a title for this section. In other words,
        // titles are forbidden for the rest of the section, hence the use of
        // bridgehead.

        const QList<Node *> sharedNodes = scn->collective();
        for (const auto &sharedNode : sharedNodes) {
            if (sharedNode->isQmlProperty()) {
                auto *qpn = static_cast<QmlPropertyNode *>(sharedNode);

                m_writer->writeStartElement(dbNamespace, "bridgehead");
                m_writer->writeAttribute("renderas", "sect2");
                writeXmlId(qpn);
                m_writer->writeCharacters(getQmlPropertyTitle(qpn));
                m_writer->writeEndElement(); // bridgehead
                newLine();

                generateDocBookSynopsis(qpn);
            }
        }
    } else if (node->isQmlProperty()) {
        auto qpn = static_cast<QmlPropertyNode *>(node);
        startSection(qpn, getQmlPropertyTitle(qpn));
        generateDocBookSynopsis(qpn);
    } else if (node->isSharedCommentNode()) {
        const auto scn = reinterpret_cast<const SharedCommentNode *>(node);
        const QList<Node *> &sharedNodes = scn->collective();

        // In the section, generate a title for the first node, then bridgeheads for
        // the next ones.
        int i = 0;
        for (const auto &sharedNode : sharedNodes) {
            // Ignore this element if there is nothing to generate.
            if (!sharedNode->isFunction(Node::QML) && !sharedNode->isQmlProperty()) {
                continue;
            }

            // Write the tag containing the title.
            if (i == 0) {
                startSectionBegin(sharedNode);
            } else {
                m_writer->writeStartElement(dbNamespace, "bridgehead");
                m_writer->writeAttribute("renderas", "sect2");
            }

            // Write the title.
            if (sharedNode->isFunction(Node::QML))
                generateQmlMethodTitle(sharedNode);
            else if (sharedNode->isQmlProperty())
                m_writer->writeCharacters(
                        getQmlPropertyTitle(static_cast<QmlPropertyNode *>(sharedNode)));

            // Complete the title and the synopsis.
            if (i == 0)
                startSectionEnd();
            else
                m_writer->writeEndElement(); // bridgehead
            generateDocBookSynopsis(sharedNode);
            ++i;
        }

        // If the list is empty, still generate a section.
        if (i == 0) {
            startSectionBegin(refForNode(node));

            if (node->isFunction(Node::QML))
                generateQmlMethodTitle(node);
            else if (node->isQmlProperty())
                m_writer->writeCharacters(
                    getQmlPropertyTitle(static_cast<QmlPropertyNode *>(node)));

            startSectionEnd();
        }
    } else { // assume the node is a method/signal handler
        startSectionBegin(node);
        generateQmlMethodTitle(node);
        startSectionEnd();
    }

    generateStatus(node);
    generateBody(node);
    generateThreadSafeness(node);
    generateSince(node);
    generateAlsoList(node);

    endSection();
}

/*!
  Recursive writing of DocBook files from the root \a node.
 */
void DocBookGenerator::generateDocumentation(Node *node)
{
    // Mainly from Generator::generateDocumentation, with parts from
    // Generator::generateDocumentation and WebXMLGenerator::generateDocumentation.
    // Don't generate nodes that are already processed, or if they're not
    // supposed to generate output, ie. external, index or images nodes.
    if (!node->url().isNull())
        return;
    if (node->isIndexNode())
        return;
    if (node->isInternal() && !m_showInternal)
        return;
    if (node->isExternalPage())
        return;

    if (node->parent()) {
        if (node->isCollectionNode()) {
            /*
              A collection node collects: groups, C++ modules, or QML
              modules. Testing for a CollectionNode must be done
              before testing for a TextPageNode because a
              CollectionNode is a PageNode at this point.

              Don't output an HTML page for the collection node unless
              the \group, \module, or \qmlmodule command was actually
              seen by qdoc in the qdoc comment for the node.

              A key prerequisite in this case is the call to
              mergeCollections(cn). We must determine whether this
              group, module, or QML module has members in other
              modules. We know at this point that cn's members list
              contains only members in the current module. Therefore,
              before outputting the page for cn, we must search for
              members of cn in the other modules and add them to the
              members list.
            */
            auto cn = static_cast<CollectionNode *>(node);
            if (cn->wasSeen()) {
                m_qdb->mergeCollections(cn);
                generateCollectionNode(cn);
            } else if (cn->isGenericCollection()) {
                // Currently used only for the module's related orphans page
                // but can be generalized for other kinds of collections if
                // other use cases pop up.
                generateGenericCollectionPage(cn);
            }
        } else if (node->isTextPageNode()) { // Pages.
            generatePageNode(static_cast<PageNode *>(node));
        } else if (node->isAggregate()) { // Aggregates.
            if ((node->isClassNode() || node->isHeader() || node->isNamespace())
                && node->docMustBeGenerated()) {
                generateCppReferencePage(static_cast<Aggregate *>(node));
            } else if (node->isQmlType()) { // Includes QML value types
                generateQmlTypePage(static_cast<QmlTypeNode *>(node));
            } else if (node->isProxyNode()) {
                generateProxyPage(static_cast<Aggregate *>(node));
            }
        }
    }

    if (node->isAggregate()) {
        auto *aggregate = static_cast<Aggregate *>(node);
        for (auto c : aggregate->childNodes()) {
            if (node->isPageNode() && !node->isPrivate())
                generateDocumentation(c);
        }
    }
}

void DocBookGenerator::generateProxyPage(Aggregate *aggregate)
{
    // Adapted from HtmlGenerator::generateProxyPage.
    Q_ASSERT(aggregate->isProxyNode());

    // Start producing the DocBook file.
    Q_ASSERT(m_writer == nullptr);
    m_writer = startDocument(aggregate);

    // Info container.
    generateHeader(aggregate->plainFullName(), "", aggregate);

    // No element synopsis.

    // Actual content.
    if (!aggregate->doc().isEmpty()) {
        startSection("details", "Detailed Description");

        generateBody(aggregate);
        generateAlsoList(aggregate);

        endSection();
    }

    Sections sections(aggregate);
    SectionVector *detailsSections = &sections.stdDetailsSections();

    for (const auto &section : std::as_const(*detailsSections)) {
        if (section.isEmpty())
            continue;

        startSection(section.title().toLower(), section.title());

        const QList<Node *> &members = section.members();
        for (const auto &member : members) {
            if (!member->isPrivate()) { // ### check necessary?
                if (!member->isClassNode()) {
                    generateDetailedMember(member, aggregate);
                } else {
                    startSectionBegin();
                    generateFullName(member, aggregate);
                    startSectionEnd();

                    generateBrief(member);
                    endSection();
                }
            }
        }

        endSection();
    }

    generateFooter();

    endDocument();
}

/*!
  Generate the HTML page for a group, module, or QML module.
 */
void DocBookGenerator::generateCollectionNode(CollectionNode *cn)
{
    // Adapted from HtmlGenerator::generateCollectionNode.
    // Start producing the DocBook file.
    Q_ASSERT(m_writer == nullptr);
    m_writer = startDocument(cn);

    // Info container.
    generateHeader(cn->fullTitle(), cn->subtitle(), cn);

    // Element synopsis.
    generateDocBookSynopsis(cn);

    // Generate brief for C++ modules, status for all modules.
    if (cn->genus() != Node::DOC && cn->genus() != Node::DontCare) {
        if (cn->isModule())
            generateBrief(cn);
        generateStatus(cn);
        generateSince(cn);
    }

    // Actual content.
    if (cn->isModule()) {
        if (!cn->noAutoList()) {
            NodeMap nmm;
            cn->getMemberNamespaces(nmm);
            if (!nmm.isEmpty()) {
                startSection("namespaces", "Namespaces");
                generateAnnotatedList(cn, nmm.values(), "namespaces");
                endSection();
            }
            nmm.clear();
            cn->getMemberClasses(nmm);
            if (!nmm.isEmpty()) {
                startSection("classes", "Classes");
                generateAnnotatedList(cn, nmm.values(), "classes");
                endSection();
            }
        }
    }

    bool generatedTitle = false;
    if (cn->isModule() && !cn->doc().briefText().isEmpty()) {
        startSection("details", "Detailed Description");
        generatedTitle = true;
    }
    // The anchor is only needed if the node has a body.
    else if (
            // generateBody generates something.
            (cn->isFunction() && ((!cn->hasDoc() && !cn->hasSharedDoc()) || !cn->isSharingComment())) ||
            cn->isExample() ||
            // generateAlsoList generates something.
            !cn->doc().alsoList().empty() ||
            // generateAnnotatedList generates something.
            (!cn->noAutoList() && (cn->isGroup() || cn->isQmlModule()))) {
        writeAnchor("details");
    }

    generateBody(cn);
    generateAlsoList(cn);

    if (!cn->noAutoList() && (cn->isGroup() || cn->isQmlModule()))
        generateAnnotatedList(cn, cn->members(), "members", AutoSection);

    if (generatedTitle)
        endSection();

    generateFooter();

    endDocument();
}

/*!
  Generate the HTML page for a generic collection. This is usually
  a collection of C++ elements that are related to an element in
  a different module.
 */
void DocBookGenerator::generateGenericCollectionPage(CollectionNode *cn)
{
    // Adapted from HtmlGenerator::generateGenericCollectionPage.
    // TODO: factor out this code to generate a file name.
    QString name = cn->name().toLower();
    name.replace(QChar(' '), QString("-"));
    QString filename = cn->tree()->physicalModuleName() + "-" + name + "." + fileExtension();

    // Start producing the DocBook file.
    Q_ASSERT(m_writer == nullptr);
    m_writer = startGenericDocument(cn, filename);

    // Info container.
    generateHeader(cn->fullTitle(), cn->subtitle(), cn);

    // Element synopsis.
    generateDocBookSynopsis(cn);

    // Actual content.
    m_writer->writeStartElement(dbNamespace, "para");
    m_writer->writeCharacters("Each function or type documented here is related to a class or "
                              "namespace that is documented in a different module. The reference "
                              "page for that class or namespace will link to the function or type "
                              "on this page.");
    m_writer->writeEndElement(); // para

    const CollectionNode *cnc = cn;
    const QList<Node *> members = cn->members();
    for (const auto &member : members)
        generateDetailedMember(member, cnc);

    generateFooter();

    endDocument();
}

void DocBookGenerator::generateFullName(const Node *node, const Node *relative)
{
    Q_ASSERT(node);
    Q_ASSERT(relative);

    // From Generator::appendFullName.
    m_writer->writeStartElement(dbNamespace, "link");
    m_writer->writeAttribute(xlinkNamespace, "href", fullDocumentLocation(node));
    m_writer->writeAttribute(xlinkNamespace, "role", targetType(node));
    m_writer->writeCharacters(node->fullName(relative));
    m_writer->writeEndElement(); // link
}

void DocBookGenerator::generateFullName(const Node *apparentNode, const QString &fullName,
                                        const Node *actualNode)
{
    Q_ASSERT(apparentNode);
    Q_ASSERT(actualNode);

    // From Generator::appendFullName.
    m_writer->writeStartElement(dbNamespace, "link");
    m_writer->writeAttribute(xlinkNamespace, "href", fullDocumentLocation(actualNode));
    m_writer->writeAttribute("role", targetType(actualNode));
    m_writer->writeCharacters(fullName);
    m_writer->writeEndElement(); // link
}

QT_END_NAMESPACE
