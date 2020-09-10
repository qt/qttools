/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
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
#include "manifestwriter.h"

#include "config.h"
#include "examplenode.h"
#include "generator.h"
#include "qdocdatabase.h"

#include <QtCore/qxmlstream.h>

QT_BEGIN_NAMESPACE

/*!
    \class ManifestWriter
    \internal
    \brief The ManifestWriter is responsible for writing manifest files.
 */
ManifestWriter::ManifestWriter()
{
    Config &config = Config::instance();
    m_project = config.getString(CONFIG_PROJECT);
    m_outputDirectory = config.getOutputDir();
    m_qdb = QDocDatabase::qdocDB();

    const QString prefix = CONFIG_QHP + Config::dot + m_project + Config::dot;
    m_manifestDir =
            QLatin1String("qthelp://") + config.getString(prefix + QLatin1String("namespace"));
    m_manifestDir += QLatin1Char('/') + config.getString(prefix + QLatin1String("virtualFolder"))
            + QLatin1Char('/');
    readManifestMetaContent();
    m_examplesPath = config.getString(CONFIG_EXAMPLESINSTALLPATH);
    if (!m_examplesPath.isEmpty())
        m_examplesPath += QLatin1Char('/');
}

/*!
  This function outputs one or more manifest files in XML.
  They are used by Creator.
 */
void ManifestWriter::generateManifestFiles()
{
    generateManifestFile("examples", "example");
    generateManifestFile("demos", "demo");
    m_qdb->exampleNodeMap().clear();
    m_manifestMetaContent.clear();
}

/*!
  This function is called by generateManifestFiles(), once
  for each manifest file to be generated. \a manifest is the
  type of manifest file.
 */
