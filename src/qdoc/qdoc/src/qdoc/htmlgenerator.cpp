// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "htmlgenerator.h"

#include "access.h"
#include "aggregate.h"
#include "classnode.h"
#include "collectionnode.h"
#include "config.h"
#include "codemarker.h"
#include "codeparser.h"
#include "enumnode.h"
#include "functionnode.h"
#include "helpprojectwriter.h"
#include "manifestwriter.h"
#include "node.h"
#include "propertynode.h"
#include "qdocdatabase.h"
#include "sharedcommentnode.h"
#include "tagfilewriter.h"
#include "tree.h"
#include "quoter.h"
#include "utilities.h"

#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include <QtCore/quuid.h>
#include <QtCore/qversionnumber.h>
#include <QtCore/qregularexpression.h>

#include <cctype>
#include <deque>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

bool HtmlGenerator::s_inUnorderedList { false };

static const Atom openCodeTag {Atom::FormattingLeft, ATOM_FORMATTING_TELETYPE};
static const Atom closeCodeTag {Atom::FormattingRight, ATOM_FORMATTING_TELETYPE};

HtmlGenerator::HtmlGenerator(FileResolver& file_resolver) : XmlGenerator(file_resolver) {}

static void addLink(const QString &linkTarget, QStringView nestedStuff, QString *res)
{
    if (!linkTarget.isEmpty()) {
        *res += QLatin1String("<a href=\"");
        *res += linkTarget;
        *res += QLatin1String("\" translate=\"no\">");
        *res += nestedStuff;
        *res += QLatin1String("</a>");
    } else {
        *res += nestedStuff;
    }
}

/*!
    \internal
    Convenience method that starts an unordered list if not in one.
 */
inline void HtmlGenerator::openUnorderedList()
{
    if (!s_inUnorderedList) {
        out() << "<ul>\n";
        s_inUnorderedList = true;
    }
}

/*!
    \internal
    Convenience method that closes an unordered list if in one.
 */
inline void HtmlGenerator::closeUnorderedList()
{
    if (s_inUnorderedList) {
        out() << "</ul>\n";
        s_inUnorderedList = false;
    }
}

/*!
  Destroys the HTML output generator. Deletes the singleton
  instance of HelpProjectWriter and the ManifestWriter instance.
 */
HtmlGenerator::~HtmlGenerator()
{
    if (m_helpProjectWriter) {
        delete m_helpProjectWriter;
        m_helpProjectWriter = nullptr;
    }

    if (m_manifestWriter) {
        delete m_manifestWriter;
        m_manifestWriter = nullptr;
    }
}

/*!
  Initializes the HTML output generator's data structures
  from the configuration (Config) singleton.
 */
void HtmlGenerator::initializeGenerator()
{
    static const struct
    {
        const char *key;
        const char *left;
        const char *right;
    } defaults[] = { { ATOM_FORMATTING_BOLD, "<b>", "</b>" },
                     { ATOM_FORMATTING_INDEX, "<!--", "-->" },
                     { ATOM_FORMATTING_ITALIC, "<i>", "</i>" },
                     { ATOM_FORMATTING_PARAMETER, "<i translate=\"no\">", "</i>" },
                     { ATOM_FORMATTING_SUBSCRIPT, "<sub>", "</sub>" },
                     { ATOM_FORMATTING_SUPERSCRIPT, "<sup>", "</sup>" },
                     { ATOM_FORMATTING_TELETYPE, "<code translate=\"no\">",
                       "</code>" }, // <tt> tag is not supported in HTML5
                     { ATOM_FORMATTING_TRADEMARK, "<span translate=\"no\">", "&#8482;" },
                     { ATOM_FORMATTING_NOTRANSLATE, "<span translate=\"no\">", "</span>" },
                     { ATOM_FORMATTING_UICONTROL, "<b translate=\"no\">", "</b>" },
                     { ATOM_FORMATTING_UNDERLINE, "<u>", "</u>" },
                     { nullptr, nullptr, nullptr } };

    Generator::initializeGenerator();
    config = &Config::instance();

    /*
      The formatting maps are owned by Generator. They are cleared in
      Generator::terminate().
     */
    for (int i = 0; defaults[i].key; ++i) {
        formattingLeftMap().insert(QLatin1String(defaults[i].key), QLatin1String(defaults[i].left));
        formattingRightMap().insert(QLatin1String(defaults[i].key),
                                    QLatin1String(defaults[i].right));
    }

    QString formatDot{HtmlGenerator::format() + Config::dot};
    m_endHeader = config->get(formatDot + CONFIG_ENDHEADER).asString();
    m_postHeader = config->get(formatDot + HTMLGENERATOR_POSTHEADER).asString();
    m_postPostHeader = config->get(formatDot + HTMLGENERATOR_POSTPOSTHEADER).asString();
    m_prologue = config->get(formatDot + HTMLGENERATOR_PROLOGUE).asString();

    m_footer = config->get(formatDot + HTMLGENERATOR_FOOTER).asString();
    m_address = config->get(formatDot + HTMLGENERATOR_ADDRESS).asString();
    m_noNavigationBar = config->get(formatDot + HTMLGENERATOR_NONAVIGATIONBAR).asBool();
    m_navigationSeparator = config->get(formatDot + HTMLGENERATOR_NAVIGATIONSEPARATOR).asString();
    tocDepth = config->get(formatDot + HTMLGENERATOR_TOCDEPTH).asInt();

    m_project = config->get(CONFIG_PROJECT).asString();
    m_productName = config->get(CONFIG_PRODUCTNAME).asString();
    m_projectDescription = config->get(CONFIG_DESCRIPTION)
            .asString(m_project + QLatin1String(" Reference Documentation"));

    m_projectUrl = config->get(CONFIG_URL).asString();
    tagFile_ = config->get(CONFIG_TAGFILE).asString();
    naturalLanguage = config->get(CONFIG_NATURALLANGUAGE).asString(QLatin1String("en"));

    m_codeIndent = config->get(CONFIG_CODEINDENT).asInt();
    m_codePrefix = config->get(CONFIG_CODEPREFIX).asString();
    m_codeSuffix = config->get(CONFIG_CODESUFFIX).asString();

    /*
      The help file write should be allocated once and only once
      per qdoc execution.
     */
    if (m_helpProjectWriter)
        m_helpProjectWriter->reset(m_project.toLower() + ".qhp", this);
    else
        m_helpProjectWriter = new HelpProjectWriter(m_project.toLower() + ".qhp", this);

    if (!m_manifestWriter)
        m_manifestWriter = new ManifestWriter();

    // Documentation template handling
    m_headerScripts = config->get(formatDot + CONFIG_HEADERSCRIPTS).asString();
    m_headerStyles = config->get(formatDot + CONFIG_HEADERSTYLES).asString();

    // Retrieve the config for the navigation bar
    m_homepage = config->get(CONFIG_NAVIGATION
                             + Config::dot + CONFIG_HOMEPAGE).asString();

    m_hometitle = config->get(CONFIG_NAVIGATION
                              + Config::dot + CONFIG_HOMETITLE)
                              .asString(m_homepage);

    m_landingpage = config->get(CONFIG_NAVIGATION
                                + Config::dot + CONFIG_LANDINGPAGE).asString();

    m_landingtitle = config->get(CONFIG_NAVIGATION
                                 + Config::dot + CONFIG_LANDINGTITLE)
                                 .asString(m_landingpage);

    m_cppclassespage = config->get(CONFIG_NAVIGATION
                                   + Config::dot + CONFIG_CPPCLASSESPAGE).asString();

    m_cppclassestitle = config->get(CONFIG_NAVIGATION
                                    + Config::dot + CONFIG_CPPCLASSESTITLE)
                                    .asString(QLatin1String("C++ Classes"));

    m_qmltypespage = config->get(CONFIG_NAVIGATION
                                 + Config::dot + CONFIG_QMLTYPESPAGE).asString();

    m_qmltypestitle = config->get(CONFIG_NAVIGATION
                                  + Config::dot + CONFIG_QMLTYPESTITLE)
                                  .asString(QLatin1String("QML Types"));

    m_trademarkspage = config->get(CONFIG_NAVIGATION
                                   + Config::dot + CONFIG_TRADEMARKSPAGE).asString();

    m_buildversion = config->get(CONFIG_BUILDVERSION).asString();
}

/*!
  Gracefully terminates the HTML output generator.
 */
void HtmlGenerator::terminateGenerator()
{
    Generator::terminateGenerator();
}

QString HtmlGenerator::format()
{
    return "HTML";
}

/*!
  If qdoc is in the \c {-prepare} phase, traverse the primary
  tree to generate the index file for the current module.

  If qdoc is in the \c {-generate} phase, traverse the primary
  tree to generate all the HTML documentation for the current
  module. Then generate the help file and the tag file.
 */
void HtmlGenerator::generateDocs()
{
    Node *qflags = m_qdb->findClassNode(QStringList("QFlags"));
    if (qflags)
        m_qflagsHref = linkForNode(qflags, nullptr);
    if (!config->preparing())
        Generator::generateDocs();

    if (!config->generating()) {
        QString fileBase =
                m_project.toLower().simplified().replace(QLatin1Char(' '), QLatin1Char('-'));
        m_qdb->generateIndex(outputDir() + QLatin1Char('/') + fileBase + ".index", m_projectUrl,
                             m_projectDescription);
    }

    if (!config->preparing()) {
        m_helpProjectWriter->generate();
        m_manifestWriter->generateManifestFiles();
        /*
          Generate the XML tag file, if it was requested.
        */
        if (!tagFile_.isEmpty()) {
            TagFileWriter tagFileWriter;
            tagFileWriter.generateTagFile(tagFile_, this);
        }
    }
}

/*!
  Generate an html file with the contents of a C++ or QML source file.
 */
void HtmlGenerator::generateExampleFilePage(const Node *en, ResolvedFile resolved_file, CodeMarker *marker)
{
    SubTitleSize subTitleSize = LargeSubTitle;
    QString fullTitle = en->fullTitle();

    beginSubPage(en, linkForExampleFile(resolved_file.get_query()));
    generateHeader(fullTitle, en, marker);
    generateTitle(fullTitle, Text() << en->subtitle(), subTitleSize, en, marker);

    Text text;
    Quoter quoter;
    Doc::quoteFromFile(en->doc().location(), quoter, resolved_file);
    QString code = quoter.quoteTo(en->location(), QString(), QString());
    CodeMarker *codeMarker = CodeMarker::markerForFileName(resolved_file.get_path());
    text << Atom(codeMarker->atomType(), code);
    Atom a(codeMarker->atomType(), code);

    generateText(text, en, codeMarker);
    endSubPage();
}

/*!
  Generate html from an instance of Atom.
 */
