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

#ifndef CODEPARSER_H
#define CODEPARSER_H

#include "node.h"

#include <QtCore/qset.h>

QT_BEGIN_NAMESPACE

class Location;
class QString;
class QDocDatabase;

class CodeParser
{
    Q_DECLARE_TR_FUNCTIONS(QDoc::CppCodeParser)

public:
    CodeParser();
    virtual ~CodeParser();

    virtual void initializeParser();
    virtual void terminateParser();
    virtual QString language() = 0;
    virtual QStringList headerFileNameFilter();
    virtual QStringList sourceFileNameFilter() = 0;
    virtual void parseHeaderFile(const Location &location, const QString &filePath);
    virtual void parseSourceFile(const Location &location, const QString &filePath) = 0;
    virtual void precompileHeaders() {}
    virtual Node *parseFnArg(const Location &, const QString &) { return nullptr; }

    bool isParsingH() const;
    bool isParsingCpp() const;
    bool isParsingQdoc() const;
    const QString &currentFile() const { return currentFile_; }
    const QString &moduleHeader() const { return moduleHeader_; }
    void setModuleHeader(const QString &t) { moduleHeader_ = t; }
    void checkModuleInclusion(Node *n);

    static void initialize();
    static void terminate();
    static CodeParser *parserForLanguage(const QString &language);
    static CodeParser *parserForHeaderFile(const QString &filePath);
    static CodeParser *parserForSourceFile(const QString &filePath);
    static void setLink(Node *node, Node::LinkType linkType, const QString &arg);
    static bool isWorthWarningAbout(const Doc &doc);

protected:
    const QSet<QString> &commonMetaCommands();
    static void extractPageLinkAndDesc(const QString &arg, QString *link, QString *desc);
    static bool showInternal() { return showInternal_; }
    QString moduleHeader_;
    QString currentFile_;
    QDocDatabase *qdb_;

private:
    static QVector<CodeParser *> parsers;
    static bool showInternal_;
    static bool singleExec_;
};

