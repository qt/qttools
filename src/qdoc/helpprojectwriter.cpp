/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "helpprojectwriter.h"

#include "atom.h"
#include "config.h"
#include "htmlgenerator.h"
#include "node.h"
#include "qdocdatabase.h"

#include <QtCore/qcryptographichash.h>
#include <QtCore/qdebug.h>
#include <QtCore/qhash.h>
#include <QtCore/qmap.h>

QT_BEGIN_NAMESPACE

HelpProjectWriter::HelpProjectWriter(const QString &defaultFileName, Generator *g)
{
    reset(defaultFileName, g);
}

void HelpProjectWriter::reset(const QString &defaultFileName, Generator *g)
{
    projects.clear();
    gen_ = g;
    /*
      Get the pointer to the singleton for the qdoc database and
      store it locally. This replaces all the local accesses to
      the node tree, which are now private.
     */
    qdb_ = QDocDatabase::qdocDB();

    // The output directory should already have been checked by the calling
    // generator.
    Config &config = Config::instance();
    outputDir = config.getOutputDir();

    const QStringList names = config.getStringList(CONFIG_QHP + Config::dot + "projects");

    for (const auto &projectName : names) {
        HelpProject project;
        project.name = projectName;

        QString prefix = CONFIG_QHP + Config::dot + projectName + Config::dot;
        project.helpNamespace = config.getString(prefix + "namespace");
        project.virtualFolder = config.getString(prefix + "virtualFolder");
        project.version = config.getString(CONFIG_VERSION);
        project.fileName = config.getString(prefix + "file");
        if (project.fileName.isEmpty())
            project.fileName = defaultFileName;
        project.extraFiles = config.getStringSet(prefix + "extraFiles");
        project.extraFiles += config.getStringSet(CONFIG_QHP + Config::dot + "extraFiles");
        project.indexTitle = config.getString(prefix + "indexTitle");
        project.indexRoot = config.getString(prefix + "indexRoot");
        const auto &filterAttributes = config.getStringList(prefix + "filterAttributes");
        project.filterAttributes =
                QSet<QString>(filterAttributes.cbegin(), filterAttributes.cend());
        project.includeIndexNodes = config.getBool(prefix + "includeIndexNodes");
        const QSet<QString> customFilterNames = config.subVars(prefix + "customFilters");
        for (const auto &filterName : customFilterNames) {
            QString name = config.getString(prefix + "customFilters" + Config::dot + filterName
                                            + Config::dot + "name");
            const auto &filters =
                    config.getStringList(prefix + "customFilters" + Config::dot + filterName
                                         + Config::dot + "filterAttributes");
            project.customFilters[name] = QSet<QString>(filters.cbegin(), filters.cend());
        }

        const auto excludedPrefixes = config.getStringSet(prefix + "excluded");
        for (auto name : excludedPrefixes)
            project.excluded.insert(name.replace(QLatin1Char('\\'), QLatin1Char('/')));

        const auto subprojectPrefixes = config.getStringList(prefix + "subprojects");
        for (const auto &name : subprojectPrefixes) {
            SubProject subproject;
            QString subprefix = prefix + "subprojects" + Config::dot + name + Config::dot;
            subproject.title = config.getString(subprefix + "title");
            if (subproject.title.isEmpty())
                continue;
            subproject.indexTitle = config.getString(subprefix + "indexTitle");
            subproject.sortPages = config.getBool(subprefix + "sortPages");
            subproject.type = config.getString(subprefix + "type");
            readSelectors(subproject, config.getStringList(subprefix + "selectors"));
            project.subprojects.append(subproject);
        }

        if (project.subprojects.isEmpty()) {
            SubProject subproject;
            readSelectors(subproject, config.getStringList(prefix + "selectors"));
            project.subprojects.insert(0, subproject);
        }

        projects.append(project);
    }
}