qsizetype HtmlGenerator::generateAtom(const Atom *atom, const Node *relative, CodeMarker *marker)
{
    qsizetype idx, skipAhead = 0;
    static bool in_para = false;
    Genus genus = Genus::DontCare;

    switch (atom->type()) {
    case Atom::AutoLink: {
        QString name = atom->string();
        if (relative && relative->name() == name.replace(QLatin1String("()"), QLatin1String())) {
            out() << protectEnc(atom->string());
            break;
        }
        // Allow auto-linking to nodes in API reference
        genus = Genus::API;
    }
        Q_FALLTHROUGH();
    case Atom::NavAutoLink:
        if (!m_inLink && !m_inContents && !m_inSectionHeading) {
            const Node *node = nullptr;
            QString link = getAutoLink(atom, relative, &node, genus);
            if (link.isEmpty()) {
                // Warn if warnings are enabled, linking occurs from a relative
                // node and no target is found. Do not warn about self-linking.
                if (autolinkErrors() && relative && relative != node)
                    relative->doc().location().warning(
                            QStringLiteral("Can't autolink to '%1'").arg(atom->string()));
            } else if (node && node->isDeprecated()) {
                if (relative && (relative->parent() != node) && !relative->isDeprecated())
                    link.clear();
            }
            if (link.isEmpty()) {
                out() << protectEnc(atom->string());
            } else {
                beginLink(link, node, relative);
                generateLink(atom);
                endLink();
            }
        } else {
            out() << protectEnc(atom->string());
        }
        break;
    case Atom::BaseName:
        break;
    case Atom::BriefLeft:
        if (!hasBrief(relative)) {
            skipAhead = skipAtoms(atom, Atom::BriefRight);
            break;
        }
        out() << "<p>";
        rewritePropertyBrief(atom, relative);
        break;
    case Atom::BriefRight:
        if (hasBrief(relative))
            out() << "</p>\n";
        break;
    case Atom::C:
        // This may at one time have been used to mark up C++ code but it is
        // now widely used to write teletype text. As a result, text marked
        // with the \c command is not passed to a code marker.
        out() << formattingLeftMap()[ATOM_FORMATTING_TELETYPE];
        out() << protectEnc(plainCode(atom->string()));
        out() << formattingRightMap()[ATOM_FORMATTING_TELETYPE];
        break;
    case Atom::CaptionLeft:
        out() << "<p class=\"figCaption\">";
        in_para = true;
        break;
    case Atom::CaptionRight:
        endLink();
        if (in_para) {
            out() << "</p>\n";
            in_para = false;
        }
        break;
    case Atom::Qml:
        out() << "<pre class=\"qml\" translate=\"no\">"
              << trimmedTrailing(highlightedCode(indent(m_codeIndent, atom->string()), relative,
                                                 false, Genus::QML),
                                 m_codePrefix, m_codeSuffix)
              << "</pre>\n";
        break;
    case Atom::Code:
        out() << "<pre class=\"cpp\" translate=\"no\">"
              << trimmedTrailing(highlightedCode(indent(m_codeIndent, atom->string()), relative),
                                 m_codePrefix, m_codeSuffix)
              << "</pre>\n";
        break;
    case Atom::CodeBad:
        out() << "<pre class=\"cpp plain\" translate=\"no\">"
              << trimmedTrailing(protectEnc(plainCode(indent(m_codeIndent, atom->string()))),
                                 m_codePrefix, m_codeSuffix)
              << "</pre>\n";
        break;
    case Atom::DetailsLeft:
        out() << "<details>\n";
        if (!atom->string().isEmpty())
            out() << "<summary>" << protectEnc(atom->string()) << "</summary>\n";
        else
            out() << "<summary>...</summary>\n";
        break;
    case Atom::DetailsRight:
        out() << "</details>\n";
        break;
    case Atom::DivLeft:
        out() << "<div";
        if (!atom->string().isEmpty())
            out() << ' ' << atom->string();
        out() << '>';
        break;
    case Atom::DivRight:
        out() << "</div>";
        break;
    case Atom::FootnoteLeft:
        // ### For now
        if (in_para) {
            out() << "</p>\n";
            in_para = false;
        }
        out() << "<!-- ";
        break;
    case Atom::FootnoteRight:
        // ### For now
        out() << "-->\n";
        break;
    case Atom::FormatElse:
    case Atom::FormatEndif:
    case Atom::FormatIf:
        break;
    case Atom::FormattingLeft:
        if (atom->string().startsWith("span "))
            out() << '<' + atom->string() << '>';
        else
            out() << formattingLeftMap()[atom->string()];
        break;
    case Atom::FormattingRight:
        if (atom->string() == ATOM_FORMATTING_LINK) {
            endLink();
        } else if (atom->string() == ATOM_FORMATTING_TRADEMARK) {
            if (appendTrademark(atom)) {
                // Make the trademark symbol a link to navigation.trademarkspage (if set)
                const Node *node{nullptr};
                const Atom tm_link(Atom::NavLink, m_trademarkspage);
                if (const auto &link = getLink(&tm_link, relative, &node);
                        !link.isEmpty() && node != relative)
                    out() << "<a href=\"%1\">%2</a>"_L1.arg(link, formattingRightMap()[atom->string()]);
                else
                    out() << formattingRightMap()[atom->string()];
            }
            out() << "</span>";
        } else if (atom->string().startsWith("span ")) {
            out() << "</span>";
        } else {
            out() << formattingRightMap()[atom->string()];
        }
        break;
    case Atom::AnnotatedList: {
        if (const auto *cn = m_qdb->getCollectionNode(atom->string(), NodeType::Group); cn)
            generateList(cn, marker, atom->string(), Generator::sortOrder(atom->strings().last()));
    } break;
    case Atom::GeneratedList: {
        const auto sortOrder{Generator::sortOrder(atom->strings().last())};
        if (atom->string() == QLatin1String("annotatedclasses")) {
            generateAnnotatedList(relative, marker, m_qdb->getCppClasses().values(), sortOrder);
        } else if (atom->string() == QLatin1String("annotatedexamples")) {
            generateAnnotatedLists(relative, marker, m_qdb->getExamples());
        } else if (atom->string() == QLatin1String("annotatedattributions")) {
            generateAnnotatedLists(relative, marker, m_qdb->getAttributions());
        } else if (atom->string() == QLatin1String("classes")) {
            generateCompactList(Generic, relative, m_qdb->getCppClasses(), true,
                                QStringLiteral(""));
        } else if (atom->string().contains("classes ")) {
            QString rootName = atom->string().mid(atom->string().indexOf("classes") + 7).trimmed();
            generateCompactList(Generic, relative, m_qdb->getCppClasses(), true, rootName);
        } else if (atom->string() == QLatin1String("qmlvaluetypes")
                   || atom->string() == QLatin1String("qmlbasictypes")) {
            generateCompactList(Generic, relative, m_qdb->getQmlValueTypes(), true,
                                QStringLiteral(""));
        } else if (atom->string() == QLatin1String("qmltypes")) {
            generateCompactList(Generic, relative, m_qdb->getQmlTypes(), true, QStringLiteral(""));
        } else if ((idx = atom->string().indexOf(QStringLiteral("bymodule"))) != -1) {
            QDocDatabase *qdb = QDocDatabase::qdocDB();
            QString moduleName = atom->string().mid(idx + 8).trimmed();
            NodeType moduleType = typeFromString(atom);
            if (const auto *cn = qdb->getCollectionNode(moduleName, moduleType)) {
                NodeMap map;
                switch (moduleType) {
                case NodeType::Module:
                    // classesbymodule <module_name>
                    map = cn->getMembers([](const Node *n) { return n->isClassNode(); });
                    generateAnnotatedList(relative, marker, map.values(), sortOrder);
                    break;
                case NodeType::QmlModule:
                    if (atom->string().contains(QLatin1String("qmlvaluetypes")))
                        map = cn->getMembers(NodeType::QmlValueType); // qmlvaluetypesbymodule <module_name>
                    else
                        map = cn->getMembers(NodeType::QmlType);      // qmltypesbymodule <module_name>
                    generateAnnotatedList(relative, marker, map.values(), sortOrder);
                    break;
                default: // fall back to listing all members
                    generateAnnotatedList(relative, marker, cn->members(), sortOrder);
                    break;
                }
            }
        } else if (atom->string() == QLatin1String("classhierarchy")) {
            generateClassHierarchy(relative, m_qdb->getCppClasses());
        } else if (atom->string() == QLatin1String("obsoleteclasses")) {
            generateCompactList(Generic, relative, m_qdb->getObsoleteClasses(), false,
                                QStringLiteral("Q"));
        } else if (atom->string() == QLatin1String("obsoleteqmltypes")) {
            generateCompactList(Generic, relative, m_qdb->getObsoleteQmlTypes(), false,
                                QStringLiteral(""));
        } else if (atom->string() == QLatin1String("obsoletecppmembers")) {
            generateCompactList(Obsolete, relative, m_qdb->getClassesWithObsoleteMembers(), false,
                                QStringLiteral("Q"));
        } else if (atom->string() == QLatin1String("obsoleteqmlmembers")) {
            generateCompactList(Obsolete, relative, m_qdb->getQmlTypesWithObsoleteMembers(), false,
                                QStringLiteral(""));
        } else if (atom->string() == QLatin1String("functionindex")) {
            generateFunctionIndex(relative);
        } else if (atom->string() == QLatin1String("attributions")) {
            generateAnnotatedList(relative, marker, m_qdb->getAttributions().values(), sortOrder);
        } else if (atom->string() == QLatin1String("legalese")) {
            generateLegaleseList(relative, marker);
        } else if (atom->string() == QLatin1String("overviews")) {
            generateList(relative, marker, "overviews", sortOrder);
        } else if (atom->string() == QLatin1String("cpp-modules")) {
            generateList(relative, marker, "cpp-modules", sortOrder);
        } else if (atom->string() == QLatin1String("qml-modules")) {
            generateList(relative, marker, "qml-modules", sortOrder);
        } else if (atom->string() == QLatin1String("namespaces")) {
            generateAnnotatedList(relative, marker, m_qdb->getNamespaces().values(), sortOrder);
        } else if (atom->string() == QLatin1String("related")) {
            generateList(relative, marker, "related", sortOrder);
        } else {
            const CollectionNode *cn = m_qdb->getCollectionNode(atom->string(), NodeType::Group);
            if (cn) {
                if (!generateGroupList(const_cast<CollectionNode *>(cn), sortOrder))
                    relative->location().warning(
                            QString("'\\generatelist %1' group is empty").arg(atom->string()));
            } else {
                relative->location().warning(
                        QString("'\\generatelist %1' no such group").arg(atom->string()));
            }
        }
    } break;
    case Atom::SinceList: {
        const NodeMultiMap &nsmap = m_qdb->getSinceMap(atom->string());
        if (nsmap.isEmpty())
            break;

        const NodeMultiMap &ncmap = m_qdb->getClassMap(atom->string());
        const NodeMultiMap &nqcmap = m_qdb->getQmlTypeMap(atom->string());

        Sections sections(nsmap);
        out() << "<ul>\n";
        const QList<Section> sinceSections = sections.sinceSections();
        for (const auto &section : sinceSections) {
            if (!section.members().isEmpty()) {
                out() << "<li>"
                      << "<a href=\"#" << Utilities::asAsciiPrintable(section.title()) << "\">"
                      << section.title() << "</a></li>\n";
            }
        }
        out() << "</ul>\n";

        int index = 0;
        for (const auto &section : sinceSections) {
            if (!section.members().isEmpty()) {
                out() << "<h3 id=\"" << Utilities::asAsciiPrintable(section.title()) << "\">"
                      << protectEnc(section.title()) << "</h3>\n";
                if (index == Sections::SinceClasses)
                    generateCompactList(Generic, relative, ncmap, false, QStringLiteral("Q"));
                else if (index == Sections::SinceQmlTypes)
                    generateCompactList(Generic, relative, nqcmap, false, QStringLiteral(""));
                else if (index == Sections::SinceMemberFunctions
                         || index == Sections::SinceQmlMethods
                         || index == Sections::SinceQmlProperties) {

                    QMap<QString, NodeMultiMap> parentmaps;

                    const QList<Node *> &members = section.members();
                    for (const auto &member : members) {
                        QString parent_full_name = (*member).parent()->fullName();

                        auto parent_entry = parentmaps.find(parent_full_name);
                        if (parent_entry == parentmaps.end())
                            parent_entry = parentmaps.insert(parent_full_name, NodeMultiMap());
                        parent_entry->insert(member->name(), member);
                    }

                    for (auto map = parentmaps.begin(); map != parentmaps.end(); ++map) {
                        NodeVector nv = map->values().toVector();
                        auto parent = nv.front()->parent();

                        out() << ((index == Sections::SinceMemberFunctions) ? "<p>Class " : "<p>QML Type ");

                        out() << "<a href=\"" << linkForNode(parent, relative) << "\" translate=\"no\">";
                        QStringList pieces = parent->fullName().split("::");
                        out() << protectEnc(pieces.last());
                        out() << "</a>"
                              << ":</p>\n";

                        generateSection(nv, relative, marker);
                        out() << "<br/>";
                    }
                } else if (index == Sections::SinceEnumValues) {
                    out() << "<div class=\"table\"><table class=\"alignedsummary\" translate=\"no\">\n";
                    const auto map_it = m_qdb->newEnumValueMaps().constFind(atom->string());
                    for (auto it = map_it->cbegin(); it != map_it->cend(); ++it) {
                        out() << "<tr><td class=\"memItemLeft\"> enum value </td><td class=\"memItemRight\">"
                              << "<b><a href=\"" << linkForNode(it.value(), nullptr) << "\">"
                              << it.key() << "</a></b></td></tr>\n";
                    }
                    out() << "</table></div>\n";
                } else {
                    generateSection(section.members(), relative, marker);
                }
            }
            ++index;
        }
    } break;
    case Atom::BR:
        out() << "<br />\n";
        break;
    case Atom::HR:
        out() << "<hr />\n";
        break;
    case Atom::Image:
    case Atom::InlineImage: {
        QString text;
        if (atom->next() && atom->next()->type() == Atom::ImageText)
            text = atom->next()->string();
        if (atom->type() == Atom::Image)
            out() << "<p class=\"centerAlign\">";

        auto maybe_resolved_file{file_resolver.resolve(atom->string())};
        if (!maybe_resolved_file) {
            // TODO: [uncentralized-admonition]
            relative->location().warning(
                    QStringLiteral("Missing image: %1").arg(protectEnc(atom->string())));
            out() << "<font color=\"red\">[Missing image " << protectEnc(atom->string())
                  << "]</font>";
        } else {
            ResolvedFile file{*maybe_resolved_file};
            QString file_name{QFileInfo{file.get_path()}.fileName()};

            // TODO: [operation-can-fail-making-the-output-incorrect]
            // The operation of copying the file can fail, making the
            // output refer to an image that does not exist.
            // This should be fine as HTML will take care of managing
            // the rendering of a missing image, but what html will
            // render is in stark contrast with what we do when the
            // image does not exist at all.
            // It may be more correct to unify the behavior between
            // the two either by considering images that cannot be
            // copied as missing or letting the HTML renderer
            // always taking care of the two cases.
            // Do notice that effectively doing this might be
            // unnecessary as extracting the output directory logic
            // should ensure that a safe assumption for copy should be
            // made at the API boundary.

            // TODO: [uncentralized-output-directory-structure]
            Config::copyFile(relative->doc().location(), file.get_path(), file_name, outputDir() + QLatin1String("/images"));

            // TODO: [uncentralized-output-directory-structure]
            out() << "<img src=\"" << "images/" + protectEnc(file_name) << '"';

            const QString altAndTitleText = protectEnc(text);
            out() << " alt=\"" << altAndTitleText;
            if (Config::instance().get(CONFIG_USEALTTEXTASTITLE).asBool())
                out()  << "\" title=\"" << altAndTitleText;
            out() << "\" />";

            // TODO: [uncentralized-output-directory-structure]
            m_helpProjectWriter->addExtraFile("images/" + file_name);
            setImageFileName(relative, "images/" + file_name);
        }

        if (atom->type() == Atom::Image)
            out() << "</p>";
    } break;
    case Atom::ImageText:
        break;
    // Admonitions
    case Atom::ImportantLeft:
    case Atom::NoteLeft:
    case Atom::WarningLeft: {
        QString admonType = atom->typeString();
        // Remove 'Left' from atom type to get the admonition type
        admonType.chop(4);
        out() << "<div class=\"admonition " << admonType.toLower() << "\">\n"
              << "<p>";
        out() << formattingLeftMap()[ATOM_FORMATTING_BOLD];
        out() << admonType << ": ";
        out() << formattingRightMap()[ATOM_FORMATTING_BOLD];
    } break;
    case Atom::ImportantRight:
    case Atom::NoteRight:
    case Atom::WarningRight:
        out() << "</p>\n"
              << "</div>\n";
        break;
    case Atom::LegaleseLeft:
        out() << "<div class=\"LegaleseLeft\">";
        break;
    case Atom::LegaleseRight:
        out() << "</div>";
        break;
    case Atom::LineBreak:
        out() << "<br/>";
        break;
    case Atom::Link:
        // Prevent nested links in table of contents
        if (m_inContents)
            break;
        Q_FALLTHROUGH();
    case Atom::NavLink: {
        const Node *node = nullptr;
        QString link = getLink(atom, relative, &node);
        if (link.isEmpty() && (node != relative) && !noLinkErrors()) {
            Location location = atom->isLinkAtom() ? static_cast<const LinkAtom*>(atom)->location
                                                   : relative->doc().location();
            location.warning(
                    QStringLiteral("Can't link to '%1'").arg(atom->string()));
        }
        beginLink(link, node, relative);
        skipAhead = 1;
    } break;
    case Atom::ExampleFileLink: {
        QString link = linkForExampleFile(atom->string());
        beginLink(link);
        skipAhead = 1;
    } break;
    case Atom::ExampleImageLink: {
        QString link = atom->string();
        link = "images/used-in-examples/" + link;
        beginLink(link);
        skipAhead = 1;
    } break;
    case Atom::LinkNode: {
        const Node *node = static_cast<const Node*>(Utilities::nodeForString(atom->string()));
        beginLink(linkForNode(node, relative), node, relative);
        skipAhead = 1;
    } break;
    case Atom::ListLeft:
        if (in_para) {
            out() << "</p>\n";
            in_para = false;
        }
        if (atom->string() == ATOM_LIST_BULLET) {
            out() << "<ul>\n";
        } else if (atom->string() == ATOM_LIST_TAG) {
            out() << "<dl>\n";
        } else if (atom->string() == ATOM_LIST_VALUE) {
            out() << R"(<div class="table"><table class="valuelist">)";
            m_threeColumnEnumValueTable = isThreeColumnEnumValueTable(atom);
            if (m_threeColumnEnumValueTable) {
                if (++m_numTableRows % 2 == 1)
                    out() << R"(<tr valign="top" class="odd">)";
                else
                    out() << R"(<tr valign="top" class="even">)";

                out() << "<th class=\"tblConst\">Constant</th>";

                // If not in \enum topic, skip the value column
                if (relative->isEnumType(Genus::CPP))
                    out() << "<th class=\"tblval\">Value</th>";

                out() << "<th class=\"tbldscr\">Description</th></tr>\n";
            } else {
                out() << "<tr><th class=\"tblConst\">Constant</th><th "
                         "class=\"tblVal\">Value</th></tr>\n";
            }
        } else {
            QString olType;
            if (atom->string() == ATOM_LIST_UPPERALPHA) {
                olType = "A";
            } else if (atom->string() == ATOM_LIST_LOWERALPHA) {
                olType = "a";
            } else if (atom->string() == ATOM_LIST_UPPERROMAN) {
                olType = "I";
            } else if (atom->string() == ATOM_LIST_LOWERROMAN) {
                olType = "i";
            } else { // (atom->string() == ATOM_LIST_NUMERIC)
                olType = "1";
            }

            if (atom->next() != nullptr && atom->next()->string().toInt() > 1) {
                out() << QString(R"(<ol class="%1" type="%1" start="%2">)")
                                 .arg(olType, atom->next()->string());
            } else
                out() << QString(R"(<ol class="%1" type="%1">)").arg(olType);
        }
        break;
    case Atom::ListItemNumber:
        break;
    case Atom::ListTagLeft:
        if (atom->string() == ATOM_LIST_TAG) {
            out() << "<dt>";
        } else { // (atom->string() == ATOM_LIST_VALUE)
            std::pair<QString, int> pair = getAtomListValue(atom);
            skipAhead = pair.second;
            QString t = protectEnc(plainCode(marker->markedUpEnumValue(pair.first, relative)));
            out() << "<tr><td class=\"topAlign\"><code translate=\"no\">" << t << "</code>";

            if (relative->isEnumType(Genus::CPP)) {
                out() << "</td><td class=\"topAlign tblval\">";
                const auto *enume = static_cast<const EnumNode *>(relative);
                QString itemValue = enume->itemValue(atom->next()->string());
                if (itemValue.isEmpty())
                    out() << '?';
                else
                    out() << "<code translate=\"no\">" << protectEnc(itemValue) << "</code>";
            }
        }
        break;
    case Atom::SinceTagRight:
    case Atom::ListTagRight:
        if (atom->string() == ATOM_LIST_TAG)
            out() << "</dt>\n";
        break;
    case Atom::ListItemLeft:
        if (atom->string() == ATOM_LIST_TAG) {
            out() << "<dd>";
        } else if (atom->string() == ATOM_LIST_VALUE) {
            if (m_threeColumnEnumValueTable) {
                out() << "</td><td class=\"topAlign\">";
                if (matchAhead(atom, Atom::ListItemRight))
                    out() << "&nbsp;";
            }
        } else {
            out() << "<li>";
        }
        if (matchAhead(atom, Atom::ParaLeft))
            skipAhead = 1;
        break;
    case Atom::ListItemRight:
        if (atom->string() == ATOM_LIST_TAG) {
            out() << "</dd>\n";
        } else if (atom->string() == ATOM_LIST_VALUE) {
            out() << "</td></tr>\n";
        } else {
            out() << "</li>\n";
        }
        break;
    case Atom::ListRight:
        if (atom->string() == ATOM_LIST_BULLET) {
            out() << "</ul>\n";
        } else if (atom->string() == ATOM_LIST_TAG) {
            out() << "</dl>\n";
        } else if (atom->string() == ATOM_LIST_VALUE) {
            out() << "</table></div>\n";
        } else {
            out() << "</ol>\n";
        }
        break;
    case Atom::Nop:
        break;
    case Atom::ParaLeft:
        out() << "<p>";
        in_para = true;
        break;
    case Atom::ParaRight:
        endLink();
        if (in_para) {
            out() << "</p>\n";
            in_para = false;
        }
        // if (!matchAhead(atom, Atom::ListItemRight) && !matchAhead(atom, Atom::TableItemRight))
        //    out() << "</p>\n";
        break;
    case Atom::QuotationLeft:
        out() << "<blockquote>";
        break;
    case Atom::QuotationRight:
        out() << "</blockquote>\n";
        break;
    case Atom::RawString:
        out() << atom->string();
        break;
    case Atom::SectionLeft:
    case Atom::SectionRight:
        break;
    case Atom::SectionHeadingLeft: {
        int unit = atom->string().toInt() + hOffset(relative);
        out() << "<h" + QString::number(unit) + QLatin1Char(' ') << "id=\""
              << Tree::refForAtom(atom) << "\">";
        m_inSectionHeading = true;
        break;
    }
    case Atom::SectionHeadingRight:
        out() << "</h" + QString::number(atom->string().toInt() + hOffset(relative)) + ">\n";
        m_inSectionHeading = false;
        break;
    case Atom::SidebarLeft:
        Q_FALLTHROUGH();
    case Atom::SidebarRight:
        break;
    case Atom::String:
        if (m_inLink && !m_inContents && !m_inSectionHeading) {
            generateLink(atom);
        } else {
            out() << protectEnc(atom->string());
        }
        break;
    case Atom::TableLeft: {
        std::pair<QString, QString> pair = getTableWidthAttr(atom);
        QString attr = pair.second;
        QString width = pair.first;

        if (in_para) {
            out() << "</p>\n";
            in_para = false;
        }

        out() << R"(<div class="table"><table class=")" << attr << '"';
        if (!width.isEmpty())
            out() << " width=\"" << width << '"';
        out() << ">\n ";
        m_numTableRows = 0;
    } break;
    case Atom::TableRight:
        out() << "</table></div>\n";
        break;
    case Atom::TableHeaderLeft:
        out() << "<thead><tr class=\"qt-style\">";
        m_inTableHeader = true;
        break;
    case Atom::TableHeaderRight:
        out() << "</tr>";
        if (matchAhead(atom, Atom::TableHeaderLeft)) {
            skipAhead = 1;
            out() << "\n<tr class=\"qt-style\">";
        } else {
            out() << "</thead>\n";
            m_inTableHeader = false;
        }
        break;
    case Atom::TableRowLeft:
        if (!atom->string().isEmpty())
            out() << "<tr " << atom->string() << '>';
        else if (++m_numTableRows % 2 == 1)
            out() << R"(<tr valign="top" class="odd">)";
        else
            out() << R"(<tr valign="top" class="even">)";
        break;
    case Atom::TableRowRight:
        out() << "</tr>\n";
        break;
    case Atom::TableItemLeft: {
        if (m_inTableHeader)
            out() << "<th ";
        else
            out() << "<td ";

        for (int i = 0; i < atom->count(); ++i) {
            if (i > 0)
                out() << ' ';
            const QString &p = atom->string(i);
            if (p.contains('=')) {
                out() << p;
            } else {
                QStringList spans = p.split(QLatin1Char(','));
                if (spans.size() == 2) {
                    if (spans.at(0) != "1")
                        out() << " colspan=\"" << spans.at(0) << '"';
                    if (spans.at(1) != "1")
                        out() << " rowspan=\"" << spans.at(1) << '"';
                }
            }
        }
        out() << '>';
        if (matchAhead(atom, Atom::ParaLeft))
            skipAhead = 1;
    } break;
    case Atom::TableItemRight:
        if (m_inTableHeader)
            out() << "</th>";
        else {
            out() << "</td>";
        }
        if (matchAhead(atom, Atom::ParaLeft))
            skipAhead = 1;
        break;
    case Atom::TableOfContents:
        Q_FALLTHROUGH();
    case Atom::Keyword:
        break;
    case Atom::Target:
        out() << "<span id=\"" << Utilities::asAsciiPrintable(atom->string()) << "\"></span>";
        break;
    case Atom::UnhandledFormat:
        out() << "<b class=\"redFont\">&lt;Missing HTML&gt;</b>";
        break;
    case Atom::UnknownCommand:
        out() << R"(<b class="redFont"><code translate=\"no\">\)" << protectEnc(atom->string()) << "</code></b>";
        break;
    case Atom::CodeQuoteArgument:
    case Atom::CodeQuoteCommand:
    case Atom::ComparesLeft:
    case Atom::ComparesRight:
    case Atom::SnippetCommand:
    case Atom::SnippetIdentifier:
    case Atom::SnippetLocation:
        // no HTML output (ignore)
        break;
    default:
        unknownAtom(atom);
    }
    return skipAhead;
}

