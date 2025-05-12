// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "helpprojectwriter.h"

#include "access.h"
#include "aggregate.h"
#include "atom.h"
#include "classnode.h"
#include "collectionnode.h"
#include "config.h"
#include "enumnode.h"
#include "functionnode.h"
#include "genustypes.h"
#include "htmlgenerator.h"
#include "node.h"
#include "qdocdatabase.h"
#include "typedefnode.h"

#include <QtCore/qhash.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

HelpProjectWriter::HelpProjectWriter(const QString &defaultFileName, Generator *g)
{
    reset(defaultFileName, g);
}

void HelpProjectWriter::reset(const QString &defaultFileName, Generator *g)
{
    m_projects.clear();
    m_gen = g;
    /*
      Get the pointer to the singleton for the qdoc database and
      store it locally. This replaces all the local accesses to
      the node tree, which are now private.
     */
    m_qdb = QDocDatabase::qdocDB();

    // The output directory should already have been checked by the calling
    // generator.
    Config &config = Config::instance();
    m_outputDir = config.getOutputDir();

    const QStringList names{config.get(CONFIG_QHP + Config::dot + "projects").asStringList()};

    for (const auto &projectName : names) {
        HelpProject project;
        project.m_name = projectName;

        QString prefix = CONFIG_QHP + Config::dot + projectName + Config::dot;
        project.m_helpNamespace = config.get(prefix + "namespace").asString();
        project.m_virtualFolder = config.get(prefix + "virtualFolder").asString();
        project.m_version = config.get(CONFIG_VERSION).asString();
        project.m_fileName = config.get(prefix + "file").asString();
        if (project.m_fileName.isEmpty())
            project.m_fileName = defaultFileName;
        project.m_extraFiles = config.get(prefix + "extraFiles").asStringSet();
        project.m_extraFiles += config.get(CONFIG_QHP + Config::dot + "extraFiles").asStringSet();
        project.m_indexTitle = config.get(prefix + "indexTitle").asString();
        project.m_indexRoot = config.get(prefix + "indexRoot").asString();
        project.m_filterAttributes = config.get(prefix + "filterAttributes").asStringSet();
        project.m_includeIndexNodes = config.get(prefix + "includeIndexNodes").asBool();
        const QSet<QString> customFilterNames = config.subVars(prefix + "customFilters");
        for (const auto &filterName : customFilterNames) {
            QString name{config.get(prefix + "customFilters" + Config::dot + filterName
                                    + Config::dot + "name").asString()};
            project.m_customFilters[name] =
                    config.get(prefix + "customFilters" + Config::dot + filterName
                               + Config::dot + "filterAttributes").asStringSet();
        }

        const auto excludedPrefixes = config.get(prefix + "excluded").asStringSet();
        for (auto name : excludedPrefixes)
            project.m_excluded.insert(name.replace(QLatin1Char('\\'), QLatin1Char('/')));

        const auto subprojectPrefixes{config.get(prefix + "subprojects").asStringList()};
        for (const auto &name : subprojectPrefixes) {
            SubProject subproject;
            QString subprefix = prefix + "subprojects" + Config::dot + name + Config::dot;
            subproject.m_title = config.get(subprefix + "title").asString();
            if (subproject.m_title.isEmpty())
                continue;
            subproject.m_indexTitle = config.get(subprefix + "indexTitle").asString();
            subproject.m_sortPages = config.get(subprefix + "sortPages").asBool();
            subproject.m_type = config.get(subprefix + "type").asString();
            readSelectors(subproject, config.get(subprefix + "selectors").asStringList());
            subprefix.chop(1);
            subproject.m_prefix = std::move(subprefix); // Stored for error reporting purposes
            project.m_subprojects.append(subproject);
        }

        if (project.m_subprojects.isEmpty()) {
            SubProject subproject;
            readSelectors(subproject, config.get(prefix + "selectors").asStringList());
            project.m_subprojects.insert(0, subproject);
        }

        m_projects.append(project);
    }
}