void HelpProjectWriter::readSelectors(SubProject &subproject, const QStringList &selectors)
{
    QHash<QString, Node::NodeType> typeHash;
    typeHash["namespace"] = Node::Namespace;
    typeHash["class"] = Node::Class;
    typeHash["struct"] = Node::Struct;
    typeHash["union"] = Node::Union;
    typeHash["header"] = Node::HeaderFile;
    typeHash["headerfile"] = Node::HeaderFile;
    typeHash["doc"] = Node::Page; // Unused (supported but ignored as a prefix)
    typeHash["fake"] = Node::Page; // Unused (supported but ignored as a prefix)
    typeHash["page"] = Node::Page;
    typeHash["enum"] = Node::Enum;
    typeHash["example"] = Node::Example;
    typeHash["externalpage"] = Node::ExternalPage;
    typeHash["typedef"] = Node::Typedef;
    typeHash["typealias"] = Node::TypeAlias;
    typeHash["function"] = Node::Function;
    typeHash["property"] = Node::Property;
    typeHash["variable"] = Node::Variable;
    typeHash["group"] = Node::Group;
    typeHash["module"] = Node::Module;
    typeHash["jsmodule"] = Node::JsModule;
    typeHash["qmlmodule"] = Node::QmlModule;
    typeHash["qmlproperty"] = Node::JsProperty;
    typeHash["jsproperty"] = Node::QmlProperty;
    typeHash["qmlclass"] = Node::QmlType; // Legacy alias for 'qmltype'
    typeHash["qmltype"] = Node::QmlType;
    typeHash["qmlbasictype"] = Node::QmlBasicType;

    for (const QString &selector : selectors) {
        QStringList pieces = selector.split(QLatin1Char(':'));
        // Remove doc: or fake: prefix
        if (pieces.size() > 1 && typeHash.value(pieces[0].toLower()) == Node::Page)
            pieces.takeFirst();

        QString typeName = pieces.takeFirst().toLower();
        if (!typeHash.contains(typeName))
            continue;

        subproject.selectors << typeHash.value(typeName);
        if (!pieces.isEmpty()) {
            pieces = pieces[0].split(QLatin1Char(','));
            for (const auto &piece : qAsConst(pieces)) {
                if (typeHash[typeName] == Node::Group
                    || typeHash[typeName] == Node::Module
                    || typeHash[typeName] == Node::QmlModule
                    || typeHash[typeName] == Node::JsModule) {
                    subproject.groups << piece.toLower();
                }
            }
        }
    }
}

void HelpProjectWriter::addExtraFile(const QString &file)
{
    for (int i = 0; i < projects.size(); ++i)
        projects[i].extraFiles.insert(file);
}

void HelpProjectWriter::addExtraFiles(const QSet<QString> &files)
{
    for (int i = 0; i < projects.size(); ++i)
        projects[i].extraFiles.unite(files);
}

/*!
    Returns a list of strings describing the keyword details for a given node.

    The first string is the human-readable name to be shown in Assistant.
    The second string is a unique identifier.
    The third string is the location of the documentation for the keyword.
 */
QStringList HelpProjectWriter::keywordDetails(const Node *node) const
{
    QStringList details;

    if (node->parent() && !node->parent()->name().isEmpty()) {
        // "name"
        if (node->isEnumType() || node->isTypedef())
            details << node->parent()->name() + "::" + node->name();
        else
            details << node->name();
        // "id"
        if (!node->isRelatedNonmember())
            details << node->parent()->name() + "::" + node->name();
        else
            details << node->name();
    } else if (node->isQmlType() || node->isQmlBasicType()) {
        details << node->name();
        details << "QML." + node->name();
    } else if (node->isJsType() || node->isJsBasicType()) {
        details << node->name();
        details << "JS." + node->name();
    } else if (node->isTextPageNode()) {
        const PageNode *fake = static_cast<const PageNode *>(node);
        details << fake->fullTitle();
        details << fake->fullTitle();
    } else {
        details << node->name();
        details << node->name();
    }
    details << gen_->fullDocumentLocation(node, false);
    return details;
}