/*!
 * Return a string representing a text that exposes information about
 * the user-visible groups that the \a node is part of. A user-visible
 * group is a group that generates an output page, that is, a \\group
 * topic exists for the group and can be linked to.
 *
 * The returned string is composed of comma separated links to the
 * groups, with their title as the user-facing text, surrounded by
 * some introductory text.
 *
 * For example, if a node named N is part of the groups with title A
 * and B, the line rendered form of the line will be "N is part of the
 * A, B groups", where A and B are clickable links that target the
 * respective page of each group.
 *
 * If a node has a single group, the comma is removed for readability
 * pusposes and "groups" is expressed as a singular noun.
 * For example, "N is part of the A group".
 *
 * The returned string is empty when the node is not linked to any
 * group that has a valid link target.
 *
 * This string is used in the summary of c++ classes or qml types to
 * link them to some of the overview documentation that is generated
 * through the "\group" command.
 *
 * Note that this is currently, incorrectly, a member of
 * HtmlGenerator as it requires access to some protected/private
 * members for escaping and linking.
 */
QString HtmlGenerator::groupReferenceText(PageNode* node) {
    auto link_for_group = [this](const CollectionNode *group) -> QString {
        QString target{linkForNode(group, nullptr)};
        return (target.isEmpty()) ? protectEnc(group->name()) : "<a href=\"" + target + "\">" + protectEnc(group->fullTitle()) + "</a>";
    };

    QString text{};

    const QStringList &groups_names{node->groupNames()};
    if (groups_names.isEmpty())
        return text;

    std::vector<CollectionNode *> groups_nodes(groups_names.size(), nullptr);
    std::transform(groups_names.cbegin(), groups_names.cend(), groups_nodes.begin(),
            [this](const QString &group_name) -> CollectionNode* {
                CollectionNode *group{m_qdb->groups()[group_name]};
                m_qdb->mergeCollections(group);
                return (group && group->wasSeen()) ? group : nullptr;
            });
    groups_nodes.erase(std::remove(groups_nodes.begin(), groups_nodes.end(), nullptr), groups_nodes.end());

    if (!groups_nodes.empty()) {
        text += node->name() + " is part of ";

        for (std::vector<CollectionNode *>::size_type index{0}; index < groups_nodes.size(); ++index) {
            text += link_for_group(groups_nodes[index]) + Utilities::separator(index, groups_nodes.size());
        }
    }
    return text;
}

/*!
  Generate a reference page for the C++ class, namespace, or
  header file documented in \a node using the code \a marker
  provided.
 */
void HtmlGenerator::generateCppReferencePage(Aggregate *aggregate, CodeMarker *marker)
{
    QString title;
    QString fullTitle;
    NamespaceNode *ns = nullptr;
    SectionVector *summarySections = nullptr;
    SectionVector *detailsSections = nullptr;

    Sections sections(aggregate);
    QString typeWord = aggregate->typeWord(true);
    auto templateDecl = aggregate->templateDecl();
    if (aggregate->isNamespace()) {
        fullTitle = aggregate->plainFullName();
        title = "%1 %2"_L1.arg(fullTitle, typeWord);
        ns = static_cast<NamespaceNode *>(aggregate);
        summarySections = &sections.stdSummarySections();
        detailsSections = &sections.stdDetailsSections();
    } else if (aggregate->isClassNode()) {
        fullTitle = aggregate->plainFullName();
        title = "%1 %2"_L1.arg(fullTitle, typeWord);
        summarySections = &sections.stdCppClassSummarySections();
        detailsSections = &sections.stdCppClassDetailsSections();
    } else if (aggregate->isHeader()) {
        title = fullTitle = aggregate->fullTitle();
        summarySections = &sections.stdSummarySections();
        detailsSections = &sections.stdDetailsSections();
    }

    Text subtitleText;
    // Generate a subtitle if there are parents to link to, or a template declaration
    if (aggregate->parent()->isInAPI() || templateDecl) {
        if (templateDecl)
            subtitleText << "%1 "_L1.arg((*templateDecl).to_qstring());
        subtitleText << aggregate->typeWord(false) << " "_L1;
        auto ancestors = fullTitle.split("::"_L1);
        ancestors.pop_back();
        for (const auto &a : ancestors)
            subtitleText << Atom(Atom::AutoLink, a) << "::"_L1;
        subtitleText << aggregate->plainName();
    }

    generateHeader(title, aggregate, marker);
    generateTableOfContents(aggregate, marker, summarySections);
    generateTitle(title, subtitleText, SmallSubTitle, aggregate, marker);
    if (ns && !ns->hasDoc() && ns->docNode()) {
        NamespaceNode *fullNamespace = ns->docNode();
        Text brief;
        brief << "The " << ns->name() << " namespace includes the following elements from module "
              << ns->tree()->camelCaseModuleName() << ". The full namespace is "
              << "documented in module " << fullNamespace->tree()->camelCaseModuleName();
        addNodeLink(brief, fullNamespace, " here.");
        out() << "<p>";
        generateText(brief, ns, marker);
        out() << "</p>\n";
    } else
        generateBrief(aggregate, marker);

    const auto parentIsClass = aggregate->parent()->isClassNode();

    if (!parentIsClass)
        generateRequisites(aggregate, marker);
    generateStatus(aggregate, marker);
    if (parentIsClass)
        generateSince(aggregate, marker);

    QString membersLink = generateAllMembersFile(Sections::allMembersSection(), marker);
    if (!membersLink.isEmpty()) {
        openUnorderedList();
        out() << "<li><a href=\"" << membersLink << "\">"
              << "List of all members, including inherited members</a></li>\n";
    }
    QString obsoleteLink = generateObsoleteMembersFile(sections, marker);
    if (!obsoleteLink.isEmpty()) {
        openUnorderedList();
        out() << "<li><a href=\"" << obsoleteLink << "\">"
              << "Deprecated members</a></li>\n";
    }

    if (QString groups_text{groupReferenceText(aggregate)}; !groups_text.isEmpty()) {
        openUnorderedList();

        out() << "<li>" << groups_text << "</li>\n";
    }

    closeUnorderedList();
    generateComparisonCategory(aggregate, marker);
    generateComparisonList(aggregate);

    generateThreadSafeness(aggregate, marker);

    bool needOtherSection = false;

    for (const auto &section : std::as_const(*summarySections)) {
        if (section.members().isEmpty() && section.reimplementedMembers().isEmpty()) {
            if (!section.inheritedMembers().isEmpty())
                needOtherSection = true;
        } else {
            if (!section.members().isEmpty()) {
                QString ref = registerRef(section.title().toLower());
                out() << "<h2 id=\"" << ref << "\">" << protectEnc(section.title()) << "</h2>\n";
                generateSection(section.members(), aggregate, marker);
            }
            if (!section.reimplementedMembers().isEmpty()) {
                QString name = QString("Reimplemented ") + section.title();
                QString ref = registerRef(name.toLower());
                out() << "<h2 id=\"" << ref << "\">" << protectEnc(name) << "</h2>\n";
                generateSection(section.reimplementedMembers(), aggregate, marker);
            }

            if (!section.inheritedMembers().isEmpty()) {
                out() << "<ul>\n";
                generateSectionInheritedList(section, aggregate);
                out() << "</ul>\n";
            }
        }
    }

    if (needOtherSection) {
        out() << "<h3>Additional Inherited Members</h3>\n"
                 "<ul>\n";

        for (const auto &section : std::as_const(*summarySections)) {
            if (section.members().isEmpty() && !section.inheritedMembers().isEmpty())
                generateSectionInheritedList(section, aggregate);
        }
        out() << "</ul>\n";
    }

    if (aggregate->doc().isEmpty()) {
        QString command = "documentation";
        if (aggregate->isClassNode())
            command = R"('\class' comment)";
        if (!ns || ns->isDocumentedHere()) {
            aggregate->location().warning(
                    QStringLiteral("No %1 for '%2'").arg(command, aggregate->plainSignature()));
        }
    } else {
        generateExtractionMark(aggregate, DetailedDescriptionMark);
        out() << "<div class=\"descr\">\n"
              << "<h2 id=\"" << registerRef("details") << "\">"
              << "Detailed Description"
              << "</h2>\n";
        generateBody(aggregate, marker);
        out() << "</div>\n";
        generateAlsoList(aggregate, marker);
        generateExtractionMark(aggregate, EndMark);
    }

    for (const auto &section : std::as_const(*detailsSections)) {
        bool headerGenerated = false;
        if (section.isEmpty())
            continue;

        const QList<Node *> &members = section.members();
        for (const auto &member : members) {
            if (member->access() == Access::Private) // ### check necessary?
                continue;
            if (!headerGenerated) {
                if (!section.divClass().isEmpty())
                    out() << "<div class=\"" << section.divClass() << "\">\n";
                out() << "<h2>" << protectEnc(section.title()) << "</h2>\n";
                headerGenerated = true;
            }
            if (!member->isClassNode())
                generateDetailedMember(member, aggregate, marker);
            else {
                out() << "<h3> class ";
                generateFullName(member, aggregate);
                out() << "</h3>";
                generateBrief(member, marker, aggregate);
            }
        }
        if (headerGenerated && !section.divClass().isEmpty())
            out() << "</div>\n";
    }
    generateFooter(aggregate);
}

void HtmlGenerator::generateProxyPage(Aggregate *aggregate, CodeMarker *marker)
{
    Q_ASSERT(aggregate->isProxyNode());

    QString title;
    QString rawTitle;
    QString fullTitle;
    Text subtitleText;
    SectionVector *summarySections = nullptr;
    SectionVector *detailsSections = nullptr;

    Sections sections(aggregate);
    rawTitle = aggregate->plainName();
    fullTitle = aggregate->plainFullName();
    title = rawTitle + " Proxy Page";
    summarySections = &sections.stdSummarySections();
    detailsSections = &sections.stdDetailsSections();
    generateHeader(title, aggregate, marker);
    generateTitle(title, subtitleText, SmallSubTitle, aggregate, marker);
    generateBrief(aggregate, marker);
    for (auto it = summarySections->constBegin(); it != summarySections->constEnd(); ++it) {
        if (!it->members().isEmpty()) {
            QString ref = registerRef(it->title().toLower());
            out() << "<h2 id=\"" << ref << "\">" << protectEnc(it->title()) << "</h2>\n";
            generateSection(it->members(), aggregate, marker);
        }
    }

    if (!aggregate->doc().isEmpty()) {
        generateExtractionMark(aggregate, DetailedDescriptionMark);
        out() << "<div class=\"descr\">\n"
              << "<h2 id=\"" << registerRef("details") << "\">"
              << "Detailed Description"
              << "</h2>\n";
        generateBody(aggregate, marker);
        out() << "</div>\n";
        generateAlsoList(aggregate, marker);
        generateExtractionMark(aggregate, EndMark);
    }

    for (const auto &section : std::as_const(*detailsSections)) {
        if (section.isEmpty())
            continue;

        if (!section.divClass().isEmpty())
            out() << "<div class=\"" << section.divClass() << "\">\n";
        out() << "<h2>" << protectEnc(section.title()) << "</h2>\n";

        const QList<Node *> &members = section.members();
        for (const auto &member : members) {
            if (!member->isPrivate()) { // ### check necessary?
                if (!member->isClassNode())
                    generateDetailedMember(member, aggregate, marker);
                else {
                    out() << "<h3> class ";
                    generateFullName(member, aggregate);
                    out() << "</h3>";
                    generateBrief(member, marker, aggregate);
                }
            }
        }
        if (!section.divClass().isEmpty())
            out() << "</div>\n";
    }
    generateFooter(aggregate);
}

/*!
  Generate the HTML page for a QML type. \qcn is the QML type.
  \marker is the code markeup object.
 */