void ManifestWriter::generateManifestFile(const QString &manifest, const QString &element)
{
    const ExampleNodeMap &exampleNodeMap = m_qdb->exampleNodeMap();
    if (exampleNodeMap.isEmpty())
        return;
    const QString fileName = manifest + "-manifest.xml";
    QFile file(m_outputDirectory + QLatin1Char('/') + fileName);
    bool demos = false;
    if (manifest == QLatin1String("demos"))
        demos = true;

    bool proceed = false;
    for (auto map = exampleNodeMap.begin(); map != exampleNodeMap.end(); ++map) {
        const ExampleNode *en = map.value();
        if (demos == en->name().startsWith("demos")) {
            proceed = true;
            break;
        }
    }
    if (!proceed || !file.open(QFile::WriteOnly | QFile::Text))
        return;

    QXmlStreamWriter writer(&file);
    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    writer.writeStartElement("instructionals");
    writer.writeAttribute("module", m_project);
    writer.writeStartElement(manifest);

    QStringList usedAttributes;
    for (auto map = exampleNodeMap.begin(); map != exampleNodeMap.end(); ++map) {
        const ExampleNode *en = map.value();
        if (demos) {
            if (!en->name().startsWith("demos"))
                continue;
        } else if (en->name().startsWith("demos")) {
            continue;
        }

        const QString installPath = retrieveInstallPath(en);
        // attributes that are always written for the element
        usedAttributes.clear();
        usedAttributes << "name"
                       << "docUrl"
                       << "projectPath";

        writer.writeStartElement(element);
        writer.writeAttribute("name", en->title());
        QString docUrl = m_manifestDir + Generator::fileBase(en) + ".html";
        writer.writeAttribute("docUrl", docUrl);
        const auto exampleFiles = en->files();
        if (en->projectFile().isEmpty())
            Location().warning("Example does not have a project file: ", en->name());
        else
            writer.writeAttribute("projectPath", installPath + en->projectFile());
        if (en->imageFileName().isEmpty()) {
            Location().warning("Example does not have an image file: ", en->name());
        } else {
            writer.writeAttribute("imageUrl", m_manifestDir + en->imageFileName());
            usedAttributes << "imageUrl";
        }

        QString fullName = m_project + QLatin1Char('/') + en->title();
        QSet<QString> tags;
        for (const auto &index : m_manifestMetaContent) {
            const auto &names = index.names;
            for (const QString &name : names) {
                bool match;
                int wildcard = name.indexOf(QChar('*'));
                switch (wildcard) {
                case -1: // no wildcard, exact match
                    match = (fullName == name);
                    break;
                case 0: // '*' matches all
                    match = true;
                    break;
                default: // match with wildcard at the end
                    match = fullName.startsWith(name.left(wildcard));
                }
                if (match) {
                    tags += index.tags;
                    const auto attributes = index.attributes;
                    for (const QString &attr : attributes) {
                        QLatin1Char div(':');
                        QStringList attrList = attr.split(div);
                        if (attrList.count() == 1)
                            attrList.append(QStringLiteral("true"));
                        QString attrName = attrList.takeFirst();
                        if (!usedAttributes.contains(attrName)) {
                            writer.writeAttribute(attrName, attrList.join(div));
                            usedAttributes << attrName;
                        }
                    }
                }
            }
        }

        writer.writeStartElement("description");
        Text brief = en->doc().briefText();
        if (!brief.isEmpty())
            writer.writeCDATA(brief.toString());
        else
            writer.writeCDATA(QString("No description available"));
        writer.writeEndElement(); // description

        // Add words from module name as tags
        // QtQuickControls -> qt,quick,controls
        // QtOpenGL -> qt,opengl
        QRegularExpression re("([A-Z]+[a-z0-9]*(3D|GL)?)");
        int pos = 0;
        QRegularExpressionMatch match;
        while ((match = re.match(m_project, pos)).hasMatch()) {
            tags << match.captured(1).toLower();
            pos = match.capturedEnd();
        }

        // Include tags added via \meta {tag} {tag1[,tag2,...]}
        // within \example topic
        const QStringMultiMap *metaTagMap = en->doc().metaTagMap();
        if (metaTagMap) {
            for (const auto &tag : metaTagMap->values("tag")) {
                const auto &tagList = tag.toLower().split(QLatin1Char(','));
                tags += QSet<QString>(tagList.cbegin(), tagList.cend());
            }
        }

        const auto &titleWords = en->title().toLower().split(QLatin1Char(' '));
        tags += QSet<QString>(titleWords.cbegin(), titleWords.cend());

        // Clean up tags, exclude invalid and common words
        QSet<QString>::iterator tag_it = tags.begin();
        QSet<QString> modified;
        while (tag_it != tags.end()) {
            QString s = *tag_it;
            if (s.at(0) == '(')
                s.remove(0, 1).chop(1);
            if (s.endsWith(QLatin1Char(':')))
                s.chop(1);

            if (s.length() < 2 || s.at(0).isDigit() || s.at(0) == '-' || s == QLatin1String("qt")
                || s == QLatin1String("the") || s == QLatin1String("and")
                || s.startsWith(QLatin1String("example")) || s.startsWith(QLatin1String("chapter")))
                tag_it = tags.erase(tag_it);
            else if (s != *tag_it) {
                modified << s;
                tag_it = tags.erase(tag_it);
            } else
                ++tag_it;
        }
        tags += modified;

        if (!tags.isEmpty()) {
            writer.writeStartElement("tags");
            bool wrote_one = false;
            QStringList sortedTags = tags.values();
            sortedTags.sort();
            for (const auto &tag : qAsConst(sortedTags)) {
                if (wrote_one)
                    writer.writeCharacters(",");
                writer.writeCharacters(tag);
                wrote_one = true;
            }
            writer.writeEndElement(); // tags
        }

        QString exampleName = en->name().mid(en->name().lastIndexOf('/') + 1);
        QMap<int, QString> filesToOpen;
        const auto files = en->files();
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

        for (auto it = filesToOpen.constEnd(); it != filesToOpen.constBegin();) {
            writer.writeStartElement("fileToOpen");
            if (--it == filesToOpen.constBegin()) {
                writer.writeAttribute(QStringLiteral("mainFile"), QStringLiteral("true"));
            }
            writer.writeCharacters(installPath + it.value());
            writer.writeEndElement();
        }

        writer.writeEndElement(); // example
    }

    writer.writeEndElement(); // examples
    writer.writeEndElement(); // instructionals
    writer.writeEndDocument();
    file.close();
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
    const QStringList names =
            config.getStringList(CONFIG_MANIFESTMETA + Config::dot + QStringLiteral("filters"));

    for (const auto &manifest : names) {
        ManifestMetaFilter filter;
        QString prefix = CONFIG_MANIFESTMETA + Config::dot + manifest + Config::dot;
        filter.names = config.getStringSet(prefix + QStringLiteral("names"));
        filter.attributes = config.getStringSet(prefix + QStringLiteral("attributes"));
        filter.tags = config.getStringSet(prefix + QStringLiteral("tags"));
        m_manifestMetaContent.append(filter);
    }
}

/*!
  Retrieve the install path for the \a example as specified with
  the \meta command, or fall back to the one defined in .qdocconf.
 */
QString ManifestWriter::retrieveInstallPath(const ExampleNode *example)
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