void HelpProjectWriter::readSelectors(SubProject &subproject, const QStringList &selectors)
{
    QHash<QString, NodeType> typeHash;
    typeHash["namespace"] = NodeType::Namespace;
    typeHash["class"] = NodeType::Class;
    typeHash["struct"] = NodeType::Struct;
    typeHash["union"] = NodeType::Union;
    typeHash["header"] = NodeType::HeaderFile;
    typeHash["headerfile"] = NodeType::HeaderFile;
    typeHash["doc"] = NodeType::Page; // Unused (supported but ignored as a prefix)
    typeHash["fake"] = NodeType::Page; // Unused (supported but ignored as a prefix)
    typeHash["page"] = NodeType::Page;
    typeHash["enum"] = NodeType::Enum;
    typeHash["example"] = NodeType::Example;
    typeHash["externalpage"] = NodeType::ExternalPage;
    typeHash["typedef"] = NodeType::Typedef;
    typeHash["typealias"] = NodeType::TypeAlias;
    typeHash["function"] = NodeType::Function;
    typeHash["property"] = NodeType::Property;
    typeHash["variable"] = NodeType::Variable;
    typeHash["group"] = NodeType::Group;
    typeHash["module"] = NodeType::Module;
    typeHash["none"] = NodeType::NoType;
    typeHash["qmlmodule"] = NodeType::QmlModule;
    typeHash["qmlproperty"] = NodeType::QmlProperty;
    typeHash["qmlclass"] = NodeType::QmlType; // Legacy alias for 'qmltype'
    typeHash["qmltype"] = NodeType::QmlType;
    typeHash["qmlbasictype"] = NodeType::QmlValueType; // Legacy alias for 'qmlvaluetype'
    typeHash["qmlvaluetype"] = NodeType::QmlValueType;

    for (const QString &selector : selectors) {
        QStringList pieces = selector.split(QLatin1Char(':'));
        // Remove doc: or fake: prefix
        if (pieces.size() > 1 && typeHash.value(pieces[0].toLower()) == NodeType::Page)
            pieces.takeFirst();

        QString typeName = pieces.takeFirst().toLower();
        if (!typeHash.contains(typeName))
            continue;

        subproject.m_selectors << static_cast<unsigned char>(typeHash.value(typeName));
        if (!pieces.isEmpty()) {
            pieces = pieces[0].split(QLatin1Char(','));
            for (const auto &piece : std::as_const(pieces)) {
                if (typeHash[typeName] == NodeType::Group
                    || typeHash[typeName] == NodeType::Module
                    || typeHash[typeName] == NodeType::QmlModule) {
                    subproject.m_groups << piece.toLower();
                }
            }
        }
    }
}

void HelpProjectWriter::addExtraFile(const QString &file)
{
    for (HelpProject &project : m_projects)
        project.m_extraFiles.insert(file);
}

Keyword HelpProjectWriter::keywordDetails(const Node *node) const
{
    QString ref = m_gen->fullDocumentLocation(node);

    if (node->parent() && !node->parent()->name().isEmpty()) {
        QString name = (node->isEnumType() || node->isTypedef())
                ? node->parent()->name()+"::"+node->name()
                : node->name();
        QString id = (!node->isRelatedNonmember())
                ? node->parent()->name()+"::"+node->name()
                : node->name();
        return Keyword(name, id, ref);
    } else if (node->isQmlType()) {
        const QString &name = node->name();
        QString moduleName = node->logicalModuleName();
        QStringList ids("QML." + name);
        if (!moduleName.isEmpty()) {
            QString majorVersion = node->logicalModule()
                    ? node->logicalModule()->logicalModuleVersion().split('.')[0]
                    : QString();
            ids << "QML." + moduleName + majorVersion + "." + name;
        }
        return Keyword(name, ids, ref);
    } else if (node->isQmlModule()) {
        const QLatin1Char delim('.');
        QStringList parts = node->logicalModuleName().split(delim) << "QML";
        std::reverse(parts.begin(), parts.end());
        return Keyword(node->logicalModuleName(), parts.join(delim), ref);
    } else if (node->isTextPageNode()) {
        const auto *pageNode = static_cast<const PageNode *>(node);
        return Keyword(pageNode->fullTitle(), pageNode->fullTitle(), ref);
    } else {
        return Keyword(node->name(), node->name(), ref);
    }
}