void HtmlGenerator::generateQmlTypePage(QmlTypeNode *qcn, CodeMarker *marker)
{
    Generator::setQmlTypeContext(qcn);
    SubTitleSize subTitleSize = LargeSubTitle;
    QString htmlTitle = qcn->fullTitle();
    if (qcn->isQmlBasicType())
        htmlTitle.append(" QML Value Type");
    else
        htmlTitle.append(" QML Type");

    generateHeader(htmlTitle, qcn, marker);
    Sections sections(qcn);
    generateTableOfContents(qcn, marker, &sections.stdQmlTypeSummarySections());
    marker = CodeMarker::markerForLanguage(QLatin1String("QML"));
    generateTitle(htmlTitle, Text() << qcn->subtitle(), subTitleSize, qcn, marker);
    generateBrief(qcn, marker);
    generateQmlRequisites(qcn, marker);
    generateStatus(qcn, marker);

    QString allQmlMembersLink;

    // No 'All Members' file for QML value types
    if (!qcn->isQmlBasicType())
        allQmlMembersLink = generateAllQmlMembersFile(sections, marker);
    QString obsoleteLink = generateObsoleteQmlMembersFile(sections, marker);
    if (!allQmlMembersLink.isEmpty() || !obsoleteLink.isEmpty()) {
        openUnorderedList();

        if (!allQmlMembersLink.isEmpty()) {
            out() << "<li><a href=\"" << allQmlMembersLink << "\">"
                  << "List of all members, including inherited members</a></li>\n";
        }
        if (!obsoleteLink.isEmpty()) {
            out() << "<li><a href=\"" << obsoleteLink << "\">"
                  << "Deprecated members</a></li>\n";
        }
    }

    if (QString groups_text{groupReferenceText(qcn)}; !groups_text.isEmpty()) {
        openUnorderedList();

        out() << "<li>" << groups_text << "</li>\n";
    }

    closeUnorderedList();

    const QList<Section> &stdQmlTypeSummarySections = sections.stdQmlTypeSummarySections();
    for (const auto &section : stdQmlTypeSummarySections) {
        if (!section.isEmpty()) {
            QString ref = registerRef(section.title().toLower());
            out() << "<h2 id=\"" << ref << "\">" << protectEnc(section.title()) << "</h2>\n";
            generateQmlSummary(section.members(), qcn, marker);
        }
    }

    generateExtractionMark(qcn, DetailedDescriptionMark);
    out() << "<h2 id=\"" << registerRef("details") << "\">"
          << "Detailed Description"
          << "</h2>\n";
    generateBody(qcn, marker);
    generateAlsoList(qcn, marker);
    generateExtractionMark(qcn, EndMark);

    const QList<Section> &stdQmlTypeDetailsSections = sections.stdQmlTypeDetailsSections();
    for (const auto &section : stdQmlTypeDetailsSections) {
        if (!section.isEmpty()) {
            out() << "<h2>" << protectEnc(section.title()) << "</h2>\n";
            const QList<Node *> &members = section.members();
            for (const auto member : members) {
                generateDetailedQmlMember(member, qcn, marker);
                out() << "<br/>\n";
            }
        }
    }
    generateFooter(qcn);
    Generator::setQmlTypeContext(nullptr);
}

/*!
  Generate the HTML page for an entity that doesn't map
  to any underlying parsable C++ or QML element.
 */
void HtmlGenerator::generatePageNode(PageNode *pn, CodeMarker *marker)
{
    SubTitleSize subTitleSize = LargeSubTitle;
    QString fullTitle = pn->fullTitle();

    generateHeader(fullTitle, pn, marker);
    /*
      Generate the TOC for the new doc format.
      Don't generate a TOC for the home page.
    */
    if ((pn->name() != QLatin1String("index.html")))
        generateTableOfContents(pn, marker, nullptr);

    generateTitle(fullTitle, Text() << pn->subtitle(), subTitleSize, pn, marker);
    if (pn->isExample()) {
        generateBrief(pn, marker, nullptr, false);
    }

    generateExtractionMark(pn, DetailedDescriptionMark);
    out() << R"(<div class="descr" id=")" << registerRef("details")
          << "\">\n";

    generateBody(pn, marker);
    out() << "</div>\n";
    generateAlsoList(pn, marker);
    generateExtractionMark(pn, EndMark);

    generateFooter(pn);
}

/*!
  Generate the HTML page for a group, module, or QML module.
 */
void HtmlGenerator::generateCollectionNode(CollectionNode *cn, CodeMarker *marker)
{
    SubTitleSize subTitleSize = LargeSubTitle;
    QString fullTitle = cn->fullTitle();
    QString ref;

    generateHeader(fullTitle, cn, marker);
    generateTableOfContents(cn, marker, nullptr);
    generateTitle(fullTitle, Text() << cn->subtitle(), subTitleSize, cn, marker);

    // Generate brief for C++ modules, status for all modules.
    if (cn->genus() != Genus::DOC && cn->genus() != Genus::DontCare) {
        if (cn->isModule())
            generateBrief(cn, marker);
        generateStatus(cn, marker);
        generateSince(cn, marker);
    }

    if (cn->isModule()) {
        if (!cn->noAutoList()) {
            NodeMap nmm{cn->getMembers(NodeType::Namespace)};
            if (!nmm.isEmpty()) {
                ref = registerRef("namespaces");
                out() << "<h2 id=\"" << ref << "\">Namespaces</h2>\n";
                generateAnnotatedList(cn, marker, nmm.values());
            }
            nmm = cn->getMembers([](const Node *n){ return n->isClassNode(); });
            if (!nmm.isEmpty()) {
                ref = registerRef("classes");
                out() << "<h2 id=\"" << ref << "\">Classes</h2>\n";
                generateAnnotatedList(cn, marker, nmm.values());
            }
        }
    }

    if (cn->isModule() && !cn->doc().briefText().isEmpty()) {
        generateExtractionMark(cn, DetailedDescriptionMark);
        ref = registerRef("details");
        out() << "<div class=\"descr\">\n";
        out() << "<h2 id=\"" << ref << "\">"
              << "Detailed Description"
              << "</h2>\n";
    } else {
        generateExtractionMark(cn, DetailedDescriptionMark);
        out() << R"(<div class="descr" id=")" << registerRef("details")
              << "\">\n";
    }

    generateBody(cn, marker);
    out() << "</div>\n";
    generateAlsoList(cn, marker);
    generateExtractionMark(cn, EndMark);

    if (!cn->noAutoList()) {
        if (cn->isGroup() || cn->isQmlModule())
            generateAnnotatedList(cn, marker, cn->members());
    }
    generateFooter(cn);
}

/*!
  Generate the HTML page for a generic collection. This is usually
  a collection of C++ elements that are related to an element in
  a different module.
 */
void HtmlGenerator::generateGenericCollectionPage(CollectionNode *cn, CodeMarker *marker)
{
    SubTitleSize subTitleSize = LargeSubTitle;
    QString fullTitle = cn->name();

    generateHeader(fullTitle, cn, marker);
    generateTitle(fullTitle, Text() << cn->subtitle(), subTitleSize, cn, marker);

    Text brief;
    brief << "Each function or type documented here is related to a class or "
          << "namespace that is documented in a different module. The reference "
          << "page for that class or namespace will link to the function or type "
          << "on this page.";
    out() << "<p>";
    generateText(brief, cn, marker);
    out() << "</p>\n";

    const QList<Node *> members = cn->members();
    for (const auto &member : members)
        generateDetailedMember(member, cn, marker);

    generateFooter(cn);
}

/*!
  Returns "html" for this subclass of Generator.
 */
QString HtmlGenerator::fileExtension() const
{
    return "html";
}

/*!
  Output a navigation bar (breadcrumbs) for the html file.
  For API reference pages, items for the navigation bar are (in order):
  \table
    \header \li Item     \li Related configuration variable  \li Notes
    \row    \li home     \li navigation.homepage             \li e.g. 'Qt 6.2'
    \row    \li landing  \li navigation.landingpage          \li Module landing page
    \row    \li types    \li navigation.cppclassespage (C++)\br
                             navigation.qmltypespage (QML)   \li Types only
    \row    \li module   \li n/a (automatic)                 \li Module page if different
                                                                 from previous item
    \row    \li page     \li n/a                             \li Current page title
  \endtable

 For other page types (page nodes) the navigation bar is constructed from home
 page, landing page, and the chain of PageNode::navigationParent() items (if one exists).
 This chain is constructed from the \\list structure on a page or pages defined in
 \c navigation.toctitles configuration variable.

 Finally, if no other navigation data exists for a page but it is a member of a
 single group (using \\ingroup), add that group page to the navigation bar.
 */
void HtmlGenerator::generateNavigationBar(const QString &title, const Node *node,
                                          CodeMarker *marker, const QString &buildversion,
                                          bool tableItems)
{
    if (m_noNavigationBar || node == nullptr)
        return;

    Text navigationbar;

    // Set list item types based on the navigation bar type
    // TODO: Do we still need table items?
    Atom::AtomType itemLeft = tableItems ? Atom::TableItemLeft : Atom::ListItemLeft;
    Atom::AtomType itemRight = tableItems ? Atom::TableItemRight : Atom::ListItemRight;

    // Helper to add an item to navigation bar based on a string link target
    auto addNavItem = [&](const QString &link, const QString &title) {
        navigationbar << Atom(itemLeft) << Atom(Atom::NavLink, link)
                      << Atom(Atom::FormattingLeft, ATOM_FORMATTING_LINK)
                      << Atom(Atom::String, title)
                      << Atom(Atom::FormattingRight, ATOM_FORMATTING_LINK) << Atom(itemRight);
    };

    // Helper to add an item to navigation bar based on a target node
    auto addNavItemNode = [&](const Node *node, const QString &title) {
        navigationbar << Atom(itemLeft);
        addNodeLink(navigationbar, node, title);
        navigationbar << Atom(itemRight);
    };

    // Resolve the associated module (collection) node and its 'state' description
    const auto *moduleNode = m_qdb->getModuleNode(node);
    QString moduleState;
    if (moduleNode && !moduleNode->state().isEmpty())
        moduleState = QStringLiteral(" (%1)").arg(moduleNode->state());

    if (m_hometitle == title)
        return;
    if (!m_homepage.isEmpty())
        addNavItem(m_homepage, m_hometitle);
    if (!m_landingpage.isEmpty() && m_landingtitle != title)
        addNavItem(m_landingpage, m_landingtitle);

    if (node->isClassNode()) {
        if (!m_cppclassespage.isEmpty() && !m_cppclassestitle.isEmpty())
            addNavItem(m_cppclassespage, m_cppclassestitle);
        if (!node->physicalModuleName().isEmpty()) {
            // Add explicit link to the \module page if:
            //   - It's not the C++ classes page that's already added, OR
            //   - It has a \modulestate associated with it
            if (moduleNode && (!moduleState.isEmpty() || moduleNode->title() != m_cppclassespage))
                addNavItemNode(moduleNode, moduleNode->name() + moduleState);
        }
        navigationbar << Atom(itemLeft) << Atom(Atom::String, node->name()) << Atom(itemRight);
    } else if (node->isQmlType()) {
        if (!m_qmltypespage.isEmpty() && !m_qmltypestitle.isEmpty())
            addNavItem(m_qmltypespage, m_qmltypestitle);
        // Add explicit link to the \qmlmodule page if:
        //   - It's not the QML types page that's already added, OR
        //   - It has a \modulestate associated with it
        if (moduleNode && (!moduleState.isEmpty() || moduleNode->title() != m_qmltypespage)) {
            addNavItemNode(moduleNode, moduleNode->name() + moduleState);
        }
        navigationbar << Atom(itemLeft) << Atom(Atom::String, node->name()) << Atom(itemRight);
    } else {
        if (node->isPageNode()) {
            auto currentNode{static_cast<const PageNode*>(node)};
            std::deque<const Node *> navNodes;
            // Cutoff at 16 items in case there's a circular dependency
            qsizetype navItems = 0;
            while (currentNode->navigationParent() && ++navItems < 16) {
                if (std::find(navNodes.cbegin(), navNodes.cend(),
                              currentNode->navigationParent()) == navNodes.cend())
                    navNodes.push_front(currentNode->navigationParent());
                currentNode = currentNode->navigationParent();
            }
            // If no nav. parent was found but the page is a \group member, add a link to the
            // (first) group page.
            if (navNodes.empty()) {
                const QStringList groups = static_cast<const PageNode *>(node)->groupNames();
                for (const auto &groupName : groups) {
                    const auto *groupNode = m_qdb->findNodeByNameAndType(QStringList{groupName}, &Node::isGroup);
                    if (groupNode && !groupNode->title().isEmpty()) {
                        navNodes.push_front(groupNode);
                        break;
                    }
                }
            }
            while (!navNodes.empty()) {
                if (navNodes.front()->isPageNode())
                    addNavItemNode(navNodes.front(), navNodes.front()->title());
                navNodes.pop_front();
            }
        }
        if (!navigationbar.isEmpty()) {
            navigationbar << Atom(itemLeft) << Atom(Atom::String, title) << Atom(itemRight);
        }
    }

    generateText(navigationbar, node, marker);

    if (buildversion.isEmpty())
        return;

    navigationbar.clear();

    if (tableItems) {
        out() << "</tr></table><table class=\"buildversion\"><tr>\n"
              << R"(<td id="buildversion" width="100%" align="right">)";
    } else {
        out() << "<li id=\"buildversion\">";
    }

    // Link buildversion string to navigation.landingpage
    if (!m_landingpage.isEmpty() && m_landingtitle != title) {
        navigationbar << Atom(Atom::NavLink, m_landingpage)
                      << Atom(Atom::FormattingLeft, ATOM_FORMATTING_LINK)
                      << Atom(Atom::String, buildversion)
                      << Atom(Atom::FormattingRight, ATOM_FORMATTING_LINK);
        generateText(navigationbar, node, marker);
    } else {
        out() << buildversion;
    }
    if (tableItems)
        out() << "</td>\n";
    else
        out() << "</li>\n";
}

void HtmlGenerator::generateHeader(const QString &title, const Node *node, CodeMarker *marker)
{
    out() << "<!DOCTYPE html>\n";
    out() << QString("<html lang=\"%1\">\n").arg(naturalLanguage);
    out() << "<head>\n";
    out() << "  <meta charset=\"utf-8\">\n";
    if (node && !node->doc().location().isEmpty())
        out() << "<!-- " << node->doc().location().fileName() << " -->\n";

    if (node && !node->doc().briefText().isEmpty()) {
        out() << "  <meta name=\"description\" content=\""
              << protectEnc(node->doc().briefText().toString())
              << "\">\n";
    }

    // determine the rest of the <title> element content: "title | titleSuffix version"
    QString titleSuffix;
    if (!m_landingtitle.isEmpty()) {
        // for normal pages: "title | landingtitle version"
        titleSuffix = m_landingtitle;
    } else if (!m_hometitle.isEmpty()) {
        // for pages that set the homepage title but not landing page title:
        // "title | hometitle version"
        if (title != m_hometitle)
            titleSuffix = m_hometitle;
    } else {
        // "title | productname version"
        titleSuffix = m_productName.isEmpty() ? m_project : m_productName;
    }
    if (title == titleSuffix)
        titleSuffix.clear();

    out() << "  <title>";
    if (!titleSuffix.isEmpty() && !title.isEmpty()) {
        out() << "%1 | %2"_L1.arg(protectEnc(title), titleSuffix);
    } else {
        out() << protectEnc(title);
    }

    // append a full version to the suffix if neither suffix nor title
    // include (a prefix of) version information
    QVersionNumber projectVersion = QVersionNumber::fromString(m_qdb->version());
    if (!projectVersion.isNull()) {
        QVersionNumber titleVersion;
        static const QRegularExpression re(QLatin1String(R"(\d+\.\d+)"));
        const QString &versionedTitle = titleSuffix.isEmpty() ? title : titleSuffix;
        auto match = re.match(versionedTitle);
        if (match.hasMatch())
            titleVersion = QVersionNumber::fromString(match.captured());
        if (titleVersion.isNull() || !titleVersion.isPrefixOf(projectVersion)) {
            // Prefix with product name if one exists
            if (!m_productName.isEmpty() && titleSuffix != m_productName)
                out() << " | %1"_L1.arg(m_productName);
            out() << " %1"_L1.arg(projectVersion.toString());
        }
    }
    out() << "</title>\n";

    // Include style sheet and script links.
    out() << m_headerStyles;
    out() << m_headerScripts;
    if (m_endHeader.isEmpty())
        out() << "</head>\n<body>\n";
    else
        out() << m_endHeader;

    out() << QString(m_postHeader).replace("\\" + COMMAND_VERSION, m_qdb->version());
    bool usingTable = m_postHeader.trimmed().endsWith(QLatin1String("<tr>"));
    generateNavigationBar(title, node, marker, m_buildversion, usingTable);
    out() << QString(m_postPostHeader).replace("\\" + COMMAND_VERSION, m_qdb->version());

    m_navigationLinks.clear();
    refMap.clear();

    if (node && !node->links().empty()) {
        std::pair<QString, QString> linkPair;
        std::pair<QString, QString> anchorPair;
        const Node *linkNode;
        bool useSeparator = false;

        if (node->links().contains(Node::PreviousLink)) {
            linkPair = node->links()[Node::PreviousLink];
            linkNode = m_qdb->findNodeForTarget(linkPair.first, node);
            if (linkNode == nullptr && !noLinkErrors())
                node->doc().location().warning(
                        QStringLiteral("Cannot link to '%1'").arg(linkPair.first));
            if (linkNode == nullptr || linkNode == node)
                anchorPair = linkPair;
            else
                anchorPair = anchorForNode(linkNode);

            out() << R"(  <link rel="prev" href=")" << anchorPair.first << "\" />\n";

            m_navigationLinks += R"(<a class="prevPage" href=")" + anchorPair.first + "\">";
            if (linkPair.first == linkPair.second && !anchorPair.second.isEmpty())
                m_navigationLinks += protect(anchorPair.second);
            else
                m_navigationLinks += protect(linkPair.second);
            m_navigationLinks += "</a>\n";
            useSeparator = !m_navigationSeparator.isEmpty();
        }
        if (node->links().contains(Node::NextLink)) {
            linkPair = node->links()[Node::NextLink];
            linkNode = m_qdb->findNodeForTarget(linkPair.first, node);
            if (linkNode == nullptr && !noLinkErrors())
                node->doc().location().warning(
                        QStringLiteral("Cannot link to '%1'").arg(linkPair.first));
            if (linkNode == nullptr || linkNode == node)
                anchorPair = linkPair;
            else
                anchorPair = anchorForNode(linkNode);

            out() << R"(  <link rel="next" href=")" << anchorPair.first << "\" />\n";

            if (useSeparator)
                m_navigationLinks += m_navigationSeparator;

            m_navigationLinks += R"(<a class="nextPage" href=")" + anchorPair.first + "\">";
            if (linkPair.first == linkPair.second && !anchorPair.second.isEmpty())
                m_navigationLinks += protect(anchorPair.second);
            else
                m_navigationLinks += protect(linkPair.second);
            m_navigationLinks += "</a>\n";
        }
        if (node->links().contains(Node::StartLink)) {
            linkPair = node->links()[Node::StartLink];
            linkNode = m_qdb->findNodeForTarget(linkPair.first, node);
            if (linkNode == nullptr && !noLinkErrors())
                node->doc().location().warning(
                        QStringLiteral("Cannot link to '%1'").arg(linkPair.first));
            if (linkNode == nullptr || linkNode == node)
                anchorPair = std::move(linkPair);
            else
                anchorPair = anchorForNode(linkNode);
            out() << R"(  <link rel="start" href=")" << anchorPair.first << "\" />\n";
        }
    }

    if (node && !node->links().empty())
        out() << "<p class=\"naviNextPrevious headerNavi\">\n" << m_navigationLinks << "</p>\n";
}

