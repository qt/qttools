// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
#include "manifestwriter.h"

#include "config.h"
#include "examplenode.h"
#include "generator.h"
#include "qdocdatabase.h"

#include <QtCore/qmap.h>
#include <QtCore/qset.h>
#include <QtCore/qxmlstream.h>

QT_BEGIN_NAMESPACE

/*!
    \internal

    For each attribute in a map of attributes, checks if the attribute is
    found in \a usedAttributes. If it is not found, issues a warning specific
    to the attribute.
 */
void warnAboutUnusedAttributes(const QStringList &usedAttributes, const ExampleNode *example)
{
    QMap<QString, QString> attributesToWarnFor;
    bool missingImageWarning = Config::instance().get(CONFIG_EXAMPLES + Config::dot + CONFIG_WARNABOUTMISSINGIMAGES).asBool();
    bool missingProjectFileWarning = Config::instance().get(CONFIG_EXAMPLES + Config::dot + CONFIG_WARNABOUTMISSINGPROJECTFILES).asBool();

    if (missingImageWarning)
        attributesToWarnFor.insert(QStringLiteral("imageUrl"),
                QStringLiteral("Example documentation should have at least one '\\image'"));
    if (missingProjectFileWarning)
        attributesToWarnFor.insert(QStringLiteral("projectPath"),
                QStringLiteral("Example has no project file"));

    if (attributesToWarnFor.empty())
        return;

    for (auto it = attributesToWarnFor.cbegin(); it != attributesToWarnFor.cend(); ++it) {
        if (!usedAttributes.contains(it.key()))
            example->doc().location().warning(example->name() + ": " + it.value());
    }
}

/*!
    \internal

    Write the description element. The description for an example is set
    with the \brief command. If no brief is available, the element is set
    to "No description available".
 */

void writeDescription(QXmlStreamWriter *writer, const ExampleNode *example)
{
    Q_ASSERT(writer && example);
    writer->writeStartElement("description");
    const Text brief = example->doc().briefText();
    if (!brief.isEmpty())
        writer->writeCDATA(brief.toString());
    else
        writer->writeCDATA(QString("No description available"));
    writer->writeEndElement(); // description
}

/*!
    \internal

    Returns a list of \a files that Qt Creator should open for the \a exampleName.
 */
QMap<int, QString> getFilesToOpen(const QStringList &files, const QString &exampleName)
{
    QMap<int, QString> filesToOpen;
    for (const QString &file : files) {
        QFileInfo fileInfo(file);
        QString fileName = fileInfo.fileName().toLower();
        // open .qml, .cpp and .h files with a
        // basename matching the example (project) name
        // QMap key indicates the priority -
        // the lowest value will be the top-most file
        if ((fileInfo.baseName().compare(exampleName, Qt::CaseInsensitive) == 0)) {
            if (fileName.endsWith(".qml"))
                filesToOpen.insert(0, file);
            else if (fileName.endsWith(".cpp"))
                filesToOpen.insert(1, file);
            else if (fileName.endsWith(".h"))
                filesToOpen.insert(2, file);
        }
        // main.qml takes precedence over main.cpp
        else if (fileName.endsWith("main.qml")) {
            filesToOpen.insert(3, file);
        } else if (fileName.endsWith("main.cpp")) {
            filesToOpen.insert(4, file);
        }
    }

    return filesToOpen;
}

/*!
    \internal
    \brief Writes the lists of files to open for the example.

    Writes out the \a filesToOpen and the full \a installPath through \a writer.
 */
void writeFilesToOpen(QXmlStreamWriter &writer, const QString &installPath,
                      const QMap<int, QString> &filesToOpen)
{
    for (auto it = filesToOpen.constEnd(); it != filesToOpen.constBegin();) {
        writer.writeStartElement("fileToOpen");
        if (--it == filesToOpen.constBegin()) {
            writer.writeAttribute(QStringLiteral("mainFile"), QStringLiteral("true"));
        }
        writer.writeCharacters(installPath + it.value());
        writer.writeEndElement();
    }
}

/*!
    \internal
    \brief Writes example metadata into \a writer.

    For instance,


      \ meta category {Application Example}

    becomes

      <meta>
        <entry name="category">Application Example</entry>
      <meta>
*/
static void writeMetaInformation(QXmlStreamWriter &writer, const QStringMultiMap &map)
{
    if (map.isEmpty())
        return;

    writer.writeStartElement("meta");
    for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
        writer.writeStartElement("entry");
        writer.writeAttribute(QStringLiteral("name"), it.key());
        writer.writeCharacters(it.value());
        writer.writeEndElement(); // tag
    }
    writer.writeEndElement(); // meta
}