bool HelpProjectWriter::generateSection(HelpProject &project, QXmlStreamWriter & /* writer */,
                                        const Node *node)
{
    if (!node->url().isEmpty() && !(project.m_includeIndexNodes && !node->url().startsWith("http")))
        return false;

    // Process (members of) unseen group nodes, i.e. nodes that use \ingroup <group_name> where
    // \group group_name itself is not documented.
    bool unseenGroup{node->isGroup() && !node->wasSeen()};

    if ((node->isPrivate() || node->isInternal() || node->isDontDocument()) && !unseenGroup)
        return false;

    if (node->name().isEmpty())
        return true;

    QString docPath = node->doc().location().filePath();
    if (!docPath.isEmpty() && project.m_excluded.contains(docPath))
        return false;

    QString objName = node->isTextPageNode() ? node->fullTitle() : node->fullDocumentName();
    // Only add nodes to the set for each subproject if they match a selector.
    // Those that match will be listed in the table of contents.

    for (auto &subproject : project.m_subprojects) {
        // No selectors: accept all nodes.
        if (subproject.m_selectors.isEmpty()) {
            subproject.m_nodes.insert(objName, node);
        } else if (subproject.m_selectors.contains(static_cast<unsigned char>(node->nodeType()))) {
            // Add all group members for '[group|module|qmlmodule]:name' selector
            if (node->isCollectionNode()) {
                if (subproject.m_groups.contains(node->name().toLower())) {
                    const auto *cn = static_cast<const CollectionNode *>(node);
                    const auto members = cn->members();
                    for (const Node *m : members) {
                        if (!m->isInAPI())
                            continue;
                        QString memberName =
                                m->isTextPageNode() ? m->fullTitle() : m->fullDocumentName();
                        subproject.m_nodes.insert(memberName, m);
                    }
                    continue;
                } else if (!subproject.m_groups.isEmpty()) {
                    continue; // Node does not represent specified group(s)
                }
            } else if (node->isTextPageNode()) {
                if (node->isExternalPage() || node->fullTitle().isEmpty())
                    continue;
            }
            subproject.m_nodes.insert(objName, node);
        }
    }

    auto appendDocKeywords = [&](const Node *n) {
        for (const auto *kw : n->doc().keywords()) {
                if (!kw->string().isEmpty()) {
                    QStringList ref_parts = m_gen->fullDocumentLocation(n).split('#'_L1);
                    // Use keyword's custom anchor if it has one
                    if (kw->count() > 1) {
                        if (ref_parts.count() > 1)
                            ref_parts.pop_back();
                        ref_parts << kw->string(1);
                    }
                    project.m_keywords.append(Keyword(kw->string(), kw->string(),
                            ref_parts.join('#'_L1)));
                }
            }
    };
    // Unseen group nodes require no further processing as they have no documentation
    if (unseenGroup)
        return false;

    switch (node->nodeType()) {

    case NodeType::Class:
    case NodeType::Struct:
    case NodeType::Union:
        project.m_keywords.append(keywordDetails(node));
        break;
    case NodeType::QmlType:
    case NodeType::QmlValueType:
        appendDocKeywords(node);
        project.m_keywords.append(keywordDetails(node));
        break;

    case NodeType::Namespace:
        project.m_keywords.append(keywordDetails(node));
        break;

    case NodeType::Enum:
    case NodeType::QmlEnum:
        project.m_keywords.append(keywordDetails(node));
        {
            const auto *enumNode = static_cast<const EnumNode *>(node);
            const auto items = enumNode->items();
            for (const auto &item : items) {
                if (enumNode->itemAccess(item.name()) == Access::Private)
                    continue;

                QString name;
                QString id;
                if (!node->parent()->name().isEmpty()) {
                    name = id = node->parent()->name() + "::" + item.name();
                } else {
                    name = id = item.name();
                }
                QString ref = m_gen->fullDocumentLocation(node);
                project.m_keywords.append(Keyword(std::move(name), std::move(id), std::move(ref)));
            }
        }
        break;

    case NodeType::Group:
    case NodeType::Module:
    case NodeType::QmlModule: {
        if (!node->fullTitle().isEmpty()) {
            appendDocKeywords(node);
            project.m_keywords.append(keywordDetails(node));
        }
    } break;

    case NodeType::Property:
    case NodeType::QmlProperty:
        project.m_keywords.append(keywordDetails(node));
        break;

    case NodeType::Function: {
        const auto *funcNode = static_cast<const FunctionNode *>(node);

        /*
          QML methods, signals, and signal handlers used to be node types,
          but now they are Function nodes with a Metaness value that specifies
          what kind of function they are, QmlSignal, QmlMethod, etc.
        */
        if (funcNode->isQmlNode()) {
            project.m_keywords.append(keywordDetails(node));
            break;
        }
        // Only insert keywords for non-constructors. Constructors are covered
        // by the classes themselves.

        if (!funcNode->isSomeCtor())
            project.m_keywords.append(keywordDetails(node));

        // Insert member status flags into the entries for the parent
        // node of the function, or the node it is related to.
        // Since parent nodes should have already been inserted into
        // the set of files, we only need to ensure that related nodes
        // are inserted.

        if (node->parent())
            project.m_memberStatus[node->parent()].insert(node->status());
    } break;
    case NodeType::TypeAlias:
    case NodeType::Typedef: {
        const auto *typedefNode = static_cast<const TypedefNode *>(node);
        Keyword typedefDetails = keywordDetails(node);
        const EnumNode *enumNode = typedefNode->associatedEnum();
        // Use the location of any associated enum node in preference
        // to that of the typedef.
        if (enumNode)
            typedefDetails.m_ref = m_gen->fullDocumentLocation(enumNode);

        project.m_keywords.append(typedefDetails);
    } break;

    case NodeType::Variable: {
        project.m_keywords.append(keywordDetails(node));
    } break;

        // Page nodes (such as manual pages) contain subtypes, titles and other
        // attributes.
    case NodeType::Page: {
        if (!node->fullTitle().isEmpty()) {
            appendDocKeywords(node);
            project.m_keywords.append(keywordDetails(node));
        }
        break;
    }
    default:;
    }

    // Add all images referenced in the page to the set of files to include.
    const Atom *atom = node->doc().body().firstAtom();
    while (atom) {
        if (atom->type() == Atom::Image || atom->type() == Atom::InlineImage) {
            // Images are all placed within a single directory regardless of
            // whether the source images are in a nested directory structure.
            QStringList pieces = atom->string().split(QLatin1Char('/'));
            project.m_files.insert("images/" + pieces.last());
        }
        atom = atom->next();
    }

    return true;
}