void HtmlGenerator::generateTitle(const QString &title, const Text &subtitle,
                                  SubTitleSize subTitleSize, const Node *relative,
                                  CodeMarker *marker)
{
    out() << QString(m_prologue).replace("\\" + COMMAND_VERSION, m_qdb->version());
    QString attribute;
    if (isApiGenus(relative->genus()))
        attribute = R"( translate="no")";

    if (!title.isEmpty())
        out() << "<h1 class=\"title\"" << attribute << ">" << protectEnc(title) << "</h1>\n";
    if (!subtitle.isEmpty()) {
        out() << "<span";
        if (subTitleSize == SmallSubTitle)
            out() << " class=\"small-subtitle\"" << attribute << ">";
        else
            out() << " class=\"subtitle\"" << attribute << ">";
        generateText(subtitle, relative, marker);
        out() << "</span>\n";
    }
}

void HtmlGenerator::generateFooter(const Node *node)
{
    if (node && !node->links().empty())
        out() << "<p class=\"naviNextPrevious footerNavi\">\n" << m_navigationLinks << "</p>\n";

    out() << QString(m_footer).replace("\\" + COMMAND_VERSION, m_qdb->version())
          << QString(m_address).replace("\\" + COMMAND_VERSION, m_qdb->version());

    out() << "</body>\n";
    out() << "</html>\n";
}

/*!
    Lists the required imports and includes in a table.
    The number of rows is known.
*/
void HtmlGenerator::generateRequisites(Aggregate *aggregate, CodeMarker *marker)
{
    QMap<QString, Text> requisites;
    Text text;

    const QString headerText = "Header";
    const QString sinceText = "Since";
    const QString inheritedByText = "Inherited By";
    const QString inheritsText = "Inherits";
    const QString nativeTypeText = "In QML";
    const QString qtVariableText = "qmake";
    const QString cmakeText = "CMake";
    const QString statusText = "Status";

    // The order of the requisites matter
    const QStringList requisiteorder { headerText,         cmakeText,    qtVariableText,  sinceText,
                                       nativeTypeText, inheritsText, inheritedByText, statusText };

    addIncludeFileToMap(aggregate, requisites, text, headerText);
    addSinceToMap(aggregate, requisites, &text, sinceText);

    if (aggregate->isClassNode() || aggregate->isNamespace()) {
        addCMakeInfoToMap(aggregate, requisites, &text, cmakeText);
        addQtVariableToMap(aggregate, requisites, &text, qtVariableText);
    }

    if (aggregate->isClassNode()) {
        auto *classe = dynamic_cast<ClassNode *>(aggregate);
        if (classe && classe->isQmlNativeType() && !classe->isInternal())
            addQmlNativeTypesToMap(requisites, &text, nativeTypeText, classe);

        addInheritsToMap(requisites, &text, inheritsText, classe);
        addInheritedByToMap(requisites, &text, inheritedByText, classe);
    }

    // Add the state description (if any) to the map
    addStatusToMap(aggregate, requisites, text, statusText);

    if (!requisites.isEmpty()) {
        // generate the table
        generateTheTable(requisiteorder, requisites, aggregate, marker);
    }
}

/*!
 * \internal
 */
void HtmlGenerator::generateTheTable(const QStringList &requisiteOrder,
                                     const QMap<QString, Text> &requisites,
                                     const Aggregate *aggregate, CodeMarker *marker)
{
    out() << "<div class=\"table\"><table class=\"alignedsummary requisites\" translate=\"no\">\n";

    for (auto it = requisiteOrder.constBegin(); it != requisiteOrder.constEnd(); ++it) {

        if (requisites.contains(*it)) {
            out() << "<tr>"
                  << "<td class=\"memItemLeft rightAlign topAlign\"> " << *it
                  << ":"
                     "</td><td class=\"memItemRight bottomAlign\"> ";

            generateText(requisites.value(*it), aggregate, marker);
            out() << "</td></tr>\n";
        }
    }
    out() << "</table></div>\n";
}

/*!
 * \internal
 * Adds inherited by information to the map.
 */
void HtmlGenerator::addInheritedByToMap(QMap<QString, Text> &requisites, Text *text,
                                        const QString &inheritedByText, ClassNode *classe)
{
    if (!classe->derivedClasses().isEmpty()) {
        text->clear();
        *text << Atom::ParaLeft;
        int count = appendSortedNames(*text, classe, classe->derivedClasses());
        *text << Atom::ParaRight;
        if (count > 0)
            requisites.insert(inheritedByText, *text);
    }
}

/*!
 * \internal
 * Adds base classes to the map.
 */
void HtmlGenerator::addInheritsToMap(QMap<QString, Text> &requisites, Text *text,
                                     const QString &inheritsText, ClassNode *classe)
{
    if (!classe->baseClasses().isEmpty()) {
        int index = 0;
        text->clear();
        const auto baseClasses = classe->baseClasses();
        for (const auto &cls : baseClasses) {
            if (cls.m_node) {
                appendFullName(*text, cls.m_node, classe);

                if (cls.m_access == Access::Protected) {
                    *text << " (protected)";
                } else if (cls.m_access == Access::Private) {
                    *text << " (private)";
                }
                *text << Utilities::comma(index++, classe->baseClasses().size());
            }
        }
        *text << Atom::ParaRight;
        if (index > 0)
            requisites.insert(inheritsText, *text);
    }
}

/*!
    \internal
    Add the QML/C++ native type information to the map.
 */
void HtmlGenerator::addQmlNativeTypesToMap(QMap<QString, Text> &requisites, Text *text,
                                            const QString &nativeTypeText, ClassNode *classe) const
{
    if (!text)
        return;

    text->clear();

    QList<QmlTypeNode *> nativeTypes { classe->qmlNativeTypes().cbegin(), classe->qmlNativeTypes().cend()};
    std::sort(nativeTypes.begin(), nativeTypes.end(), Node::nodeNameLessThan);
    qsizetype index { 0 };

    for (const auto &item : std::as_const(nativeTypes)) {
        addNodeLink(*text, item);
        *text << Utilities::comma(index++, nativeTypes.size());
    }
    requisites.insert(nativeTypeText, *text);
}

/*!
 * \internal
 *  Adds the CMake package and link library information to the map.
 */
void HtmlGenerator::addCMakeInfoToMap(const Aggregate *aggregate, QMap<QString, Text> &requisites,
                                      Text *text, const QString &CMakeInfo) const
{
    if (!aggregate->physicalModuleName().isEmpty() && text != nullptr) {
        const CollectionNode *cn =
                m_qdb->getCollectionNode(aggregate->physicalModuleName(), NodeType::Module);

        const auto result = cmakeRequisite(cn);

        if (!result) {
            return;
        }

        text->clear();

        const Atom lineBreak = Atom(Atom::RawString, "<br/>\n");

        *text << openCodeTag << result->first << closeCodeTag << lineBreak
              << openCodeTag << result->second << closeCodeTag;

        requisites.insert(CMakeInfo, *text);
    }
}

/*!
 * \internal
 * Adds the Qt variable (from the \\qtvariable command) to the map.
 */
void HtmlGenerator::addQtVariableToMap(const Aggregate *aggregate, QMap<QString, Text> &requisites,
                                       Text *text, const QString &qtVariableText) const
{
    if (!aggregate->physicalModuleName().isEmpty()) {
        const CollectionNode *cn =
                m_qdb->getCollectionNode(aggregate->physicalModuleName(), NodeType::Module);

        if (cn && !cn->qtVariable().isEmpty()) {
            text->clear();
            *text << openCodeTag << "QT += " + cn->qtVariable() << closeCodeTag;
            requisites.insert(qtVariableText, *text);
        }
    }
}

/*!
 * \internal
 * Adds the since information (from the \\since command) to the map.
 *
 */
void HtmlGenerator::addSinceToMap(const Aggregate *aggregate, QMap<QString, Text> &requisites,
                                  Text *text, const QString &sinceText) const
{
    if (!aggregate->since().isEmpty() && text != nullptr) {
        text->clear();
        *text << formatSince(aggregate) << Atom::ParaRight;
        requisites.insert(sinceText, *text);
    }
}

/*!
 * \internal
 * Adds the status description for \a aggregate, together with a <span> element, to the \a
 * requisites map.
 *
 * The span element can be used for adding CSS styling/icon associated with a specific status.
 * The span class name is constructed by converting the description (sans \\deprecated
 * version info) to lowercase and replacing all non-alphanum characters with hyphens. In
 * addition, the span has a class \c status. For example,
 * 'Tech Preview' -> class="status tech-preview"
*/
void HtmlGenerator::addStatusToMap(const Aggregate *aggregate, QMap<QString, Text> &requisites,
                                  Text &text, const QString &statusText) const
{
    auto status{formatStatus(aggregate, m_qdb)};
    if (!status)
        return;

    QString spanClass;
    if (aggregate->status() == Node::Deprecated)
        spanClass = u"deprecated"_s; // Disregard any version info
    else
        spanClass = Utilities::asAsciiPrintable(status.value());

    text.clear();
    text << Atom(Atom::String, status.value())
         << Atom(Atom::FormattingLeft, ATOM_FORMATTING_SPAN +
                 "class=\"status %1\""_L1.arg(spanClass))
         << Atom(Atom::FormattingRight, ATOM_FORMATTING_SPAN);
    requisites.insert(statusText, text);
}

/*!
 * \internal
 * Adds the include file (resolved automatically or set with the
 * \\inheaderfile command) to the map.
 */
void HtmlGenerator::addIncludeFileToMap(const Aggregate *aggregate,
                                         QMap<QString, Text> &requisites, Text& text,
                                         const QString &headerText)
{
    if (aggregate->includeFile()) {
        text.clear();
        text << openCodeTag << "#include <%1>"_L1.arg(*aggregate->includeFile()) << closeCodeTag;
        requisites.insert(headerText, text);
    }
}

/*!
    Lists the required imports and includes in a table.
    The number of rows is known.
*/
void HtmlGenerator::generateQmlRequisites(QmlTypeNode *qcn, CodeMarker *marker)
{
    if (qcn == nullptr)
        return;

    QMap<QString, Text> requisites;
    Text text;

    const QString importText = "Import Statement";
    const QString sinceText = "Since";
    const QString inheritedByText = "Inherited By";
    const QString inheritsText = "Inherits";
    const QString nativeTypeText = "In C++";
    const QString statusText = "Status";

    // add the module name and version to the map
    QString logicalModuleVersion;
    const CollectionNode *collection = qcn->logicalModule();

    // skip import statement of \internal collections
    if (!qcn->logicalModuleName().isEmpty() && (!collection || !collection->isInternal() || m_showInternal)) {
        QStringList parts = QStringList() << "import" << qcn->logicalModuleName() << qcn->logicalModuleVersion();
        text.clear();
        text << openCodeTag << parts.join(' ').trimmed() << closeCodeTag;
        requisites.insert(importText, text);
    } else if (!qcn->isQmlBasicType() && qcn->logicalModuleName().isEmpty()) {
        qcn->doc().location().warning(QStringLiteral("Could not resolve QML import statement for type '%1'").arg(qcn->name()),
                                      QStringLiteral("Maybe you forgot to use the '\\%1' command?").arg(COMMAND_INQMLMODULE));
    }

    // add the since and project into the map
    if (!qcn->since().isEmpty()) {
        text.clear();
        text << formatSince(qcn) << Atom::ParaRight;
        requisites.insert(sinceText, text);
    }

    // add the native type to the map
    if (ClassNode *cn = qcn->classNode(); cn && cn->isQmlNativeType() && !cn->isInternal()) {
        text.clear();
        addNodeLink(text, cn);
        requisites.insert(nativeTypeText, text);
    }

    // add the inherits to the map
    QmlTypeNode *base = qcn->qmlBaseNode();
    NodeList subs;
    QmlTypeNode::subclasses(qcn, subs);
    QStringList knownTypeNames{qcn->name()};

    while (base && base->isInternal()) {
        base = base->qmlBaseNode();
    }
    if (base) {
        knownTypeNames << base->name();
        text.clear();
        text << Atom::ParaLeft;
        addNodeLink(text, base);

        // Disambiguate with '(<QML module name>)' if there are clashing type names
        for (const auto sub : std::as_const(subs)) {
            if (knownTypeNames.contains(sub->name())) {
                text << Atom(Atom::String, " (%1)"_L1.arg(base->logicalModuleName()));
                break;
            }
        }
        text << Atom::ParaRight;
        requisites.insert(inheritsText, text);
    }

    // add the inherited-by to the map
    if (!subs.isEmpty()) {
        text.clear();
        text << Atom::ParaLeft;
        int count = appendSortedQmlNames(text, qcn, knownTypeNames, subs);
        text << Atom::ParaRight;
        if (count > 0)
            requisites.insert(inheritedByText, text);
    }

    // Add the state description (if any) to the map
    addStatusToMap(qcn, requisites, text, statusText);

    // The order of the requisites matter
    const QStringList requisiteorder {std::move(importText), std::move(sinceText),
                                      std::move(nativeTypeText), std::move(inheritsText),
                                      std::move(inheritedByText), std::move(statusText)};

    if (!requisites.isEmpty())
        generateTheTable(requisiteorder, requisites, qcn, marker);
}

void HtmlGenerator::generateBrief(const Node *node, CodeMarker *marker, const Node *relative,
                                  bool addLink)
{
    Text brief = node->doc().briefText();

    if (!brief.isEmpty()) {
        if (!brief.lastAtom()->string().endsWith('.')) {
            brief << Atom(Atom::String, ".");
            node->doc().location().warning(
                    QStringLiteral("'\\brief' statement does not end with a full stop."));
        }
        generateExtractionMark(node, BriefMark);
        out() << "<p>";
        generateText(brief, node, marker);

        if (addLink) {
            if (!relative || node == relative)
                out() << " <a href=\"#";
            else
                out() << " <a href=\"" << linkForNode(node, relative) << '#';
            out() << registerRef("details") << "\">More...</a>";
        }

        out() << "</p>\n";
        generateExtractionMark(node, EndMark);
    }
}

/*!
  Revised for the new doc format.
  Generates a table of contents beginning at \a node.
 */