#define COMMAND_ABSTRACT Doc::alias(QLatin1String("abstract"))
#define COMMAND_AUDIENCE Doc::alias(QLatin1String("audience"))
#define COMMAND_AUTHOR Doc::alias(QLatin1String("author"))
#define COMMAND_CATEGORY Doc::alias(QLatin1String("category"))
#define COMMAND_CLASS Doc::alias(QLatin1String("class"))
#define COMMAND_COMPONENT Doc::alias(QLatin1String("component"))
#define COMMAND_CONTENTSPAGE Doc::alias(QLatin1String("contentspage"))
#define COMMAND_COPYRHOLDER Doc::alias(QLatin1String("copyrholder"))
#define COMMAND_COPYRYEAR Doc::alias(QLatin1String("copyryear"))
#define COMMAND_DEPRECATED Doc::alias(QLatin1String("deprecated")) // ### don't document
#define COMMAND_DONTDOCUMENT Doc::alias(QLatin1String("dontdocument"))
#define COMMAND_DITAMAP Doc::alias(QLatin1String("ditamap"))
#define COMMAND_ENUM Doc::alias(QLatin1String("enum"))
#define COMMAND_EXAMPLE Doc::alias(QLatin1String("example"))
#define COMMAND_EXTERNALPAGE Doc::alias(QLatin1String("externalpage"))
#define COMMAND_FN Doc::alias(QLatin1String("fn"))
#define COMMAND_GROUP Doc::alias(QLatin1String("group"))
#define COMMAND_HEADERFILE Doc::alias(QLatin1String("headerfile"))
#define COMMAND_INGROUP Doc::alias(QLatin1String("ingroup"))
#define COMMAND_INHEADERFILE Doc::alias(QLatin1String("inheaderfile"))
#define COMMAND_INJSMODULE Doc::alias(QLatin1String("injsmodule"))
#define COMMAND_INMODULE Doc::alias(QLatin1String("inmodule")) // ### don't document
#define COMMAND_INPUBLICGROUP Doc::alias(QLatin1String("inpublicgroup"))
#define COMMAND_INQMLMODULE Doc::alias(QLatin1String("inqmlmodule"))
#define COMMAND_INTERNAL Doc::alias(QLatin1String("internal"))
#define COMMAND_JSATTACHEDMETHOD Doc::alias(QLatin1String("jsattachedmethod"))
#define COMMAND_JSATTACHEDPROPERTY Doc::alias(QLatin1String("jsattachedproperty"))
#define COMMAND_JSATTACHEDSIGNAL Doc::alias(QLatin1String("jsattachedsignal"))
#define COMMAND_JSBASICTYPE Doc::alias(QLatin1String("jsbasictype"))
#define COMMAND_JSMETHOD Doc::alias(QLatin1String("jsmethod"))
#define COMMAND_JSMODULE Doc::alias(QLatin1String("jsmodule"))
#define COMMAND_JSPROPERTY Doc::alias(QLatin1String("jsproperty"))
#define COMMAND_JSPROPERTYGROUP Doc::alias(QLatin1String("jspropertygroup"))
#define COMMAND_JSSIGNAL Doc::alias(QLatin1String("jssignal"))
#define COMMAND_JSTYPE Doc::alias(QLatin1String("jstype"))
#define COMMAND_LICENSEDESCRIPTION Doc::alias(QLatin1String("licensedescription"))
#define COMMAND_LICENSENAME Doc::alias(QLatin1String("licensename"))
#define COMMAND_LICENSEYEAR Doc::alias(QLatin1String("licenseyear"))
#define COMMAND_LIFECYCLEVERSION Doc::alias(QLatin1String("lifecycleversion"))
#define COMMAND_LIFECYCLEWSTATUS Doc::alias(QLatin1String("lifecyclestatus"))
#define COMMAND_MACRO Doc::alias(QLatin1String("macro"))
#define COMMAND_MAINCLASS Doc::alias(QLatin1String("mainclass"))
#define COMMAND_MODULE Doc::alias(QLatin1String("module"))
#define COMMAND_NAMESPACE Doc::alias(QLatin1String("namespace"))
#define COMMAND_NEXTPAGE Doc::alias(QLatin1String("nextpage"))
#define COMMAND_NOAUTOLIST Doc::alias(QLatin1String("noautolist"))
#define COMMAND_NONREENTRANT Doc::alias(QLatin1String("nonreentrant"))
#define COMMAND_OBSOLETE Doc::alias(QLatin1String("obsolete"))
#define COMMAND_OVERLOAD Doc::alias(QLatin1String("overload"))
#define COMMAND_PAGE Doc::alias(QLatin1String("page"))
#define COMMAND_PERMISSIONS Doc::alias(QLatin1String("permissions"))
#define COMMAND_PRELIMINARY Doc::alias(QLatin1String("preliminary"))
#define COMMAND_PREVIOUSPAGE Doc::alias(QLatin1String("previouspage"))
#define COMMAND_PRODNAME Doc::alias(QLatin1String("prodname"))
#define COMMAND_PROPERTY Doc::alias(QLatin1String("property"))
#define COMMAND_PUBLISHER Doc::alias(QLatin1String("publisher"))
#define COMMAND_QMLABSTRACT Doc::alias(QLatin1String("qmlabstract"))
#define COMMAND_QMLATTACHEDMETHOD Doc::alias(QLatin1String("qmlattachedmethod"))
#define COMMAND_QMLATTACHEDPROPERTY Doc::alias(QLatin1String("qmlattachedproperty"))
#define COMMAND_QMLATTACHEDSIGNAL Doc::alias(QLatin1String("qmlattachedsignal"))
#define COMMAND_QMLBASICTYPE Doc::alias(QLatin1String("qmlbasictype"))
#define COMMAND_QMLCLASS Doc::alias(QLatin1String("qmlclass"))
#define COMMAND_QMLDEFAULT Doc::alias(QLatin1String("default"))
#define COMMAND_QMLINHERITS Doc::alias(QLatin1String("inherits"))
#define COMMAND_QMLINSTANTIATES Doc::alias(QLatin1String("instantiates"))
#define COMMAND_QMLMETHOD Doc::alias(QLatin1String("qmlmethod"))
#define COMMAND_QMLMODULE Doc::alias(QLatin1String("qmlmodule"))
#define COMMAND_QMLPROPERTY Doc::alias(QLatin1String("qmlproperty"))
#define COMMAND_QMLPROPERTYGROUP Doc::alias(QLatin1String("qmlpropertygroup"))
#define COMMAND_QMLREADONLY Doc::alias(QLatin1String("readonly"))
#define COMMAND_QMLSIGNAL Doc::alias(QLatin1String("qmlsignal"))
#define COMMAND_QMLTYPE Doc::alias(QLatin1String("qmltype"))
#define COMMAND_QTVARIABLE Doc::alias(QLatin1String("qtvariable"))
#define COMMAND_REENTRANT Doc::alias(QLatin1String("reentrant"))
#define COMMAND_REIMP Doc::alias(QLatin1String("reimp"))
#define COMMAND_RELATES Doc::alias(QLatin1String("relates"))
#define COMMAND_RELEASEDATE Doc::alias(QLatin1String("releasedate"))
#define COMMAND_SINCE Doc::alias(QLatin1String("since"))
#define COMMAND_STRUCT Doc::alias(QLatin1String("struct"))
#define COMMAND_SUBTITLE Doc::alias(QLatin1String("subtitle"))
#define COMMAND_STARTPAGE Doc::alias(QLatin1String("startpage"))
#define COMMAND_THREADSAFE Doc::alias(QLatin1String("threadsafe"))
#define COMMAND_TITLE Doc::alias(QLatin1String("title"))
#define COMMAND_TYPEALIAS Doc::alias(QLatin1String("typealias"))
#define COMMAND_TYPEDEF Doc::alias(QLatin1String("typedef"))
#define COMMAND_VARIABLE Doc::alias(QLatin1String("variable"))
#define COMMAND_VERSION Doc::alias(QLatin1String("version"))
#define COMMAND_UNION Doc::alias(QLatin1String("union"))
#define COMMAND_WRAPPER Doc::alias(QLatin1String("wrapper"))

QT_END_NAMESPACE

#endif