void HelpProjectWriter::generateSections(HelpProject &project, QXmlStreamWriter &writer,
                                         const Node *node)
{
    /*
      Don't include index nodes in the help file.
     */
    if (node->isIndexNode())
        return;
    if (!generateSection(project, writer, node))
        return;

    if (node->isAggregate()) {
        const auto *aggregate = static_cast<const Aggregate *>(node);

        // Ensure that we don't visit nodes more than once.
        NodeList childSet;
        NodeList children = aggregate->childNodes();
        std::sort(children.begin(), children.end(), Node::nodeNameLessThan);
        for (auto *child : children) {
            // Skip related non-members adopted by some other aggregate
            if (child->parent() != aggregate)
                continue;
            // Process unseen group nodes (even though they're marked internal)
            if (child->isGroup() && !child->wasSeen()) {
                childSet << child;
                continue;
            }
            if (child->isIndexNode() || child->isPrivate())
                continue;
            if (child->isTextPageNode()) {
                if (!childSet.contains(child))
                    childSet << child;
            } else {
                // Store member status of children
                project.m_memberStatus[node].insert(child->status());
                if (child->isFunction() && static_cast<const FunctionNode *>(child)->isOverload())
                    continue;
                if (!childSet.contains(child))
                    childSet << child;
            }
        }
        for (const auto *child : std::as_const(childSet))
            generateSections(project, writer, child);
    }
}