/*!
    \class ManifestWriter
    \internal
    \brief The ManifestWriter is responsible for writing manifest files.
 */
ManifestWriter::ManifestWriter()
{
    Config &config = Config::instance();
    m_project = config.get(CONFIG_PROJECT).asString();
    m_outputDirectory = config.getOutputDir();
    m_qdb = QDocDatabase::qdocDB();

    const QString prefix = CONFIG_QHP + Config::dot + m_project + Config::dot;
    m_manifestDir =
            QLatin1String("qthelp://") + config.get(prefix + QLatin1String("namespace")).asString();
    m_manifestDir +=
            QLatin1Char('/') + config.get(prefix + QLatin1String("virtualFolder")).asString()
            + QLatin1Char('/');
    readManifestMetaContent();
    m_examplesPath = config.get(CONFIG_EXAMPLESINSTALLPATH).asString();
    if (!m_examplesPath.isEmpty())
        m_examplesPath += QLatin1Char('/');
}

template <typename F>
void ManifestWriter::processManifestMetaContent(const QString &fullName, F matchFunc)
{
    for (const auto &index : m_manifestMetaContent) {
        const auto &names = index.m_names;
        for (const QString &name : names) {
            bool match;
            qsizetype wildcard = name.indexOf(QChar('*'));
            switch (wildcard) {
            case -1: // no wildcard used, exact match required
                match = (fullName == name);
                break;
            case 0: // '*' matches all examples
                match = true;
                break;
            default: // match with wildcard at the end
                match = fullName.startsWith(name.left(wildcard));
            }
            if (match)
                matchFunc(index);
        }
    }
}

/*!
  This function outputs one or more manifest files in XML.
  They are used by Creator.
 */
void ManifestWriter::generateManifestFiles()
{
    generateExampleManifestFile();
    m_qdb->exampleNodeMap().clear();
    m_manifestMetaContent.clear();
}

/*
    Returns Qt module name as lower case tag, stripping Qt prefix:
    QtQuickControls -> quickcontrols
    QtOpenGL -> opengl
    QtQuick3D -> quick3d
 */
static QString moduleNameAsTag(const QString &module)
{
    QString moduleName = module;
    if (moduleName.startsWith("Qt"))
        moduleName = moduleName.mid(2);
    // Some examples are in QtDoc module, but 'doc' as tag makes little sense
    if (moduleName == "Doc")
        return QString();
    return moduleName.toLower();
}

/*
    Return tags that were added with
       \ meta {tag} {tag1[,tag2,...]}
    or
       \ meta {tags} {tag1[,tag2,...]}
    from example metadata
 */
static QSet<QString> tagsAddedWithMetaCommand(const ExampleNode *example)
{
    Q_ASSERT(example);

    QSet<QString> tags;
    const QStringMultiMap *metaTagMap = example->doc().metaTagMap();
    if (metaTagMap) {
        QStringList originalTags = metaTagMap->values("tag");
        originalTags << metaTagMap->values("tags");
        for (const auto &tag : originalTags) {
            const auto &tagList = tag.toLower().split(QLatin1Char(','), Qt::SkipEmptyParts);
            tags += QSet<QString>(tagList.constBegin(), tagList.constEnd());
        }
    }
    return tags;
}

/*
    Writes the contents of tags into writer, formatted as
      <tags>tag1,tag2..</tags>
 */
static void writeTagsElement(QXmlStreamWriter *writer, const QSet<QString> &tags)
{
    Q_ASSERT(writer);
    if (tags.isEmpty())
        return;

    writer->writeStartElement("tags");
    QStringList sortedTags = tags.values();
    sortedTags.sort();
    writer->writeCharacters(sortedTags.join(","));
    writer->writeEndElement(); // tags
}

/*!
  This function is called by generateExampleManifestFiles(), once
  for each manifest file to be generated.
 */
