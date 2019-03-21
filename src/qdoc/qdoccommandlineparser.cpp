/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#include "qdoccommandlineparser.h"

#include <QtCore/qdebug.h>
#include <QtCore/qfile.h>

#include "config.h"
#include "generator.h"
#include "loggingcategory.h"

QDocCommandLineParser::QDocCommandLineParser()
    : QCommandLineParser(),
      defineOption(QStringList() << QStringLiteral("D")),
      dependsOption(QStringList() << QStringLiteral("depends")),
      highlightingOption(QStringList() << QStringLiteral("highlighting")),
      showInternalOption(QStringList() << QStringLiteral("showinternal")),
      redirectDocumentationToDevNullOption(QStringList() << QStringLiteral("redirect-documentation-to-dev-null")),
      noExamplesOption(QStringList() << QStringLiteral("no-examples")),
      indexDirOption(QStringList() << QStringLiteral("indexdir")),
      installDirOption(QStringList() << QStringLiteral("installdir")),
      obsoleteLinksOption(QStringList() << QStringLiteral("obsoletelinks")),
      outputDirOption(QStringList() << QStringLiteral("outputdir")),
      outputFormatOption(QStringList() << QStringLiteral("outputformat")),
      noLinkErrorsOption(QStringList() << QStringLiteral("no-link-errors")),
      autoLinkErrorsOption(QStringList() << QStringLiteral("autolink-errors")),
      debugOption(QStringList() << QStringLiteral("debug")),
      prepareOption(QStringList() << QStringLiteral("prepare")),
      generateOption(QStringList() << QStringLiteral("generate")),
      logProgressOption(QStringList() << QStringLiteral("log-progress")),
      singleExecOption(QStringList() << QStringLiteral("single-exec")),
      writeQaPagesOption(QStringList() << QStringLiteral("write-qa-pages")),
      includePathOption("I", "Add dir to the include path for header files.", "path"),
      includePathSystemOption("isystem", "Add dir to the system include path for header files.", "path"),
      frameworkOption("F", "Add macOS framework to the include path for header files.", "framework"),
      timestampsOption(QStringList() << QStringLiteral("timestamps"))
{
    setApplicationDescription(QCoreApplication::translate("qdoc", "Qt documentation generator"));
    addHelpOption();
    addVersionOption();

    setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);

    addPositionalArgument("file1.qdocconf ...", QCoreApplication::translate("qdoc", "Input files"));

    defineOption.setDescription(QCoreApplication::translate("qdoc", "Define the argument as a macro while parsing sources"));
    defineOption.setValueName(QStringLiteral("macro[=def]"));
    addOption(defineOption);

    dependsOption.setDescription(QCoreApplication::translate("qdoc", "Specify dependent modules"));
    dependsOption.setValueName(QStringLiteral("module"));
    addOption(dependsOption);

    highlightingOption.setDescription(QCoreApplication::translate("qdoc", "Turn on syntax highlighting (makes qdoc run slower)"));
    addOption(highlightingOption);

    showInternalOption.setDescription(QCoreApplication::translate("qdoc", "Include content marked internal"));
    addOption(showInternalOption);

    redirectDocumentationToDevNullOption.setDescription(QCoreApplication::translate("qdoc", "Save all documentation content to /dev/null. Useful if someone is interested in qdoc errors only."));
    addOption(redirectDocumentationToDevNullOption);

    noExamplesOption.setDescription(QCoreApplication::translate("qdoc", "Do not generate documentation for examples"));
    addOption(noExamplesOption);

    indexDirOption.setDescription(QCoreApplication::translate("qdoc", "Specify a directory where QDoc should search for index files to load"));
    indexDirOption.setValueName(QStringLiteral("dir"));
    addOption(indexDirOption);

    installDirOption.setDescription(QCoreApplication::translate("qdoc", "Specify the directory where the output will be after running \"make install\""));
    installDirOption.setValueName(QStringLiteral("dir"));
    addOption(installDirOption);

    obsoleteLinksOption.setDescription(QCoreApplication::translate("qdoc", "Report links from obsolete items to non-obsolete items"));
    addOption(obsoleteLinksOption);

    outputDirOption.setDescription(QCoreApplication::translate("qdoc", "Specify output directory, overrides setting in qdocconf file"));
    outputDirOption.setValueName(QStringLiteral("dir"));
    addOption(outputDirOption);

    outputFormatOption.setDescription(QCoreApplication::translate("qdoc", "Specify output format, overrides setting in qdocconf file"));
    outputFormatOption.setValueName(QStringLiteral("format"));
    addOption(outputFormatOption);

    noLinkErrorsOption.setDescription(QCoreApplication::translate("qdoc", "Do not print link errors (i.e. missing targets)"));
    addOption(noLinkErrorsOption);

    autoLinkErrorsOption.setDescription(QCoreApplication::translate("qdoc", "Show errors when automatic linking fails"));
    addOption(autoLinkErrorsOption);

    debugOption.setDescription(QCoreApplication::translate("qdoc", "Enable debug output"));
    addOption(debugOption);

    prepareOption.setDescription(QCoreApplication::translate("qdoc", "Run qdoc only to generate an index file, not the docs"));
    addOption(prepareOption);

    generateOption.setDescription(QCoreApplication::translate("qdoc", "Run qdoc to read the index files and generate the docs"));
    addOption(generateOption);

    logProgressOption.setDescription(QCoreApplication::translate("qdoc", "Log progress on stderr."));
    addOption(logProgressOption);

    singleExecOption.setDescription(QCoreApplication::translate("qdoc", "Run qdoc once over all the qdoc conf files."));
    addOption(singleExecOption);

    writeQaPagesOption.setDescription(QCoreApplication::translate("qdoc", "Write QA pages."));
    addOption(writeQaPagesOption);

    includePathOption.setFlags(QCommandLineOption::ShortOptionStyle);
    addOption(includePathOption);

    addOption(includePathSystemOption);

    frameworkOption.setFlags(QCommandLineOption::ShortOptionStyle);
    addOption(frameworkOption);

    timestampsOption.setDescription(QCoreApplication::translate("qdoc", "Timestamp each qdoc log line."));
    addOption(timestampsOption);
}