void HtmlGenerator::generateTableOfContents(const Node *node, CodeMarker *marker,
                                            QList<Section> *sections)
{
    QList<Atom *> toc;
    if (node->doc().hasTableOfContents())
        toc = node->doc().tableOfContents();
    if (tocDepth == 0 || (toc.isEmpty() && !sections && !node->isModule())) {
        generateSidebar();
        return;
    }

    int sectionNumber = 1;
    int detailsBase = 0;

    // disable nested links in table of contents
    m_inContents = true;

    out() << "<div class=\"sidebar\">\n";
    out() << "<div class=\"toc\">\n";
    out() << "<h3 id=\"toc\">Contents</h3>\n";

    if (node->isModule()) {
        openUnorderedList();
        if (!static_cast<const CollectionNode *>(node)->noAutoList()) {
            if (node->hasNamespaces()) {
                out() << "<li class=\"level" << sectionNumber << "\"><a href=\"#"
                      << registerRef("namespaces") << "\">Namespaces</a></li>\n";
            }
            if (node->hasClasses()) {
                out() << "<li class=\"level" << sectionNumber << "\"><a href=\"#"
                      << registerRef("classes") << "\">Classes</a></li>\n";
            }
        }
        out() << "<li class=\"level" << sectionNumber << "\"><a href=\"#" << registerRef("details")
              << "\">Detailed Description</a></li>\n";
        for (const auto &entry : std::as_const(toc)) {
            if (entry->string().toInt() == 1) {
                detailsBase = 1;
                break;
            }
        }
    } else if (sections && (node->isClassNode() || node->isNamespace() || node->isQmlType())) {
        for (const auto &section : std::as_const(*sections)) {
            if (!section.members().isEmpty()) {
                openUnorderedList();
                out() << "<li class=\"level" << sectionNumber << "\"><a href=\"#"
                      << registerRef(section.plural()) << "\">" << section.title() << "</a></li>\n";
            }
            if (!section.reimplementedMembers().isEmpty()) {
                openUnorderedList();
                QString ref = QString("Reimplemented ") + section.plural();
                out() << "<li class=\"level" << sectionNumber << "\"><a href=\"#"
                      << registerRef(ref.toLower()) << "\">"
                      << QString("Reimplemented ") + section.title() << "</a></li>\n";
            }
        }
        if (!node->isNamespace() || node->hasDoc()) {
            openUnorderedList();
            out() << "<li class=\"level" << sectionNumber << "\"><a href=\"#"
                  << registerRef("details") << "\">Detailed Description</a></li>\n";
        }
        for (const auto &entry : toc) {
            if (entry->string().toInt() == 1) {
                detailsBase = 1;
                break;
            }
        }
    }

    for (const auto &atom : toc) {
        sectionNumber = atom->string().toInt() + detailsBase;
        // restrict the ToC depth to the one set by the HTML.tocdepth variable or
        // print all levels if tocDepth is not set.
        if (sectionNumber <= tocDepth || tocDepth < 0) {
            openUnorderedList();
            int numAtoms;
            Text headingText = Text::sectionHeading(atom);
            out() << "<li class=\"level" << sectionNumber << "\">";
            out() << "<a href=\"" << '#' << Tree::refForAtom(atom) << "\">";
            generateAtomList(headingText.firstAtom(), node, marker, true, numAtoms);
            out() << "</a></li>\n";
        }
    }
    closeUnorderedList();
    out() << "</div>\n";
    out() << R"(<div class="sidebar-content" id="sidebar-content"></div>)";
    out() << "</div>\n";
    m_inContents = false;
    m_inLink = false;
}

/*!
  Outputs a placeholder div where the style can add customized sidebar content.
 */
void HtmlGenerator::generateSidebar()
{
    out() << "<div class=\"sidebar\">";
    out() << R"(<div class="sidebar-content" id="sidebar-content"></div>)";
    out() << "</div>\n";
}

QString HtmlGenerator::generateAllMembersFile(const Section &section, CodeMarker *marker)
{
    if (section.isEmpty())
        return QString();

    const Aggregate *aggregate = section.aggregate();
    QString fileName = fileBase(aggregate) + "-members." + fileExtension();
    beginSubPage(aggregate, fileName);
    QString title = "List of All Members for " + aggregate->plainFullName();
    generateHeader(title, aggregate, marker);
    generateSidebar();
    generateTitle(title, Text(), SmallSubTitle, aggregate, marker);
    out() << "<p>This is the complete list of members for ";
    generateFullName(aggregate, nullptr);
    out() << ", including inherited members.</p>\n";

    generateSectionList(section, aggregate, marker);

    generateFooter();
    endSubPage();
    return fileName;
}

/*!
  This function creates an html page on which are listed all
  the members of the QML class used to generte the \a sections,
  including the inherited members. The \a marker is used for
  formatting stuff.
 */
QString HtmlGenerator::generateAllQmlMembersFile(const Sections &sections, CodeMarker *marker)
{

    if (sections.allMembersSection().isEmpty())
        return QString();

    const Aggregate *aggregate = sections.aggregate();
    QString fileName = fileBase(aggregate) + "-members." + fileExtension();
    beginSubPage(aggregate, fileName);
    QString title = "List of All Members for " + aggregate->name();
    generateHeader(title, aggregate, marker);
    generateSidebar();
    generateTitle(title, Text(), SmallSubTitle, aggregate, marker);
    out() << "<p>This is the complete list of members for ";
    generateFullName(aggregate, nullptr);
    out() << ", including inherited members.</p>\n";

    ClassNodesList &cknl = sections.allMembersSection().classNodesList();
    for (int i = 0; i < cknl.size(); i++) {
        ClassNodes ckn = cknl[i];
        const QmlTypeNode *qcn = ckn.first;
        NodeVector &nodes = ckn.second;
        if (nodes.isEmpty())
            continue;
        if (i != 0) {
            out() << "<p>The following members are inherited from ";
            generateFullName(qcn, nullptr);
            out() << ".</p>\n";
        }
        openUnorderedList();
        for (int j = 0; j < nodes.size(); j++) {
            Node *node = nodes[j];
            if (node->access() == Access::Private || node->isInternal())
                continue;
            if (node->isSharingComment() && node->sharedCommentNode()->isPropertyGroup())
                continue;

            std::function<void(Node *)> generate = [&](Node *n) {
                out() << "<li class=\"fn\" translate=\"no\">";
                generateQmlItem(n, aggregate, marker, true);
                if (n->isDefault())
                    out() << " [default]";
                else if (n->isAttached())
                    out() << " [attached]";
                // Indent property group members
                if (n->isPropertyGroup()) {
                    out() << "<ul>\n";
                    const QList<Node *> &collective =
                            static_cast<SharedCommentNode *>(n)->collective();
                    std::for_each(collective.begin(), collective.end(), generate);
                    out() << "</ul>\n";
                }
                out() << "</li>\n";
            };
            generate(node);
        }
        closeUnorderedList();
    }


    generateFooter();
    endSubPage();
    return fileName;
}

QString HtmlGenerator::generateObsoleteMembersFile(const Sections &sections, CodeMarker *marker)
{
    SectionPtrVector summary_spv;
    SectionPtrVector details_spv;
    if (!sections.hasObsoleteMembers(&summary_spv, &details_spv))
        return QString();

    Aggregate *aggregate = sections.aggregate();
    QString title = "Obsolete Members for " + aggregate->plainFullName();
    QString fileName = fileBase(aggregate) + "-obsolete." + fileExtension();

    beginSubPage(aggregate, fileName);
    generateHeader(title, aggregate, marker);
    generateSidebar();
    generateTitle(title, Text(), SmallSubTitle, aggregate, marker);

    out() << "<p><b>The following members of class "
          << "<a href=\"" << linkForNode(aggregate, nullptr) << "\" translate=\"no\">"
          << protectEnc(aggregate->name()) << "</a>"
          << " are deprecated.</b> "
          << "They are provided to keep old source code working. "
          << "We strongly advise against using them in new code.</p>\n";

    for (const auto &section : summary_spv) {
        out() << "<h2>" << protectEnc(section->title()) << "</h2>\n";
        generateSectionList(*section, aggregate, marker, true);
    }

    for (const auto &section : details_spv) {
        out() << "<h2>" << protectEnc(section->title()) << "</h2>\n";

        const NodeVector &members = section->obsoleteMembers();
        for (const auto &member : members) {
            if (member->access() != Access::Private)
                generateDetailedMember(member, aggregate, marker);
        }
    }

    generateFooter();
    endSubPage();
    return fileName;
}

/*!
  Generates a separate file where deprecated members of the QML
  type \a qcn are listed. The \a marker is used to generate
  the section lists, which are then traversed and output here.
 */
QString HtmlGenerator::generateObsoleteQmlMembersFile(const Sections &sections, CodeMarker *marker)
{
    SectionPtrVector summary_spv;
    SectionPtrVector details_spv;
    if (!sections.hasObsoleteMembers(&summary_spv, &details_spv))
        return QString();

    Aggregate *aggregate = sections.aggregate();
    QString title = "Obsolete Members for " + aggregate->name();
    QString fileName = fileBase(aggregate) + "-obsolete." + fileExtension();

    beginSubPage(aggregate, fileName);
    generateHeader(title, aggregate, marker);
    generateSidebar();
    generateTitle(title, Text(), SmallSubTitle, aggregate, marker);

    out() << "<p><b>The following members of QML type "
          << "<a href=\"" << linkForNode(aggregate, nullptr) << "\">"
          << protectEnc(aggregate->name()) << "</a>"
          << " are deprecated.</b> "
          << "They are provided to keep old source code working. "
          << "We strongly advise against using them in new code.</p>\n";

    for (const auto &section : summary_spv) {
        QString ref = registerRef(section->title().toLower());
        out() << "<h2 id=\"" << ref << "\">" << protectEnc(section->title()) << "</h2>\n";
        generateQmlSummary(section->obsoleteMembers(), aggregate, marker);
    }

    for (const auto &section : details_spv) {
        out() << "<h2>" << protectEnc(section->title()) << "</h2>\n";
        const NodeVector &members = section->obsoleteMembers();
        for (const auto &member : members) {
            generateDetailedQmlMember(member, aggregate, marker);
            out() << "<br/>\n";
        }
    }

    generateFooter();
    endSubPage();
    return fileName;
}

void HtmlGenerator::generateClassHierarchy(const Node *relative, NodeMultiMap &classMap)
{
    if (classMap.isEmpty())
        return;

    NodeMap topLevel;
    for (const auto &it : classMap) {
        auto *classe = static_cast<ClassNode *>(it);
        if (classe->baseClasses().isEmpty())
            topLevel.insert(classe->name(), classe);
    }

    QStack<NodeMap> stack;
    stack.push(topLevel);

    out() << "<ul>\n";
    while (!stack.isEmpty()) {
        if (stack.top().isEmpty()) {
            stack.pop();
            out() << "</ul>\n";
        } else {
            ClassNode *child = static_cast<ClassNode *>(*stack.top().begin());
            out() << "<li>";
            generateFullName(child, relative);
            out() << "</li>\n";
            stack.top().erase(stack.top().begin());

            NodeMap newTop;
            const auto derivedClasses = child->derivedClasses();
            for (const RelatedClass &d : derivedClasses) {
                if (d.m_node && d.m_node->isInAPI())
                    newTop.insert(d.m_node->name(), d.m_node);
            }
            if (!newTop.isEmpty()) {
                stack.push(newTop);
                out() << "<ul>\n";
            }
        }
    }
}

/*!
  Outputs an annotated list of the nodes in \a unsortedNodes.
  A two-column table is output.
 */
void HtmlGenerator::generateAnnotatedList(const Node *relative, CodeMarker *marker,
                                          const NodeList &unsortedNodes, Qt::SortOrder sortOrder)
{
    if (unsortedNodes.isEmpty() || relative == nullptr)
        return;

    NodeMultiMap nmm;
    bool allInternal = true;
    for (auto *node : unsortedNodes) {
        if (!node->isInternal() && !node->isDeprecated()) {
            allInternal = false;
            nmm.insert(node->fullName(relative), node);
        }
    }
    if (allInternal)
        return;
    out() << "<div class=\"table\"><table class=\"annotated\">\n";
    int row = 0;
    NodeList nodes = nmm.values();

    if (sortOrder == Qt::DescendingOrder)
        std::sort(nodes.rbegin(), nodes.rend(), Node::nodeSortKeyOrNameLessThan);
    else
        std::sort(nodes.begin(), nodes.end(), Node::nodeSortKeyOrNameLessThan);

    for (const auto *node : std::as_const(nodes)) {
        if (++row % 2 == 1)
            out() << "<tr class=\"odd topAlign\">";
        else
            out() << "<tr class=\"even topAlign\">";
        out() << "<td class=\"tblName\" translate=\"no\"><p>";
        generateFullName(node, relative);
        out() << "</p></td>";

        if (!node->isTextPageNode()) {
            Text brief = node->doc().trimmedBriefText(node->name());
            if (!brief.isEmpty()) {
                out() << "<td class=\"tblDescr\"><p>";
                generateText(brief, node, marker);
                out() << "</p></td>";
            } else if (!node->reconstitutedBrief().isEmpty()) {
                out() << "<td class=\"tblDescr\"><p>";
                out() << node->reconstitutedBrief();
                out() << "</p></td>";
            }
        } else {
            out() << "<td class=\"tblDescr\"><p>";
            if (!node->reconstitutedBrief().isEmpty()) {
                out() << node->reconstitutedBrief();
            } else
                out() << protectEnc(node->doc().briefText().toString());
            out() << "</p></td>";
        }
        out() << "</tr>\n";
    }
    out() << "</table></div>\n";
}

/*!
  Outputs a series of annotated lists from the nodes in \a nmm,
  divided into sections based by the key names in the multimap.
 */
void HtmlGenerator::generateAnnotatedLists(const Node *relative, CodeMarker *marker,
                                           const NodeMultiMap &nmm)
{
    const auto &uniqueKeys = nmm.uniqueKeys();
    for (const QString &name : uniqueKeys) {
        if (!name.isEmpty()) {
            out() << "<h2 id=\"" << registerRef(name.toLower()) << "\">" << protectEnc(name)
                  << "</h2>\n";
        }
        generateAnnotatedList(relative, marker, nmm.values(name));
    }
}

/*!
  This function finds the common prefix of the names of all
  the classes in the class map \a nmm and then generates a
  compact list of the class names alphabetized on the part
  of the name not including the common prefix. You can tell
  the function to use \a commonPrefix as the common prefix,
  but normally you let it figure it out itself by looking at
  the name of the first and last classes in the class map
  \a nmm.
 */
void HtmlGenerator::generateCompactList(ListType listType, const Node *relative,
                                        const NodeMultiMap &nmm, bool includeAlphabet,
                                        const QString &commonPrefix)
{
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
    qsizetype paragraphOffset[NumParagraphs + 1]; // 37 + 1
    paragraphOffset[0] = 0;
    for (int i = 0; i < NumParagraphs; i++) // i = 0..36
        paragraphOffset[i + 1] = paragraphOffset[i] + paragraph[i].size();

    /*
      Output the alphabet as a row of links.
     */
    if (includeAlphabet) {
        out() << "<p  class=\"centerAlign functionIndex\" translate=\"no\"><b>";
        for (int i = 0; i < 26; i++) {
            QChar ch('a' + i);
            if (usedParagraphNames.contains(char('a' + i)))
                out() << QString("<a href=\"#%1\">%2</a>&nbsp;").arg(ch).arg(ch.toUpper());
        }
        out() << "</b></p>\n";
    }

    /*
      Output a <div> element to contain all the <dl> elements.
     */
    out() << "<div class=\"flowListDiv\" translate=\"no\">\n";
    m_numTableRows = 0;

    // Build a map of all duplicate names across the entire list
    QHash<QString, int> nameOccurrences;
    for (const auto &[key, node] : nmm.asKeyValueRange()) {
        QStringList pieces{node->fullName(relative).split("::"_L1)};
        const QString &name{pieces.last()};
        nameOccurrences[name]++;
    }

    int curParNr = 0;
    int curParOffset = 0;

    for (int i = 0; i < nmm.size(); i++) {
        while ((curParNr < NumParagraphs) && (curParOffset == paragraph[curParNr].size())) {
            ++curParNr;
            curParOffset = 0;
        }

        /*
          Starting a new paragraph means starting a new <dl>.
        */
        if (curParOffset == 0) {
            if (i > 0)
                out() << "</dl>\n";
            if (++m_numTableRows % 2 == 1)
                out() << "<dl class=\"flowList odd\">";
            else
                out() << "<dl class=\"flowList even\">";
            out() << "<dt class=\"alphaChar\"";
            if (includeAlphabet)
                out() << QString(" id=\"%1\"").arg(paragraphName[curParNr][0].toLower());
            out() << "><b>" << paragraphName[curParNr] << "</b></dt>\n";
        }

        /*
          Output a <dd> for the current offset in the current paragraph.
         */
        out() << "<dd>";
        if ((curParNr < NumParagraphs) && !paragraphName[curParNr].isEmpty()) {
            NodeMultiMap::Iterator it;
            NodeMultiMap::Iterator next;
            it = paragraph[curParNr].begin();
            for (int j = 0; j < curParOffset; j++)
                ++it;

            if (listType == Generic) {
                /*
                  Previously, we used generateFullName() for this, but we
                  require some special formatting.
                */
                out() << "<a href=\"" << linkForNode(it.value(), relative) << "\">";
            } else if (listType == Obsolete) {
                QString fileName = fileBase(it.value()) + "-obsolete." + fileExtension();
                QString link;
                if (useOutputSubdirs())
                    link = "../%1/"_L1.arg(it.value()->tree()->physicalModuleName());
                link += fileName;
                out() << "<a href=\"" << link << "\">";
            }

            QStringList pieces{it.value()->fullName(relative).split("::"_L1)};
            const auto &name{pieces.last()};

            // Add module disambiguation if there are multiple types with the same name
            if (nameOccurrences[name] > 1) {
                const QString moduleName = it.value()->isQmlNode() ? it.value()->logicalModuleName()
                                                                   : it.value()->tree()->camelCaseModuleName();
                pieces.last().append(": %1"_L1.arg(moduleName));
            }

            out() << protectEnc(pieces.last());
            out() << "</a>";
            if (pieces.size() > 1) {
                out() << " (";
                generateFullName(it.value()->parent(), relative);
                out() << ')';
            }
        }
        out() << "</dd>\n";
        curParOffset++;
    }
    if (nmm.size() > 0)
        out() << "</dl>\n";

    out() << "</div>\n";
}