void HelpProjectWriter::generate()
{
    // Warn if .qhp configuration was expected but not provided
    if (auto &config = Config::instance(); m_projects.isEmpty() && config.get(CONFIG_QHP).asBool()) {
        config.location().warning(u"Documentation configuration for '%1' doesn't define a help project (qhp)"_s
                .arg(config.get(CONFIG_PROJECT).asString()));
    }
    for (HelpProject &project : m_projects)
        generateProject(project);
}

void HelpProjectWriter::writeSection(QXmlStreamWriter &writer, const QString &path,
                                     const QString &value)
{
    writer.writeStartElement(QStringLiteral("section"));
    writer.writeAttribute(QStringLiteral("ref"), path);
    writer.writeAttribute(QStringLiteral("title"), value);
    writer.writeEndElement(); // section
}

/*!
    Write subsections for all members, compatibility members and obsolete members.
 */
void HelpProjectWriter::addMembers(HelpProject &project, QXmlStreamWriter &writer, const Node *node)
{
    QString href = m_gen->fullDocumentLocation(node);
    href = href.left(href.size() - 5);
    if (href.isEmpty())
        return;

    bool derivedClass = false;
    if (node->isClassNode())
        derivedClass = !(static_cast<const ClassNode *>(node)->baseClasses().isEmpty());

    // Do not generate a 'List of all members' for namespaces or header files,
    // but always generate it for derived classes and QML types (but not QML value types)
    if (!node->isNamespace() && !node->isHeader() && !node->isQmlBasicType() && !node->isWrapper()
        && (derivedClass || node->isQmlType() || !project.m_memberStatus[node].isEmpty())) {
        QString membersPath = href + QStringLiteral("-members.html");
        writeSection(writer, membersPath, QStringLiteral("List of all members"));
    }
    if (project.m_memberStatus[node].contains(Node::Deprecated)) {
        QString obsoletePath = href + QStringLiteral("-obsolete.html");
        writeSection(writer, obsoletePath, QStringLiteral("Obsolete members"));
    }
}

void HelpProjectWriter::writeNode(HelpProject &project, QXmlStreamWriter &writer, const Node *node)
{
    QString href = m_gen->fullDocumentLocation(node);
    QString objName = node->name();

    switch (node->nodeType()) {

    case NodeType::Class:
    case NodeType::Struct:
    case NodeType::Union:
    case NodeType::QmlType:
    case NodeType::QmlValueType: {
        QString typeStr = m_gen->typeString(node);
        if (!typeStr.isEmpty())
            typeStr[0] = typeStr[0].toTitleCase();
        writer.writeStartElement("section");
        writer.writeAttribute("ref", href);
        if (node->parent() && !node->parent()->name().isEmpty())
            writer.writeAttribute("title",
                                  QStringLiteral("%1::%2 %3 Reference")
                                          .arg(node->parent()->name(), objName, typeStr));
        else
            writer.writeAttribute("title", QStringLiteral("%1 %2 Reference").arg(objName, typeStr));

        addMembers(project, writer, node);
        writer.writeEndElement(); // section
    } break;

    case NodeType::Namespace:
        writeSection(writer, href, "%1 Namespace Reference"_L1.arg(objName));
        break;

    case NodeType::Example:
    case NodeType::HeaderFile:
    case NodeType::Page:
    case NodeType::Group:
    case NodeType::Module:
    case NodeType::QmlModule: {
        writer.writeStartElement("section");
        writer.writeAttribute("ref", href);
        writer.writeAttribute("title", node->fullTitle());
        if (node->nodeType() == NodeType::HeaderFile)
            addMembers(project, writer, node);
        writer.writeEndElement(); // section
    } break;
    default:;
    }
}