void QDocCommandLineParser::process(const QCoreApplication &app, QDocGlobals &qdocGlobals)
{
    QCommandLineParser::process(app);

    qdocGlobals.addDefine(values(defineOption));
    qdocGlobals.dependModules() += values(dependsOption);
    qdocGlobals.enableHighlighting(isSet(highlightingOption));
    qdocGlobals.setShowInternal(isSet(showInternalOption));
    qdocGlobals.setSingleExec(isSet(singleExecOption));
    qdocGlobals.setWriteQaPages(isSet(writeQaPagesOption));
    qdocGlobals.setRedirectDocumentationToDevNull(isSet(redirectDocumentationToDevNullOption));
    Config::generateExamples = !isSet(noExamplesOption);
    foreach (const QString &indexDir, values(indexDirOption)) {
        if (QFile::exists(indexDir))
            qdocGlobals.appendToIndexDirs(indexDir);
        else
            qDebug() << "Cannot find index directory" << indexDir;
    }
    if (isSet(installDirOption))
        Config::installDir = value(installDirOption);
    qdocGlobals.setObsoleteLinks(isSet(obsoleteLinksOption));
    if (isSet(outputDirOption))
        Config::overrideOutputDir = value(outputDirOption);
    foreach (const QString &format, values(outputFormatOption))
        Config::overrideOutputFormats.insert(format);
    qdocGlobals.setNoLinkErrors(isSet(noLinkErrorsOption) || qEnvironmentVariableIsSet("QDOC_NOLINKERRORS"));
    qdocGlobals.setAutolinkErrors(isSet(autoLinkErrorsOption));
    if (isSet(debugOption))
        Generator::startDebugging(QString("command line"));
    qCDebug(lcQdoc).noquote() << "Arguments :" << QCoreApplication::arguments();

    if (isSet(prepareOption))
        Generator::setQDocPass(Generator::Prepare);
    if (isSet(generateOption))
        Generator::setQDocPass(Generator::Generate);
    if (isSet(singleExecOption)) {
        Generator::setSingleExec();
        if (isSet(indexDirOption))
            qDebug() << "WARNING: -indexdir option ignored: Index files are not used in -single-exec mode.";
    }
    if (isSet(writeQaPagesOption))
        Generator::setWriteQaPages();
    if (isSet(logProgressOption))
        Location::startLoggingProgress();
    if (isSet(timestampsOption))
        Generator::setUseTimestamps();

    QDir currentDir = QDir::current();
    const auto paths = values(includePathOption);
    for (const auto &i : paths)
        qdocGlobals.addIncludePath("-I", currentDir.absoluteFilePath(i));

#ifdef QDOC_PASS_ISYSTEM
    const auto paths2 = values(includePathSystemOption);
    for (const auto &i : paths2)
        qdocGlobals.addIncludePath("-isystem", currentDir.absoluteFilePath(i));
#endif
    const auto paths3 = values(frameworkOption);
    for (const auto &i : paths3)
        qdocGlobals.addIncludePath("-F", currentDir.absoluteFilePath(i));

    /*
      The default indent for code is 0.
      The default value for false is 0.
      The default supported file extensions are cpp, h, qdoc and qml.
      The default language is c++.
      The default output format is html.
      The default tab size is 8.
      And those are all the default values for configuration variables.
     */
    if (qdocGlobals.defaults().isEmpty()) {
        qdocGlobals.defaults().insert(CONFIG_CODEINDENT, QLatin1String("0"));
        qdocGlobals.defaults().insert(CONFIG_FALSEHOODS, QLatin1String("0"));
        qdocGlobals.defaults().insert(CONFIG_FILEEXTENSIONS, QLatin1String("*.cpp *.h *.qdoc *.qml"));
        qdocGlobals.defaults().insert(CONFIG_LANGUAGE, QLatin1String("Cpp"));
        qdocGlobals.defaults().insert(CONFIG_OUTPUTFORMATS, QLatin1String("HTML"));
        qdocGlobals.defaults().insert(CONFIG_TABSIZE, QLatin1String("8"));
    }
}