void HtmlGenerator::generateFunctionIndex(const Node *relative)
{
    out() << "<p  class=\"centerAlign functionIndex\" translate=\"no\"><b>";
    for (int i = 0; i < 26; i++) {
        QChar ch('a' + i);
        out() << QString("<a href=\"#%1\">%2</a>&nbsp;").arg(ch).arg(ch.toUpper());
    }
    out() << "</b></p>\n";

    char nextLetter = 'a';

    out() << "<ul translate=\"no\">\n";
    NodeMapMap &funcIndex = m_qdb->getFunctionIndex();
    for (auto fnMap = funcIndex.constBegin(); fnMap != funcIndex.constEnd(); ++fnMap) {
        const QString &key = fnMap.key();
        const QChar firstLetter = key.isEmpty() ? QChar('A') : key.front();
        Q_ASSERT_X(firstLetter.unicode() < 256, "generateFunctionIndex",
                   "Only valid C++ identifiers were expected");
        const char currentLetter = firstLetter.isLower() ? firstLetter.unicode() : nextLetter - 1;

        if (currentLetter < nextLetter) {
            out() << "<li>";
        } else {
            // TODO: This is not covered by our tests
            while (nextLetter < currentLetter)
                out() << QStringLiteral("<li id=\"%1\"></li>").arg(nextLetter++);
            Q_ASSERT(nextLetter == currentLetter);
            out() << QStringLiteral("<li id=\"%1\">").arg(nextLetter++);
        }
        out() << protectEnc(key) << ':';

        for (auto it = (*fnMap).constBegin(); it != (*fnMap).constEnd(); ++it) {
            out() << ' ';
            generateFullName((*it)->parent(), relative, *it);
        }
        out() << "</li>\n";
    }
    while (nextLetter <= 'z')
        out() << QStringLiteral("<li id=\"%1\"></li>").arg(nextLetter++);
    out() << "</ul>\n";
}

void HtmlGenerator::generateLegaleseList(const Node *relative, CodeMarker *marker)
{
    TextToNodeMap &legaleseTexts = m_qdb->getLegaleseTexts();
    for (auto it = legaleseTexts.cbegin(), end = legaleseTexts.cend(); it != end; ++it) {
        Text text = it.key();
        generateText(text, relative, marker);
        out() << "<ul>\n";
        do {
            out() << "<li>";
            generateFullName(it.value(), relative);
            out() << "</li>\n";
            ++it;
        } while (it != legaleseTexts.constEnd() && it.key() == text);
        out() << "</ul>\n";
    }
}

void HtmlGenerator::generateQmlItem(const Node *node, const Node *relative, CodeMarker *marker,
                                    bool summary)
{
    QString marked = marker->markedUpQmlItem(node, summary);
    marked.replace("@param>", "i>");

    marked.replace("<@extra>", "<code class=\"%1 extra\" translate=\"no\">"_L1
                    .arg(summary ? "summary"_L1 : "details"_L1));
    marked.replace("</@extra>", "</code>");


    if (summary) {
        marked.remove("<@name>");
        marked.remove("</@name>");
        marked.remove("<@type>");
        marked.remove("</@type>");
    }
    out() << highlightedCode(marked, relative, false, Genus::QML);
}

/*!
  This function generates a simple list (without annotations) for
  the members of collection node \a {cn}. The list is sorted
  according to \a sortOrder.

  Returns \c true if the list was generated (collection has members),
  \c false otherwise.
 */
bool HtmlGenerator::generateGroupList(CollectionNode *cn, Qt::SortOrder sortOrder)
{
    m_qdb->mergeCollections(cn);
    if (cn->members().isEmpty())
        return false;

    NodeList members{cn->members()};
    if (sortOrder == Qt::DescendingOrder)
        std::sort(members.rbegin(), members.rend(), Node::nodeSortKeyOrNameLessThan);
    else
        std::sort(members.begin(), members.end(), Node::nodeSortKeyOrNameLessThan);
    out() << "<ul>\n";
    for (const auto *node : std::as_const(members)) {
        out() << "<li translate=\"no\">";
        generateFullName(node, nullptr);
        out() << "</li>\n";
    }
    out() << "</ul>\n";
    return true;
}

void HtmlGenerator::generateList(const Node *relative, CodeMarker *marker,
                                 const QString &selector, Qt::SortOrder sortOrder)
{
    CNMap cnm;
    NodeType type = NodeType::NoType;
    if (selector == QLatin1String("overviews"))
        type = NodeType::Group;
    else if (selector == QLatin1String("cpp-modules"))
        type = NodeType::Module;
    else if (selector == QLatin1String("qml-modules"))
        type = NodeType::QmlModule;
    if (type != NodeType::NoType) {
        NodeList nodeList;
        m_qdb->mergeCollections(type, cnm, relative);
        const auto collectionList = cnm.values();
        nodeList.reserve(collectionList.size());
        for (auto *collectionNode : collectionList)
            nodeList.append(collectionNode);
        generateAnnotatedList(relative, marker, nodeList, sortOrder);
    } else {
        /*
          \generatelist {selector} is only allowed in a
          comment where the topic is \group, \module, or
          \qmlmodule.
        */
        if (relative && !relative->isCollectionNode()) {
            relative->doc().location().warning(
                    QStringLiteral("\\generatelist {%1} is only allowed in \\group, "
                                   "\\module and \\qmlmodule comments.")
                            .arg(selector));
            return;
        }
        auto *node = const_cast<Node *>(relative);
        auto *collectionNode = static_cast<CollectionNode *>(node);
        if (!collectionNode)
            return;
        m_qdb->mergeCollections(collectionNode);
        generateAnnotatedList(collectionNode, marker, collectionNode->members(), sortOrder);
    }
}

void HtmlGenerator::generateSection(const NodeVector &nv, const Node *relative, CodeMarker *marker)
{
    bool alignNames = true;
    if (!nv.isEmpty()) {
        bool twoColumn = false;
        if (nv.first()->isProperty()) {
            twoColumn = (nv.size() >= 5);
            alignNames = false;
        }
        if (alignNames) {
            out() << "<div class=\"table\"><table class=\"alignedsummary\" translate=\"no\">\n";
        } else {
            if (twoColumn)
                out() << "<div class=\"table\"><table class=\"propsummary\" translate=\"no\">\n"
                      << "<tr><td class=\"topAlign\">";
            out() << "<ul>\n";
        }

        int i = 0;
        for (const auto &member : nv) {
            if (member->access() == Access::Private)
                continue;

            if (alignNames) {
                out() << "<tr><td class=\"memItemLeft rightAlign topAlign\"> ";
            } else {
                if (twoColumn && i == (nv.size() + 1) / 2)
                    out() << "</ul></td><td class=\"topAlign\"><ul>\n";
                out() << "<li class=\"fn\" translate=\"no\">";
            }

            generateSynopsis(member, relative, marker, Section::Summary, alignNames);
            if (alignNames)
                out() << "</td></tr>\n";
            else
                out() << "</li>\n";
            i++;
        }
        if (alignNames)
            out() << "</table></div>\n";
        else {
            out() << "</ul>\n";
            if (twoColumn)
                out() << "</td></tr>\n</table></div>\n";
        }
    }
}

void HtmlGenerator::generateSectionList(const Section &section, const Node *relative,
                                        CodeMarker *marker, bool useObsoleteMembers)
{
    bool alignNames = true;
    const NodeVector &members =
            (useObsoleteMembers ? section.obsoleteMembers() : section.members());
    if (!members.isEmpty()) {
        bool hasPrivateSignals = false;
        bool isInvokable = false;
        bool twoColumn = false;
        if (section.style() == Section::AllMembers) {
            alignNames = false;
            twoColumn = (members.size() >= 16);
        } else if (members.first()->isProperty()) {
            twoColumn = (members.size() >= 5);
            alignNames = false;
        }
        if (alignNames) {
            out() << "<div class=\"table\"><table class=\"alignedsummary\" translate=\"no\">\n";
        } else {
            if (twoColumn)
                out() << "<div class=\"table\"><table class=\"propsummary\" translate=\"no\">\n"
                      << "<tr><td class=\"topAlign\">";
            out() << "<ul>\n";
        }

        int i = 0;
        for (const auto &member : members) {
            if (member->access() == Access::Private)
                continue;

            if (alignNames) {
                out() << "<tr><td class=\"memItemLeft topAlign rightAlign\"> ";
            } else {
                if (twoColumn && i == (members.size() + 1) / 2)
                    out() << "</ul></td><td class=\"topAlign\"><ul>\n";
                out() << "<li class=\"fn\" translate=\"no\">";
            }

            generateSynopsis(member, relative, marker, section.style(), alignNames);
            if (member->isFunction()) {
                const auto *fn = static_cast<const FunctionNode *>(member);
                if (fn->isPrivateSignal()) {
                    hasPrivateSignals = true;
                    if (alignNames)
                        out() << "</td><td class=\"memItemRight bottomAlign\">[see note below]";
                } else if (fn->isInvokable()) {
                    isInvokable = true;
                    if (alignNames)
                        out() << "</td><td class=\"memItemRight bottomAlign\">[see note below]";
                }
            }
            if (alignNames)
                out() << "</td></tr>\n";
            else
                out() << "</li>\n";
            i++;
        }
        if (alignNames)
            out() << "</table></div>\n";
        else {
            out() << "</ul>\n";
            if (twoColumn)
                out() << "</td></tr>\n</table></div>\n";
        }
        if (alignNames) {
            if (hasPrivateSignals)
                generateAddendum(relative, Generator::PrivateSignal, marker);
            if (isInvokable)
                generateAddendum(relative, Generator::Invokable, marker);
        }
    }

    if (!useObsoleteMembers && section.style() == Section::Summary
        && !section.inheritedMembers().isEmpty()) {
        out() << "<ul>\n";
        generateSectionInheritedList(section, relative);
        out() << "</ul>\n";
    }
}

void HtmlGenerator::generateSectionInheritedList(const Section &section, const Node *relative)
{
    const QList<std::pair<Aggregate *, int>> &inheritedMembers = section.inheritedMembers();
    for (const auto &member : inheritedMembers) {
        out() << "<li class=\"fn\" translate=\"no\">";
        out() << member.second << ' ';
        if (member.second == 1) {
            out() << section.singular();
        } else {
            out() << section.plural();
        }
        out() << " inherited from <a href=\"" << fileName(member.first) << '#'
              << Generator::cleanRef(section.title().toLower()) << "\">"
              << protectEnc(member.first->plainFullName(relative)) << "</a></li>\n";
    }
}

void HtmlGenerator::generateSynopsis(const Node *node, const Node *relative, CodeMarker *marker,
                                     Section::Style style, bool alignNames)
{
    QString marked = marker->markedUpSynopsis(node, relative, style);
    marked.replace("@param>", "i>");

    if (style == Section::Summary) {
        marked.remove("<@name>");
        marked.remove("</@name>");
    }

    if (style == Section::AllMembers) {
        static const  QRegularExpression extraRegExp("<@extra>.*</@extra>",
                                                     QRegularExpression::InvertedGreedinessOption);
        marked.remove(extraRegExp);
    } else {
        marked.replace("<@extra>", "<code class=\"%1 extra\" translate=\"no\">"_L1
                    .arg(style == Section::Summary ? "summary"_L1 : "details"_L1));
        marked.replace("</@extra>", "</code>");
    }

    if (style != Section::Details) {
        marked.remove("<@type>");
        marked.remove("</@type>");
    }

    out() << highlightedCode(marked, relative, alignNames);
}

QString HtmlGenerator::highlightedCode(const QString &markedCode, const Node *relative,
                                       bool alignNames, Genus genus)
{
    QString src = markedCode;
    QString html;
    html.reserve(src.size());
    QStringView arg;
    QStringView par1;

    const QChar charLangle = '<';
    const QChar charAt = '@';

    static const QString typeTag("type");
    static const QString headerTag("headerfile");
    static const QString funcTag("func");
    static const QString linkTag("link");

    // replace all <@link> tags: "(<@link node=\"([^\"]+)\">).*(</@link>)"
    // replace all <@func> tags: "(<@func target=\"([^\"]*)\">)(.*)(</@func>)"
    // replace all "(<@(type|headerfile)(?: +[^>]*)?>)(.*)(</@\\2>)" tags
    bool done = false;
    for (int i = 0, srcSize = src.size(); i < srcSize;) {
        if (src.at(i) == charLangle && src.at(i + 1) == charAt) {
            if (alignNames && !done) {
                html += QLatin1String("</td><td class=\"memItemRight bottomAlign\">");
                done = true;
            }
            i += 2;
            if (parseArg(src, linkTag, &i, srcSize, &arg, &par1)) {
                html += QLatin1String("<b>");
                const Node *n = static_cast<const Node*>(Utilities::nodeForString(par1.toString()));
                QString link = linkForNode(n, relative);
                addLink(link, arg, &html);
                html += QLatin1String("</b>");
            } else if (parseArg(src, funcTag, &i, srcSize, &arg, &par1)) {
                const FunctionNode *fn = m_qdb->findFunctionNode(par1.toString(), relative, genus);
                QString link = linkForNode(fn, relative);
                addLink(link, arg, &html);
                par1 = QStringView();
            } else if (parseArg(src, typeTag, &i, srcSize, &arg, &par1)) {
                par1 = QStringView();
                const Node *n = m_qdb->findTypeNode(arg.toString(), relative, genus);
                html += QLatin1String("<span class=\"type\">");
                if (n && (n->isQmlBasicType())) {
                    if (relative && (relative->genus() == n->genus() || genus == n->genus()))
                        addLink(linkForNode(n, relative), arg, &html);
                    else
                        html += arg;
                } else
                    addLink(linkForNode(n, relative), arg, &html);
                html += QLatin1String("</span>");
            } else if (parseArg(src, headerTag, &i, srcSize, &arg, &par1)) {
                par1 = QStringView();
                if (arg.startsWith(QLatin1Char('&')))
                    html += arg;
                else {
                    const Node *n = m_qdb->findNodeForInclude(QStringList(arg.toString()));
                    if (n && n != relative)
                        addLink(linkForNode(n, relative), arg, &html);
                    else
                        html += arg;
                }
            } else {
                html += charLangle;
                html += charAt;
            }
        } else {
            html += src.at(i++);
        }
    }

    // replace all
    // "<@comment>" -> "<span class=\"comment\">";
    // "<@preprocessor>" -> "<span class=\"preprocessor\">";
    // "<@string>" -> "<span class=\"string\">";
    // "<@char>" -> "<span class=\"char\">";
    // "<@number>" -> "<span class=\"number\">";
    // "<@op>" -> "<span class=\"operator\">";
    // "<@type>" -> "<span class=\"type\">";
    // "<@name>" -> "<span class=\"name\">";
    // "<@keyword>" -> "<span class=\"keyword\">";
    // "</@(?:comment|preprocessor|string|char|number|op|type|name|keyword)>" -> "</span>"
    src = html;
    html = QString();
    html.reserve(src.size());
    static const QLatin1String spanTags[] = {
        QLatin1String("comment>"),      QLatin1String("<span class=\"comment\">"),
        QLatin1String("preprocessor>"), QLatin1String("<span class=\"preprocessor\">"),
        QLatin1String("string>"),       QLatin1String("<span class=\"string\">"),
        QLatin1String("char>"),         QLatin1String("<span class=\"char\">"),
        QLatin1String("number>"),       QLatin1String("<span class=\"number\">"),
        QLatin1String("op>"),           QLatin1String("<span class=\"operator\">"),
        QLatin1String("type>"),         QLatin1String("<span class=\"type\">"),
        QLatin1String("name>"),         QLatin1String("<span class=\"name\">"),
        QLatin1String("keyword>"),      QLatin1String("<span class=\"keyword\">")
    };
    int nTags = 9;
    // Update the upper bound of k in the following code to match the length
    // of the above array.
    for (int i = 0, n = src.size(); i < n;) {
        if (src.at(i) == QLatin1Char('<')) {
            if (src.at(i + 1) == QLatin1Char('@')) {
                i += 2;
                bool handled = false;
                for (int k = 0; k != nTags; ++k) {
                    const QLatin1String &tag = spanTags[2 * k];
                    if (i + tag.size() <= src.size() && tag == QStringView(src).mid(i, tag.size())) {
                        html += spanTags[2 * k + 1];
                        i += tag.size();
                        handled = true;
                        break;
                    }
                }
                if (!handled) {
                    // drop 'our' unknown tags (the ones still containing '@')
                    while (i < n && src.at(i) != QLatin1Char('>'))
                        ++i;
                    ++i;
                }
                continue;
            } else if (src.at(i + 1) == QLatin1Char('/') && src.at(i + 2) == QLatin1Char('@')) {
                i += 3;
                bool handled = false;
                for (int k = 0; k != nTags; ++k) {
                    const QLatin1String &tag = spanTags[2 * k];
                    if (i + tag.size() <= src.size() && tag == QStringView(src).mid(i, tag.size())) {
                        html += QLatin1String("</span>");
                        i += tag.size();
                        handled = true;
                        break;
                    }
                }
                if (!handled) {
                    // drop 'our' unknown tags (the ones still containing '@')
                    while (i < n && src.at(i) != QLatin1Char('>'))
                        ++i;
                    ++i;
                }
                continue;
            }
        }
        html += src.at(i);
        ++i;
    }
    return html;
}