bool HelpProjectWriter::generateSection(HelpProject &project, QXmlStreamWriter & /* writer */,
                                        const Node *node)
{
    if (!node->url().isEmpty() && !(project.includeIndexNodes && !node->url().startsWith("http")))
        return false;

    if (node->isPrivate() || node->isInternal())
        return false;

    if (node->name().isEmpty())
        return true;

    QString docPath = node->doc().location().filePath();
    if (!docPath.isEmpty() && project.excluded.contains(docPath))
        return false;

    QString objName = node->isTextPageNode() ? node->fullTitle() : node->fullDocumentName();
    // Only add nodes to the set for each subproject if they match a selector.
    // Those that match will be listed in the table of contents.

    for (int i = 0; i < project.subprojects.length(); i++) {
        SubProject subproject = project.subprojects[i];
        // No selectors: accept all nodes.
        if (subproject.selectors.isEmpty()) {
            project.subprojects[i].nodes[objName] = node;
        } else if (subproject.selectors.contains(node->nodeType())) {
            // Add all group members for '[group|module|qmlmodule]:name' selector
            if (node->isCollectionNode()) {
                if (project.subprojects[i].groups.contains(node->name().toLower())) {
                    const CollectionNode *cn = static_cast<const CollectionNode *>(node);
                    const auto members = cn->members();
                    for (const Node *m : members) {
                        QString memberName =
                                m->isTextPageNode() ? m->fullTitle() : m->fullDocumentName();
                        project.subprojects[i].nodes[memberName] = m;
                    }
                    continue;
                } else if (!project.subprojects[i].groups.isEmpty()) {
                    continue; // Node does not represent specified group(s)
                }
            } else if (node->isTextPageNode()) {
                if (node->isExternalPage() || node->fullTitle().isEmpty())
                    continue;
            }
            project.subprojects[i].nodes[objName] = node;
        }
    }

    switch (node->nodeType()) {

    case Node::Class:
    case Node::Struct:
    case Node::Union:
        project.keywords.append(keywordDetails(node));
        break;
    case Node::QmlType:
    case Node::QmlBasicType:
    case Node::JsType:
    case Node::JsBasicType:
        if (node->doc().hasKeywords()) {
            const auto keywords = node->doc().keywords();
            for (const Atom *keyword : keywords) {
                if (!keyword->string().isEmpty()) {
                    QStringList details;
                    details << keyword->string() << keyword->string()
                            << gen_->fullDocumentLocation(node, false);
                    project.keywords.append(details);
                } else
                    node->doc().location().warning(
                            tr("Bad keyword in %1").arg(gen_->fullDocumentLocation(node, false)));
            }
        }
        project.keywords.append(keywordDetails(node));
        break;

    case Node::Namespace:
        project.keywords.append(keywordDetails(node));
        break;

    case Node::Enum:
        project.keywords.append(keywordDetails(node));
        {
            const EnumNode *enumNode = static_cast<const EnumNode *>(node);
            const auto items = enumNode->items();
            for (const auto &item : items) {
                QStringList details;

                if (enumNode->itemAccess(item.name()) == Node::Private)
                    continue;

                if (!node->parent()->name().isEmpty()) {
                    details << node->parent()->name() + "::" + item.name(); // "name"
                    details << node->parent()->name() + "::" + item.name(); // "id"
                } else {
                    details << item.name(); // "name"
                    details << item.name(); // "id"
                }
                details << gen_->fullDocumentLocation(node, false);
                project.keywords.append(details);
            }
        }
        break;

    case Node::Group:
    case Node::Module:
    case Node::QmlModule:
    case Node::JsModule: {
        const CollectionNode *cn = static_cast<const CollectionNode *>(node);
        if (!cn->fullTitle().isEmpty()) {
            if (cn->doc().hasKeywords()) {
                const auto keywords = cn->doc().keywords();
                for (const Atom *keyword : keywords) {
                    if (!keyword->string().isEmpty()) {
                        QStringList details;
                        details << keyword->string() << keyword->string()
                                << gen_->fullDocumentLocation(node, false);
                        project.keywords.append(details);
                    } else
                        cn->doc().location().warning(
                                tr("Bad keyword in %1")
                                        .arg(gen_->fullDocumentLocation(node, false)));
                }
            }
            project.keywords.append(keywordDetails(node));
        }
    } break;

    case Node::Property:
    case Node::QmlProperty:
    case Node::JsProperty:
        project.keywords.append(keywordDetails(node));
        break;

    case Node::Function: {
        const FunctionNode *funcNode = static_cast<const FunctionNode *>(node);

        /*
          QML and JS methods, signals, and signal handlers used to be node types,
          but now they are Function nodes with a Metaness value that specifies
          what kind of function they are, QmlSignal, JsSignal, QmlMethod, etc. It
          suffices at this point to test whether the node is of the QML or JS Genus,
          because we already know it is NodeType::Function.
         */
        if (funcNode->isQmlNode() || funcNode->isJsNode()) {
            project.keywords.append(keywordDetails(node));
            break;
        }
        // Only insert keywords for non-constructors. Constructors are covered
        // by the classes themselves.

        if (!funcNode->isSomeCtor())
            project.keywords.append(keywordDetails(node));

        // Insert member status flags into the entries for the parent
        // node of the function, or the node it is related to.
        // Since parent nodes should have already been inserted into
        // the set of files, we only need to ensure that related nodes
        // are inserted.

        if (node->parent())
            project.memberStatus[node->parent()].insert(node->status());
    } break;
    case Node::TypeAlias:
    case Node::Typedef: {
        const TypedefNode *typedefNode = static_cast<const TypedefNode *>(node);
        QStringList typedefDetails = keywordDetails(node);
        const EnumNode *enumNode = typedefNode->associatedEnum();
        // Use the location of any associated enum node in preference
        // to that of the typedef.
        if (enumNode)
            typedefDetails[2] = gen_->fullDocumentLocation(enumNode, false);

        project.keywords.append(typedefDetails);
    } break;

    case Node::Variable: {
        project.keywords.append(keywordDetails(node));
    } break;

        // Page nodes (such as manual pages) contain subtypes, titles and other
        // attributes.
    case Node::Page: {
        const PageNode *pn = static_cast<const PageNode *>(node);
        if (!pn->fullTitle().isEmpty()) {
            if (pn->doc().hasKeywords()) {
                const auto keywords = pn->doc().keywords();
                for (const Atom *keyword : keywords) {
                    if (!keyword->string().isEmpty()) {
                        QStringList details;
                        details << keyword->string() << keyword->string()
                                << gen_->fullDocumentLocation(node, false);
                        project.keywords.append(details);
                    } else {
                        QString loc = gen_->fullDocumentLocation(node, false);
                        pn->doc().location().warning(tr("Bad keyword in %1").arg(loc));
                    }
                }
            }
            project.keywords.append(keywordDetails(node));
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
            project.files.insert("images/" + pieces.last());
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
        const Aggregate *aggregate = static_cast<const Aggregate *>(node);

        // Ensure that we don't visit nodes more than once.
        QSet<const Node *> childSet;
        const NodeList &children = aggregate->childNodes();
        for (const auto *child : children) {
            // Skip related non-members adopted by some other aggregate
            if (child->parent() != aggregate)
                continue;
            if (child->isIndexNode() || child->isPrivate())
                continue;
            if (child->isTextPageNode()) {
                childSet << child;
            } else {
                // Store member status of children
                project.memberStatus[node].insert(child->status());
                if (child->isFunction() && static_cast<const FunctionNode *>(child)->isOverload())
                    continue;
                childSet << child;
            }
        }
        for (const auto *child : qAsConst(childSet))
            generateSections(project, writer, child);
    }
}

void HelpProjectWriter::generate()
{
    for (int i = 0; i < projects.size(); ++i)
        generateProject(projects[i]);
}

void HelpProjectWriter::writeHashFile(QFile &file)
{
    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(&file);

    QFile hashFile(file.fileName() + ".sha1");
    if (!hashFile.open(QFile::WriteOnly | QFile::Text))
        return;

    hashFile.write(hash.result().toHex());
    hashFile.close();
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
    if (node->isQmlBasicType() || node->isJsBasicType())
        return;

    QString href = gen_->fullDocumentLocation(node, false);
    href = href.left(href.size() - 5);
    if (href.isEmpty())
        return;

    bool derivedClass = false;
    if (node->isClassNode())
        derivedClass = !(static_cast<const ClassNode *>(node)->baseClasses().isEmpty());

    // Do not generate a 'List of all members' for namespaces or header files,
    // but always generate it for derived classes and QML classes
    if (!node->isNamespace() && !node->isHeader()
        && (derivedClass || node->isQmlType() || node->isJsType()
            || !project.memberStatus[node].isEmpty())) {
        QString membersPath = href + QStringLiteral("-members.html");
        writeSection(writer, membersPath, tr("List of all members"));
    }
    if (project.memberStatus[node].contains(Node::Obsolete)) {
        QString obsoletePath = href + QStringLiteral("-obsolete.html");
        writeSection(writer, obsoletePath, tr("Obsolete members"));
    }
}

void HelpProjectWriter::writeNode(HelpProject &project, QXmlStreamWriter &writer, const Node *node)
{
    QString href = gen_->fullDocumentLocation(node, false);
    QString objName = node->name();

    switch (node->nodeType()) {

    case Node::Class:
    case Node::Struct:
    case Node::Union:
    case Node::QmlType:
    case Node::JsType:
    case Node::QmlBasicType:
    case Node::JsBasicType: {
        QString typeStr = gen_->typeString(node);
        if (!typeStr.isEmpty())
            typeStr[0] = typeStr[0].toTitleCase();
        writer.writeStartElement("section");
        writer.writeAttribute("ref", href);
        if (node->parent() && !node->parent()->name().isEmpty())
            writer.writeAttribute(
                    "title", tr("%1::%2 %3 Reference").arg(node->parent()->name()).arg(objName).arg(typeStr));
        else
            writer.writeAttribute("title", tr("%1 %2 Reference").arg(objName).arg(typeStr));

        addMembers(project, writer, node);
        writer.writeEndElement(); // section
    } break;

    case Node::Namespace:
        writeSection(writer, href, objName);
        break;

    case Node::Example:
    case Node::HeaderFile:
    case Node::Page:
    case Node::Group:
    case Node::Module:
    case Node::JsModule:
    case Node::QmlModule: {
        writer.writeStartElement("section");
        writer.writeAttribute("ref", href);
        writer.writeAttribute("title", node->fullTitle());
        if (node->nodeType() == Node::HeaderFile)
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
    QVector<Tree *> searchOrder = qdb_->searchOrder();
    qdb_->setLocalSearch();

    if (!project.indexRoot.isEmpty())
        rootNode = qdb_->findPageNodeByTitle(project.indexRoot);
    else
        rootNode = qdb_->primaryTreeRoot();

    if (rootNode == nullptr)
        return;

    project.files.clear();
    project.keywords.clear();

    QFile file(outputDir + QDir::separator() + project.fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text))
        return;

    QXmlStreamWriter writer(&file);
    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    writer.writeStartElement("QtHelpProject");
    writer.writeAttribute("version", "1.0");

    // Write metaData, virtualFolder and namespace elements.
    writer.writeTextElement("namespace", project.helpNamespace);
    writer.writeTextElement("virtualFolder", project.virtualFolder);
    writer.writeStartElement("metaData");
    writer.writeAttribute("name", "version");
    writer.writeAttribute("value", project.version);
    writer.writeEndElement();

    // Write customFilter elements.
    for (auto it = project.customFilters.constBegin(); it != project.customFilters.constEnd();
         ++it) {
        writer.writeStartElement("customFilter");
        writer.writeAttribute("name", it.key());
        QStringList sortedAttributes = it.value().values();
        sortedAttributes.sort();
        for (const auto &filter : qAsConst(sortedAttributes))
            writer.writeTextElement("filterAttribute", filter);
        writer.writeEndElement(); // customFilter
    }

    // Start the filterSection.
    writer.writeStartElement("filterSection");

    // Write filterAttribute elements.
    QStringList sortedFilterAttributes = project.filterAttributes.values();
    sortedFilterAttributes.sort();
    for (const auto &filterName : qAsConst(sortedFilterAttributes))
        writer.writeTextElement("filterAttribute", filterName);

    writer.writeStartElement("toc");
    writer.writeStartElement("section");
    const Node *node = qdb_->findPageNodeByTitle(project.indexTitle);
    if (node == nullptr)
        node = qdb_->findNodeByNameAndType(QStringList("index.html"), &Node::isPageNode);
    QString indexPath;
    if (node)
        indexPath = gen_->fullDocumentLocation(node, false);
    else
        indexPath = "index.html";
    writer.writeAttribute("ref", indexPath);
    writer.writeAttribute("title", project.indexTitle);

    generateSections(project, writer, rootNode);

    for (int i = 0; i < project.subprojects.length(); i++) {
        SubProject subproject = project.subprojects[i];

        if (subproject.type == QLatin1String("manual")) {

            const Node *indexPage = qdb_->findNodeForTarget(subproject.indexTitle, nullptr);
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

                            const Node *page = qdb_->findNodeForTarget(atom->string(), nullptr);
                            writer.writeStartElement("section");
                            QString indexPath = gen_->fullDocumentLocation(page, false);
                            writer.writeAttribute("ref", indexPath);
                            QString title = atom->string();
                            if (atom->next() && atom->next()->string() == ATOM_FORMATTING_LINK)
                                if (atom->next()->next())
                                    title = atom->next()->next()->string();
                            writer.writeAttribute("title", title);

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
                rootNode->doc().location().warning(
                        tr("Failed to find index: %1").arg(subproject.indexTitle));

        } else {

            writer.writeStartElement("section");
            QString indexPath = gen_->fullDocumentLocation(
                    qdb_->findNodeForTarget(subproject.indexTitle, nullptr), false);
            writer.writeAttribute("ref", indexPath);
            writer.writeAttribute("title", subproject.title);

            if (subproject.sortPages) {
                QStringList titles = subproject.nodes.keys();
                titles.sort();
                for (const auto &title : qAsConst(titles)) {
                    writeNode(project, writer, subproject.nodes[title]);
                }
            } else {
                // Find a contents node and navigate from there, using the NextLink values.
                QSet<QString> visited;
                bool contentsFound = false;
                for (const auto *node : qAsConst(subproject.nodes)) {
                    QString nextTitle = node->links().value(Node::NextLink).first;
                    if (!nextTitle.isEmpty()
                        && node->links().value(Node::ContentsLink).first.isEmpty()) {

                        const Node *nextPage = qdb_->findNodeForTarget(nextTitle, nullptr);

                        // Write the contents node.
                        writeNode(project, writer, node);
                        contentsFound = true;

                        while (nextPage) {
                            writeNode(project, writer, nextPage);
                            nextTitle = nextPage->links().value(Node::NextLink).first;
                            if (nextTitle.isEmpty() || visited.contains(nextTitle))
                                break;
                            nextPage = qdb_->findNodeForTarget(nextTitle, nullptr);
                            visited.insert(nextTitle);
                        }
                        break;
                    }
                }
                // No contents/nextpage links found, write all nodes unsorted
                if (!contentsFound) {
                    QList<const Node *> subnodes = subproject.nodes.values();

                    std::sort(subnodes.begin(), subnodes.end(), Node::nodeNameLessThan);

                    for (const auto *node : qAsConst(subnodes))
                        writeNode(project, writer, node);
                }
            }

            writer.writeEndElement(); // section
        }
    }

    // Restore original search order
    qdb_->setSearchOrder(searchOrder);

    writer.writeEndElement(); // section
    writer.writeEndElement(); // toc

    writer.writeStartElement("keywords");
    std::sort(project.keywords.begin(), project.keywords.end());
    for (const QStringList &details : qAsConst(project.keywords)) {
        writer.writeStartElement("keyword");
        writer.writeAttribute("name", details[0]);
        writer.writeAttribute("id", details[1]);
        writer.writeAttribute("ref", details[2]);
        writer.writeEndElement(); // keyword
    }
    writer.writeEndElement(); // keywords

    writer.writeStartElement("files");

    // The list of files to write is the union of generated files and
    // other files (images and extras) included in the project
    QSet<QString> files =
            QSet<QString>(gen_->outputFileNames().cbegin(), gen_->outputFileNames().cend());
    files.unite(project.files);
    files.unite(project.extraFiles);
    QStringList sortedFiles = files.values();
    sortedFiles.sort();
    for (const auto &usedFile : qAsConst(sortedFiles)) {
        if (!usedFile.isEmpty())
            writer.writeTextElement("file", usedFile);
    }
    writer.writeEndElement(); // files

    writer.writeEndElement(); // filterSection
    writer.writeEndElement(); // QtHelpProject
    writer.writeEndDocument();
    writeHashFile(file);
    file.close();
}

QT_END_NAMESPACE
