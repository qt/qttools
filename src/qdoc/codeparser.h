// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

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
    virtual Node *parseFnArg(const Location &, const QString &, const QString & = QString())
    {
        return nullptr;
    }

    [[nodiscard]] const QString &currentFile() const { return m_currentFile; }
    [[nodiscard]] const QString &moduleHeader() const { return m_moduleHeader; }
    void setModuleHeader(const QString &t) { m_moduleHeader = t; }
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
    static void extractPageLinkAndDesc(QStringView arg, QString *link, QString *desc);
    static bool showInternal() { return s_showInternal; }
    QString m_moduleHeader {};
    QString m_currentFile {};
    QDocDatabase *m_qdb {};

private:
    static QList<CodeParser *> s_parsers;
    static bool s_showInternal;
};

#define COMMAND_ABSTRACT Doc::alias(QLatin1String("abstract"))
#define COMMAND_CLASS Doc::alias(QLatin1String("class"))
#define COMMAND_DEFAULT Doc::alias(QLatin1String("default"))
#define COMMAND_DEPRECATED Doc::alias(QLatin1String("deprecated")) // ### don't document
#define COMMAND_DONTDOCUMENT Doc::alias(QLatin1String("dontdocument"))
#define COMMAND_ENUM Doc::alias(QLatin1String("enum"))
#define COMMAND_EXAMPLE Doc::alias(QLatin1String("example"))
#define COMMAND_EXTERNALPAGE Doc::alias(QLatin1String("externalpage"))
#define COMMAND_FN Doc::alias(QLatin1String("fn"))
#define COMMAND_GROUP Doc::alias(QLatin1String("group"))
#define COMMAND_HEADERFILE Doc::alias(QLatin1String("headerfile"))
#define COMMAND_INGROUP Doc::alias(QLatin1String("ingroup"))
#define COMMAND_INHEADERFILE Doc::alias(QLatin1String("inheaderfile"))
#define COMMAND_INMODULE Doc::alias(QLatin1String("inmodule")) // ### don't document
#define COMMAND_INPUBLICGROUP Doc::alias(QLatin1String("inpublicgroup"))
#define COMMAND_INQMLMODULE Doc::alias(QLatin1String("inqmlmodule"))
#define COMMAND_INTERNAL Doc::alias(QLatin1String("internal"))
#define COMMAND_MACRO Doc::alias(QLatin1String("macro"))
#define COMMAND_MODULE Doc::alias(QLatin1String("module"))
#define COMMAND_MODULESTATE Doc::alias(QLatin1String("modulestate"))
#define COMMAND_NAMESPACE Doc::alias(QLatin1String("namespace"))
#define COMMAND_NEXTPAGE Doc::alias(QLatin1String("nextpage"))
#define COMMAND_NOAUTOLIST Doc::alias(QLatin1String("noautolist"))
#define COMMAND_NONREENTRANT Doc::alias(QLatin1String("nonreentrant"))
#define COMMAND_OBSOLETE Doc::alias(QLatin1String("obsolete"))
#define COMMAND_OVERLOAD Doc::alias(QLatin1String("overload"))
#define COMMAND_PAGE Doc::alias(QLatin1String("page"))
#define COMMAND_PRELIMINARY Doc::alias(QLatin1String("preliminary"))
#define COMMAND_PREVIOUSPAGE Doc::alias(QLatin1String("previouspage"))
#define COMMAND_PROPERTY Doc::alias(QLatin1String("property"))
#define COMMAND_QMLABSTRACT Doc::alias(QLatin1String("qmlabstract"))
#define COMMAND_QMLATTACHEDMETHOD Doc::alias(QLatin1String("qmlattachedmethod"))
#define COMMAND_QMLATTACHEDPROPERTY Doc::alias(QLatin1String("qmlattachedproperty"))
#define COMMAND_QMLATTACHEDSIGNAL Doc::alias(QLatin1String("qmlattachedsignal"))
#define COMMAND_QMLVALUETYPE Doc::alias(QLatin1String("qmlvaluetype"))
#define COMMAND_QMLCLASS Doc::alias(QLatin1String("qmlclass"))
#define COMMAND_QMLDEFAULT Doc::alias(QLatin1String("qmldefault"))
#define COMMAND_QMLINHERITS Doc::alias(QLatin1String("inherits"))
#define COMMAND_QMLINSTANTIATES Doc::alias(QLatin1String("instantiates"))
#define COMMAND_QMLMETHOD Doc::alias(QLatin1String("qmlmethod"))
#define COMMAND_QMLMODULE Doc::alias(QLatin1String("qmlmodule"))
#define COMMAND_QMLPROPERTY Doc::alias(QLatin1String("qmlproperty"))
#define COMMAND_QMLPROPERTYGROUP Doc::alias(QLatin1String("qmlpropertygroup"))
#define COMMAND_QMLREADONLY Doc::alias(QLatin1String("readonly"))
#define COMMAND_QMLREQUIRED Doc::alias(QLatin1String("required"))
#define COMMAND_QMLSIGNAL Doc::alias(QLatin1String("qmlsignal"))
#define COMMAND_QMLTYPE Doc::alias(QLatin1String("qmltype"))
#define COMMAND_QTCMAKEPACKAGE Doc::alias(QLatin1String("qtcmakepackage"))
#define COMMAND_QTVARIABLE Doc::alias(QLatin1String("qtvariable"))
#define COMMAND_REENTRANT Doc::alias(QLatin1String("reentrant"))
#define COMMAND_REIMP Doc::alias(QLatin1String("reimp"))
#define COMMAND_RELATES Doc::alias(QLatin1String("relates"))
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
#define COMMAND_ATTRIBUTION Doc::alias(QLatin1String("attribution"))

// deprecated alias of qmlvaluetype
#define COMMAND_QMLBASICTYPE Doc::alias(QLatin1String("qmlbasictype"))

QT_END_NAMESPACE

#endif