void HtmlGenerator::generateLink(const Atom *atom)
{
    Q_ASSERT(m_inLink);

    if (m_linkNode && m_linkNode->isFunction()) {
        auto match = XmlGenerator::m_funcLeftParen.match(atom->string());
        if (match.hasMatch()) {
            // C++: move () outside of link
            qsizetype leftParenLoc = match.capturedStart(1);
            out() << protectEnc(atom->string().left(leftParenLoc));
            endLink();
            out() << protectEnc(atom->string().mid(leftParenLoc));
            return;
        }
    }
    out() << protectEnc(atom->string());
}

QString HtmlGenerator::protectEnc(const QString &string)
{
    return protect(string);
}

QString HtmlGenerator::protect(const QString &string)
{
    if (string.isEmpty())
        return string;

#define APPEND(x)                                                                                  \
    if (html.isEmpty()) {                                                                          \
        html = string;                                                                             \
        html.truncate(i);                                                                          \
    }                                                                                              \
    html += (x);

    QString html;
    qsizetype n = string.size();

    for (int i = 0; i < n; ++i) {
        QChar ch = string.at(i);

        if (ch == QLatin1Char('&')) {
            APPEND("&amp;");
        } else if (ch == QLatin1Char('<')) {
            APPEND("&lt;");
        } else if (ch == QLatin1Char('>')) {
            APPEND("&gt;");
        } else if (ch == QChar(8211)) {
            APPEND("&ndash;");
        } else if (ch == QChar(8212)) {
            APPEND("&mdash;");
        } else if (ch == QLatin1Char('"')) {
            APPEND("&quot;");
        } else {
            if (!html.isEmpty())
                html += ch;
        }
    }

    if (!html.isEmpty())
        return html;
    return string;

#undef APPEND
}

QString HtmlGenerator::fileBase(const Node *node) const
{
    QString result = Generator::fileBase(node);
    if (!node->isAggregate() && node->isDeprecated())
        result += QLatin1String("-obsolete");
    return result;
}

QString HtmlGenerator::fileName(const Node *node)
{
    if (node->isExternalPage())
        return node->name();
    return Generator::fileName(node);
}

void HtmlGenerator::generateFullName(const Node *apparentNode, const Node *relative,
                                     const Node *actualNode)
{
    if (actualNode == nullptr)
        actualNode = apparentNode;
    bool link = !linkForNode(actualNode, relative).isEmpty();
    if (link) {
        out() << "<a href=\"" << linkForNode(actualNode, relative);
        if (actualNode->isDeprecated())
            out() << "\" class=\"obsolete";
        out() << "\">";
    }
    out() << protectEnc(apparentNode->fullName(relative));
    if (link)
        out() << "</a>";
}

/*!
    Generates a link to the declaration of the C++ API entity
    represented by \a node.
*/
void HtmlGenerator::generateSourceLink(const Node *node)
{
    Q_ASSERT(node);
    if (node->genus() != Genus::CPP)
        return;

    const auto srcLink = Config::instance().getSourceLink();
    if (!srcLink.enabled)
        return;

    // With no valid configuration or location, do nothing
    const auto &loc{node->declLocation()};
    if (loc.isEmpty() || srcLink.baseUrl.isEmpty() || srcLink.rootPath.isEmpty())
        return;

    QString srcUrl{srcLink.baseUrl};
    if (!srcUrl.contains('\1'_L1)) {
        if (!srcUrl.endsWith('/'_L1))
            srcUrl += '/'_L1;
        srcUrl += '\1'_L1;
    }

    QDir rootDir{srcLink.rootPath};
    srcUrl.replace('\1'_L1, rootDir.relativeFilePath(loc.filePath()));
    srcUrl.replace('\2'_L1, QString::number(loc.lineNo()));
    const auto &description{"View declaration of this %1"_L1.arg(node->nodeTypeString())};
    out() << "<a class=\"srclink\" href=\"%1\" title=\"%2\">%3</a>"_L1
            .arg(srcUrl, description, srcLink.linkText);
}

void HtmlGenerator::generateDetailedMember(const Node *node, const PageNode *relative,
                                           CodeMarker *marker)
{
    const EnumNode *etn;
    generateExtractionMark(node, MemberMark);
    QString nodeRef = nullptr;
    if (node->isSharedCommentNode()) {
        const auto *scn = reinterpret_cast<const SharedCommentNode *>(node);
        const QList<Node *> &collective = scn->collective();
        if (collective.size() > 1)
            out() << "<div class=\"fngroup\">\n";
        for (const auto *sharedNode : collective) {
            nodeRef = refForNode(sharedNode);
            out() << R"(<h3 class="fn fngroupitem" translate="no" id=")" << nodeRef << "\">";
            generateSynopsis(sharedNode, relative, marker, Section::Details);
            generateSourceLink(sharedNode);
            out() << "</h3>";
        }
        if (collective.size() > 1)
            out() << "</div>";
        out() << '\n';
    } else {
        nodeRef = refForNode(node);
        if (node->isEnumType(Genus::CPP) && (etn = static_cast<const EnumNode *>(node))->flagsType()) {
            out() << R"(<h3 class="flags" id=")" << nodeRef << "\">";
            generateSynopsis(etn, relative, marker, Section::Details);
            out() << "<br/>";
            generateSynopsis(etn->flagsType(), relative, marker, Section::Details);
            generateSourceLink(node);
            out() << "</h3>\n";
        } else {
            out() << R"(<h3 class="fn" translate="no" id=")" << nodeRef << "\">";
            generateSynopsis(node, relative, marker, Section::Details);
            generateSourceLink(node);
            out() << "</h3>" << '\n';
        }
    }

    generateStatus(node, marker);
    generateBody(node, marker);
    if (node->isFunction()) {
        const auto *func = static_cast<const FunctionNode *>(node);
        if (func->hasOverloads() && (func->isSignal() || func->isSlot()))
            generateAddendum(node, OverloadNote, marker, AdmonitionPrefix::Note);
    }
    generateComparisonCategory(node, marker);
    generateThreadSafeness(node, marker);
    generateSince(node, marker);
    generateNoexceptNote(node, marker);

    if (node->isProperty()) {
        const auto property = static_cast<const PropertyNode *>(node);
        if (property->propertyType() == PropertyNode::PropertyType::StandardProperty ||
            property->propertyType() == PropertyNode::PropertyType::BindableProperty) {
            Section section("", "", "", "", Section::Accessors);

            section.appendMembers(property->getters().toVector());
            section.appendMembers(property->setters().toVector());
            section.appendMembers(property->resetters().toVector());

            if (!section.members().isEmpty()) {
                out() << "<p><b>Access functions:</b></p>\n";
                generateSectionList(section, node, marker);
            }

            Section notifiers("", "", "", "", Section::Accessors);
            notifiers.appendMembers(property->notifiers().toVector());

            if (!notifiers.members().isEmpty()) {
                out() << "<p><b>Notifier signal:</b></p>\n";
                generateSectionList(notifiers, node, marker);
            }
        }
    } else if (node->isEnumType(Genus::CPP)) {
        const auto *enumTypeNode = static_cast<const EnumNode *>(node);
        if (enumTypeNode->flagsType()) {
            out() << "<p>The " << protectEnc(enumTypeNode->flagsType()->name())
                  << " type is a typedef for "
                  << "<a href=\"" << m_qflagsHref << "\">QFlags</a>&lt;"
                  << protectEnc(enumTypeNode->name()) << "&gt;. It stores an OR combination of "
                  << protectEnc(enumTypeNode->name()) << " values.</p>\n";
        }
    }
    generateAlsoList(node, marker);
    generateExtractionMark(node, EndMark);
}

/*!
  This version of the function is called when outputting the link
  to an example file or example image, where the \a link is known
  to be correct.
 */
void HtmlGenerator::beginLink(const QString &link)
{
    m_link = link;
    m_inLink = true;
    m_linkNode = nullptr;

    if (!m_link.isEmpty())
        out() << "<a href=\"" << m_link << "\" translate=\"no\">";
}

void HtmlGenerator::beginLink(const QString &link, const Node *node, const Node *relative)
{
    m_link = link;
    m_inLink = true;
    m_linkNode = node;
    if (m_link.isEmpty())
        return;

    const QString &translate_attr =
    (node && isApiGenus(node->genus())) ? " translate=\"no\""_L1 : ""_L1;

    if (node == nullptr || (relative != nullptr && node->status() == relative->status()))
        out() << "<a href=\"" << m_link << "\"%1>"_L1.arg(translate_attr);
    else if (node->isDeprecated())
        out() << "<a href=\"" << m_link << "\" class=\"obsolete\"%1>"_L1.arg(translate_attr);
    else
        out() << "<a href=\"" << m_link << "\"%1>"_L1.arg(translate_attr);
}

void HtmlGenerator::endLink()
{
    if (!m_inLink)
        return;

    m_inLink = false;
    m_linkNode = nullptr;

    if (!m_link.isEmpty())
        out() << "</a>";
}

/*!
  Generates the summary list for the \a members. Only used for
  sections of QML element documentation.
 */
void HtmlGenerator::generateQmlSummary(const NodeVector &members, const Node *relative,
                                       CodeMarker *marker)
{
    if (!members.isEmpty()) {
        out() << "<ul>\n";
        for (const auto &member : members) {
            out() << "<li class=\"fn\" translate=\"no\">";
            generateQmlItem(member, relative, marker, true);
            if (member->isPropertyGroup()) {
                const auto *scn = static_cast<const SharedCommentNode *>(member);
                if (scn->count() > 0) {
                    out() << "<ul>\n";
                    const QList<Node *> &sharedNodes = scn->collective();
                    for (const auto &node : sharedNodes) {
                        if (node->isQmlProperty()) {
                            out() << "<li class=\"fn\" translate=\"no\">";
                            generateQmlItem(node, relative, marker, true);
                            out() << "</li>\n";
                        }
                    }
                    out() << "</ul>\n";
                }
            }
            out() << "</li>\n";
        }
        out() << "</ul>\n";
    }
}

/*!
  Outputs the html detailed documentation for a section
  on a QML element reference page.
 */
void HtmlGenerator::generateDetailedQmlMember(Node *node, const Aggregate *relative,
                                              CodeMarker *marker)
{
    generateExtractionMark(node, MemberMark);

    QString qmlItemHeader("<div class=\"qmlproto\" translate=\"no\">\n"
                          "<div class=\"table\"><table class=\"qmlname\">\n");

    QString qmlItemStart("<tr valign=\"top\" class=\"odd\" id=\"%1\">\n"
                         "<td class=\"%2\"><p>\n");
    QString qmlItemEnd("</p></td></tr>\n");

    QString qmlItemFooter("</table></div></div>\n");

    auto generateQmlProperty = [&](Node *n) {
        out() << qmlItemStart.arg(refForNode(n), "tblQmlPropNode");
        generateQmlItem(n, relative, marker, false);
        out() << qmlItemEnd;
    };

    auto generateQmlMethod = [&](Node *n) {
        out() << qmlItemStart.arg(refForNode(n), "tblQmlFuncNode");
        generateSynopsis(n, relative, marker, Section::Details, false);
        out() << qmlItemEnd;
    };

    out() << "<div class=\"qmlitem\">";
    if (node->isPropertyGroup()) {
        const auto *scn = static_cast<const SharedCommentNode *>(node);
        out() << qmlItemHeader;
        if (!scn->name().isEmpty()) {
            const QString nodeRef = refForNode(scn);
            out() << R"(<tr valign="top" class="even" id=")" << nodeRef << "\">";
            out() << "<th class=\"centerAlign\"><p>";
            out() << "<b>" << scn->name() << " group</b>";
            out() << "</p></th></tr>\n";
        }
        const QList<Node *> sharedNodes = scn->collective();
        for (const auto &sharedNode : sharedNodes) {
            if (sharedNode->isQmlProperty())
                generateQmlProperty(sharedNode);
        }
        out() << qmlItemFooter;
    } else if (node->isQmlProperty()) {
        out() << qmlItemHeader;
        generateQmlProperty(node);
        out() << qmlItemFooter;
    } else if (node->isSharedCommentNode()) {
        const auto *scn = reinterpret_cast<const SharedCommentNode *>(node);
        const QList<Node *> &sharedNodes = scn->collective();
        if (sharedNodes.size() > 1)
            out() << "<div class=\"fngroup\">\n";
        out() << qmlItemHeader;
        for (const auto &sharedNode : sharedNodes) {
            // Generate the node only if it's a QML method
            if (sharedNode->isFunction(Genus::QML))
                generateQmlMethod(sharedNode);
            else if (sharedNode->isQmlProperty())
                generateQmlProperty(sharedNode);
        }
        out() << qmlItemFooter;
        if (sharedNodes.size() > 1)
            out() << "</div>"; // fngroup
    } else { // assume the node is a method/signal handler
        out() << qmlItemHeader;
        generateQmlMethod(node);
        out() << qmlItemFooter;
    }

    out() << "<div class=\"qmldoc\">";
    generateStatus(node, marker);
    generateBody(node, marker);
    generateThreadSafeness(node, marker);
    generateSince(node, marker);
    generateAlsoList(node, marker);
    out() << "</div></div>";
    generateExtractionMark(node, EndMark);
}

void HtmlGenerator::generateExtractionMark(const Node *node, ExtractionMarkType markType)
{
    if (markType != EndMark) {
        out() << "<!-- $$$" + node->name();
        if (markType == MemberMark) {
            if (node->isFunction()) {
                const auto *func = static_cast<const FunctionNode *>(node);
                if (!func->hasAssociatedProperties()) {
                    if (func->overloadNumber() == 0)
                        out() << "[overload1]";
                    out() << "$$$" + func->name() + func->parameters().rawSignature().remove(' ');
                }
            } else if (node->isProperty()) {
                out() << "-prop";
                const auto *prop = static_cast<const PropertyNode *>(node);
                const NodeList &list = prop->functions();
                for (const auto *propFuncNode : list) {
                    if (propFuncNode->isFunction()) {
                        const auto *func = static_cast<const FunctionNode *>(propFuncNode);
                        out() << "$$$" + func->name()
                                        + func->parameters().rawSignature().remove(' ');
                    }
                }
            } else if (node->isEnumType()) {
                const auto *enumNode = static_cast<const EnumNode *>(node);
                const auto &items = enumNode->items();
                for (const auto &item : items)
                    out() << "$$$" + item.name();
            }
        } else if (markType == BriefMark) {
            out() << "-brief";
        } else if (markType == DetailedDescriptionMark) {
            out() << "-description";
        }
        out() << " -->\n";
    } else {
        out() << "<!-- @@@" + node->name() + " -->\n";
    }
}

QT_END_NAMESPACE