void HelpProjectWriter::generateProject(HelpProject &project)
{
    const Node *rootNode;

    // Restrict searching only to the local (primary) tree
    QList<Tree *> searchOrder = m_qdb->searchOrder();
    m_qdb->setLocalSearch();

    if (!project.m_indexRoot.isEmpty())
        rootNode = m_qdb->findPageNodeByTitle(project.m_indexRoot);
    else
        rootNode = m_qdb->primaryTreeRoot();

    if (rootNode == nullptr)
        return;

    project.m_files.clear();
    project.m_keywords.clear();

    QFile file(m_outputDir + QDir::separator() + project.m_fileName);
    if (!file.open(QFile::WriteOnly))
        return;

    QXmlStreamWriter writer(&file);
    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    writer.writeStartElement("QtHelpProject");
    writer.writeAttribute("version", "1.0");

    // Write metaData, virtualFolder and namespace elements.
    writer.writeTextElement("namespace", project.m_helpNamespace);
    writer.writeTextElement("virtualFolder", project.m_virtualFolder);
    writer.writeStartElement("metaData");
    writer.writeAttribute("name", "version");
    writer.writeAttribute("value", project.m_version);
    writer.writeEndElement();

    // Write customFilter elements.
    for (auto it = project.m_customFilters.constBegin(); it != project.m_customFilters.constEnd();
         ++it) {
        writer.writeStartElement("customFilter");
        writer.writeAttribute("name", it.key());
        QStringList sortedAttributes = it.value().values();
        sortedAttributes.sort();
        for (const auto &filter : std::as_const(sortedAttributes))
            writer.writeTextElement("filterAttribute", filter);
        writer.writeEndElement(); // customFilter
    }

    // Start the filterSection.
    writer.writeStartElement("filterSection");

    // Write filterAttribute elements.
    QStringList sortedFilterAttributes = project.m_filterAttributes.values();
    sortedFilterAttributes.sort();
    for (const auto &filterName : std::as_const(sortedFilterAttributes))
        writer.writeTextElement("filterAttribute", filterName);

    writer.writeStartElement("toc");
    writer.writeStartElement("section");
    const Node *node = m_qdb->findPageNodeByTitle(project.m_indexTitle);
    if (!node)
        node = m_qdb->findNodeByNameAndType(QStringList(project.m_indexTitle), &Node::isPageNode);
    if (!node)
        node = m_qdb->findNodeByNameAndType(QStringList("index.html"), &Node::isPageNode);
    QString indexPath;
    if (node)
        indexPath = m_gen->fullDocumentLocation(node);
    else
        indexPath = "index.html";
    writer.writeAttribute("ref", indexPath);
    writer.writeAttribute("title", project.m_indexTitle);

    generateSections(project, writer, rootNode);

    for (int i = 0; i < project.m_subprojects.size(); i++) {
        SubProject subproject = project.m_subprojects[i];

        if (subproject.m_type == QLatin1String("manual")) {

            const Node *indexPage = m_qdb->findNodeForTarget(subproject.m_indexTitle, nullptr);
            if (indexPage) {
                Text indexBody = indexPage->doc().body();
                const Atom *atom = indexBody.firstAtom();
                QStack<int> sectionStack;
                bool inItem = false;

                while (atom) {
                    switch (atom->type()) {
                    case Atom::ListLeft:
                        sectionStack.push(0);
                        break;
                    case Atom::ListRight:
                        if (sectionStack.pop() > 0)
                            writer.writeEndElement(); // section
                        break;
                    case Atom::ListItemLeft:
                        inItem = true;
                        break;
                    case Atom::ListItemRight:
                        inItem = false;
                        break;
                    case Atom::Link:
                        if (inItem) {
                            if (sectionStack.top() > 0)
                                writer.writeEndElement(); // section

                            const Node *page = m_qdb->findNodeForTarget(atom->string(), nullptr);
                            writer.writeStartElement("section");
                            QString indexPath = m_gen->fullDocumentLocation(page);
                            writer.writeAttribute("ref", indexPath);
                            writer.writeAttribute("title", atom->linkText());

                            sectionStack.top() += 1;
                        }
                        break;
                    default:;
                    }

                    if (atom == indexBody.lastAtom())
                        break;
                    atom = atom->next();
                }
            } else
                Config::instance().location().warning(
                        "Failed to find %1.indexTitle '%2'"_L1.arg(subproject.m_prefix, subproject.m_indexTitle));

        } else {

            writer.writeStartElement("section");
            QString indexPath = m_gen->fullDocumentLocation(
                    m_qdb->findNodeForTarget(subproject.m_indexTitle, nullptr));
            if (indexPath.isEmpty() && !subproject.m_indexTitle.isEmpty())
                Config::instance().location().warning(
                        "Failed to find %1.indexTitle '%2'"_L1.arg(subproject.m_prefix, subproject.m_indexTitle));
            writer.writeAttribute("ref", indexPath);
            writer.writeAttribute("title", subproject.m_title);

            if (subproject.m_sortPages) {
                QStringList titles = subproject.m_nodes.keys();
                titles.sort();
                for (const auto &title : std::as_const(titles)) {
                    writeNode(project, writer, subproject.m_nodes[title]);
                }
            } else {
                // Find a contents node and navigate from there, using the NextLink values.
                QSet<QString> visited;
                bool contentsFound = false;
                for (const auto *node : std::as_const(subproject.m_nodes)) {
                    QString nextTitle = node->links().value(Node::NextLink).first;
                    if (!nextTitle.isEmpty()
                        && node->links().value(Node::ContentsLink).first.isEmpty()) {

                        const Node *nextPage = m_qdb->findNodeForTarget(nextTitle, nullptr);

                        // Write the contents node.
                        writeNode(project, writer, node);
                        contentsFound = true;

                        while (nextPage) {
                            writeNode(project, writer, nextPage);
                            nextTitle = nextPage->links().value(Node::NextLink).first;
                            if (nextTitle.isEmpty() || visited.contains(nextTitle))
                                break;
                            nextPage = m_qdb->findNodeForTarget(nextTitle, nullptr);
                            visited.insert(nextTitle);
                        }
                        break;
                    }
                }
                // No contents/nextpage links found, write all nodes unsorted
                if (!contentsFound) {
                    QList<const Node *> subnodes = subproject.m_nodes.values();

                    std::sort(subnodes.begin(), subnodes.end(), Node::nodeNameLessThan);

                    for (const auto *node : std::as_const(subnodes))
                        writeNode(project, writer, node);
                }
            }

            writer.writeEndElement(); // section
        }
    }

    // Restore original search order
    m_qdb->setSearchOrder(searchOrder);

    writer.writeEndElement(); // section
    writer.writeEndElement(); // toc

    writer.writeStartElement("keywords");
    std::sort(project.m_keywords.begin(), project.m_keywords.end());
    for (const auto &k : std::as_const(project.m_keywords)) {
        for (const auto &id : std::as_const(k.m_ids)) {
            writer.writeStartElement("keyword");
            writer.writeAttribute("name", k.m_name);
            writer.writeAttribute("id", id);
            writer.writeAttribute("ref", k.m_ref);
            writer.writeEndElement(); //keyword
        }
    }
    writer.writeEndElement(); // keywords

    writer.writeStartElement("files");

    // The list of files to write is the union of generated files and
    // other files (images and extras) included in the project
    QSet<QString> files =
            QSet<QString>(m_gen->outputFileNames().cbegin(), m_gen->outputFileNames().cend());
    files.unite(project.m_files);
    files.unite(project.m_extraFiles);
    QStringList sortedFiles = files.values();
    sortedFiles.sort();
    for (const auto &usedFile : std::as_const(sortedFiles)) {
        if (!usedFile.isEmpty())
            writer.writeTextElement("file", usedFile);
    }
    writer.writeEndElement(); // files

    writer.writeEndElement(); // filterSection
    writer.writeEndElement(); // QtHelpProject
    writer.writeEndDocument();
    file.close();
}

QT_END_NAMESPACE