void ManifestWriter::generateExampleManifestFile()
{
    const ExampleNodeMap &exampleNodeMap = m_qdb->exampleNodeMap();
    if (exampleNodeMap.isEmpty())
        return;

    const QString outputFileName = "examples-manifest.xml";
    QFile outputFile(m_outputDirectory + QLatin1Char('/') + outputFileName);
    if (!outputFile.open(QFile::WriteOnly | QFile::Text))
        return;

    QXmlStreamWriter writer(&outputFile);
    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    writer.writeStartElement("instructionals");
    writer.writeAttribute("module", m_project);
    writer.writeStartElement("examples");

    for (const auto &example : exampleNodeMap.values()) {
        QMap<QString, QString> usedAttributes;
        QSet<QString> tags;
        const QString installPath = retrieveExampleInstallationPath(example);
        const QString fullName = m_project + QLatin1Char('/') + example->title();

        processManifestMetaContent(
                fullName, [&](const ManifestMetaFilter &filter) { tags += filter.m_tags; });
        tags += tagsAddedWithMetaCommand(example);
        // omit from the manifest if explicitly marked broken
        if (tags.contains("broken"))
            continue;

        // attributes that are always written for the element
        usedAttributes.insert("name", example->title());
        usedAttributes.insert("docUrl", m_manifestDir + Generator::currentGenerator()->fileBase(example) + ".html");

        if (!example->projectFile().isEmpty())
            usedAttributes.insert("projectPath", installPath + example->projectFile());
        if (!example->imageFileName().isEmpty())
            usedAttributes.insert("imageUrl",  m_manifestDir + example->imageFileName());

        processManifestMetaContent(fullName, [&](const ManifestMetaFilter &filter) {
            const auto attributes = filter.m_attributes;
            for (const auto &attribute : attributes) {
                const QLatin1Char div(':');
                QStringList attrList = attribute.split(div);
                if (attrList.size() == 1)
                    attrList.append(QStringLiteral("true"));
                QString attrName = attrList.takeFirst();
                if (!usedAttributes.contains(attrName))
                    usedAttributes.insert(attrName, attrList.join(div));
            }
        });

        writer.writeStartElement("example");
        for (auto it = usedAttributes.cbegin(); it != usedAttributes.cend(); ++it)
            writer.writeAttribute(it.key(), it.value());

        warnAboutUnusedAttributes(usedAttributes.keys(), example);
        writeDescription(&writer, example);

        const QString moduleNameTag = moduleNameAsTag(m_project);
        if (!moduleNameTag.isEmpty())
            tags << moduleNameTag;
        writeTagsElement(&writer, tags);

        const QString exampleName = example->name().mid(example->name().lastIndexOf('/') + 1);
        const auto files = example->files();
        const QMap<int, QString> filesToOpen = getFilesToOpen(files, exampleName);
        writeFilesToOpen(writer, installPath, filesToOpen);

        if (const QStringMultiMap *metaTagMapP = example->doc().metaTagMap()) {
            // Write \meta elements into the XML, except for 'tag', 'installpath',
            // as they are handled separately
            QStringMultiMap map = *metaTagMapP;
            erase_if(map, [](QStringMultiMap::iterator iter) {
                return iter.key() == "tag" || iter.key() == "tags" || iter.key() == "installpath";
            });
            writeMetaInformation(writer, map);
        }

        writer.writeEndElement(); // example
    }

    writer.writeEndElement(); // examples

    if (!m_exampleCategories.isEmpty()) {
        writer.writeStartElement("categories");
        for (const auto &examplecategory : m_exampleCategories) {
            writer.writeStartElement("category");
            writer.writeCharacters(examplecategory);
            writer.writeEndElement();
        }
        writer.writeEndElement(); // categories
    }

    writer.writeEndElement(); // instructionals
    writer.writeEndDocument();
    outputFile.close();
}

/*!
  Reads metacontent - additional attributes and tags to apply
  when generating manifest files, read from config.

  The manifest metacontent map is cleared immediately after
  the manifest files have been generated.
 */
void ManifestWriter::readManifestMetaContent()
{
    Config &config = Config::instance();
    const QStringList names{config.get(CONFIG_MANIFESTMETA
                            + Config::dot
                            + QStringLiteral("filters")).asStringList()};

    for (const auto &manifest : names) {
        ManifestMetaFilter filter;
        QString prefix = CONFIG_MANIFESTMETA + Config::dot + manifest + Config::dot;
        filter.m_names = config.get(prefix + QStringLiteral("names")).asStringSet();
        filter.m_attributes = config.get(prefix + QStringLiteral("attributes")).asStringSet();
        filter.m_tags = config.get(prefix + QStringLiteral("tags")).asStringSet();
        m_manifestMetaContent.append(filter);
    }

    m_exampleCategories = config.get(CONFIG_MANIFESTMETA
                                     + QStringLiteral(".examplecategories")).asStringList();
}

/*!
  Retrieve the install path for the \a example as specified with
  the \\meta command, or fall back to the one defined in .qdocconf.
 */
QString ManifestWriter::retrieveExampleInstallationPath(const ExampleNode *example) const
{
    QString installPath;
    if (example->doc().metaTagMap())
        installPath = example->doc().metaTagMap()->value(QLatin1String("installpath"));
    if (installPath.isEmpty())
        installPath = m_examplesPath;
    if (!installPath.isEmpty() && !installPath.endsWith(QLatin1Char('/')))
        installPath += QLatin1Char('/');

    return installPath;
}

QT_END_NAMESPACE
