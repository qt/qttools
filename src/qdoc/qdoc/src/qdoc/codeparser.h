// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef CODEPARSER_H
#define CODEPARSER_H

#include "node.h"

#include <QtCore/qset.h>

QT_BEGIN_NAMESPACE

#define COMMAND_ABSTRACT QLatin1String("abstract")
#define COMMAND_CLASS QLatin1String("class")
#define COMMAND_CMAKEPACKAGE QLatin1String("cmakepackage")
#define COMMAND_CMAKECOMPONENT QLatin1String("cmakecomponent")
#define COMMAND_CMAKETARGETITEM QLatin1String("cmaketargetitem")
#define COMMAND_COMPARES QLatin1String("compares")
#define COMMAND_COMPARESWITH QLatin1String("compareswith")
#define COMMAND_DEFAULT QLatin1String("default")
#define COMMAND_DEPRECATED QLatin1String("deprecated") // ### don't document
#define COMMAND_DONTDOCUMENT QLatin1String("dontdocument")
#define COMMAND_ENUM QLatin1String("enum")
#define COMMAND_EXAMPLE QLatin1String("example")
#define COMMAND_EXTERNALPAGE QLatin1String("externalpage")
#define COMMAND_FN QLatin1String("fn")
#define COMMAND_GROUP QLatin1String("group")
#define COMMAND_HEADERFILE QLatin1String("headerfile")
#define COMMAND_INGROUP QLatin1String("ingroup")
#define COMMAND_INHEADERFILE QLatin1String("inheaderfile")
#define COMMAND_INMODULE QLatin1String("inmodule") // ### don't document
#define COMMAND_INPUBLICGROUP QLatin1String("inpublicgroup")
#define COMMAND_INQMLMODULE QLatin1String("inqmlmodule")
#define COMMAND_INTERNAL QLatin1String("internal")
#define COMMAND_MACRO QLatin1String("macro")
#define COMMAND_MODULE QLatin1String("module")
#define COMMAND_MODULESTATE QLatin1String("modulestate")
#define COMMAND_NAMESPACE QLatin1String("namespace")
#define COMMAND_NEXTPAGE QLatin1String("nextpage")
#define COMMAND_NOAUTOLIST QLatin1String("noautolist")
#define COMMAND_NONREENTRANT QLatin1String("nonreentrant")
#define COMMAND_OBSOLETE QLatin1String("obsolete")
#define COMMAND_OVERLOAD QLatin1String("overload")
#define COMMAND_PAGE QLatin1String("page")
#define COMMAND_PRELIMINARY QLatin1String("preliminary")
#define COMMAND_PREVIOUSPAGE QLatin1String("previouspage")
#define COMMAND_PROPERTY QLatin1String("property")
#define COMMAND_QMLABSTRACT QLatin1String("qmlabstract")
#define COMMAND_QMLATTACHEDMETHOD QLatin1String("qmlattachedmethod")
#define COMMAND_QMLATTACHEDPROPERTY QLatin1String("qmlattachedproperty")
#define COMMAND_QMLATTACHEDSIGNAL QLatin1String("qmlattachedsignal")
#define COMMAND_QMLVALUETYPE QLatin1String("qmlvaluetype")
#define COMMAND_QMLCLASS QLatin1String("qmlclass")
#define COMMAND_QMLDEFAULT QLatin1String("qmldefault")
#define COMMAND_QMLENUM QLatin1String("qmlenum")
#define COMMAND_QMLENUMERATORSFROM QLatin1String("qmlenumeratorsfrom")
#define COMMAND_QMLINHERITS QLatin1String("inherits")
#define COMMAND_QMLINSTANTIATES QLatin1String("instantiates") // TODO Qt 7.0.0 - Remove: Deprecated since 6.8.
#define COMMAND_QMLMETHOD QLatin1String("qmlmethod")
#define COMMAND_QMLMODULE QLatin1String("qmlmodule")
#define COMMAND_QMLNATIVETYPE QLatin1String("nativetype")
#define COMMAND_QMLPROPERTY QLatin1String("qmlproperty")
#define COMMAND_QMLPROPERTYGROUP QLatin1String("qmlpropertygroup")
#define COMMAND_QMLREADONLY QLatin1String("readonly")
#define COMMAND_QMLREQUIRED QLatin1String("required")
#define COMMAND_QMLSIGNAL QLatin1String("qmlsignal")
#define COMMAND_QMLTYPE QLatin1String("qmltype")
#define COMMAND_QTCMAKEPACKAGE QLatin1String("qtcmakepackage")
#define COMMAND_QTCMAKETARGETITEM QLatin1String("qtcmaketargetitem")
#define COMMAND_QTVARIABLE QLatin1String("qtvariable")
#define COMMAND_REENTRANT QLatin1String("reentrant")
#define COMMAND_REIMP QLatin1String("reimp")
#define COMMAND_RELATES QLatin1String("relates")
#define COMMAND_SINCE QLatin1String("since")
#define COMMAND_STRUCT QLatin1String("struct")
#define COMMAND_SUBTITLE QLatin1String("subtitle")
#define COMMAND_STARTPAGE QLatin1String("startpage")
#define COMMAND_THREADSAFE QLatin1String("threadsafe")
#define COMMAND_TITLE QLatin1String("title")
#define COMMAND_TYPEALIAS QLatin1String("typealias")
#define COMMAND_TYPEDEF QLatin1String("typedef")
#define COMMAND_VARIABLE QLatin1String("variable")
#define COMMAND_VERSION QLatin1String("version")
#define COMMAND_UNION QLatin1String("union")
#define COMMAND_WRAPPER QLatin1String("wrapper")
#define COMMAND_ATTRIBUTION QLatin1String("attribution")

// deprecated alias of qmlvaluetype
#define COMMAND_QMLBASICTYPE QLatin1String("qmlbasictype")

class Location;
class QString;
class QDocDatabase;
class CppCodeParser;

struct UntiedDocumentation {
    Doc documentation;
    QStringList context;
};

struct TiedDocumentation {
    Doc documentation;
    Node* node;
};

class CodeParser
{
public:
    static inline const QSet<QString> common_meta_commands{
        COMMAND_ABSTRACT, COMMAND_CMAKEPACKAGE, COMMAND_CMAKECOMPONENT, COMMAND_CMAKETARGETITEM, COMMAND_DEFAULT,
        COMMAND_DEPRECATED, COMMAND_INGROUP, COMMAND_INMODULE, COMMAND_INPUBLICGROUP, COMMAND_INQMLMODULE,
        COMMAND_INTERNAL, COMMAND_MODULESTATE, COMMAND_NOAUTOLIST, COMMAND_NONREENTRANT, COMMAND_OBSOLETE,
        COMMAND_PRELIMINARY, COMMAND_QMLABSTRACT, COMMAND_QMLDEFAULT, COMMAND_QMLENUMERATORSFROM, COMMAND_QMLINHERITS,
        COMMAND_QMLREADONLY, COMMAND_QMLREQUIRED, COMMAND_QTCMAKEPACKAGE, COMMAND_QTCMAKETARGETITEM,
        COMMAND_QTVARIABLE, COMMAND_REENTRANT, COMMAND_SINCE, COMMAND_STARTPAGE, COMMAND_SUBTITLE,
        COMMAND_THREADSAFE, COMMAND_TITLE, COMMAND_WRAPPER, COMMAND_ATTRIBUTION,
    };

public:
    CodeParser();
    virtual ~CodeParser();

    virtual void initializeParser() = 0;
    virtual void terminateParser();
    virtual QString language() = 0;
    virtual QStringList sourceFileNameFilter() = 0;
    virtual void parseSourceFile(const Location &location, const QString &filePath, CppCodeParser& cpp_code_parser) = 0;

    static void initialize();
    static void terminate();
    static CodeParser *parserForLanguage(const QString &language);
    static CodeParser *parserForSourceFile(const QString &filePath);
    static void setLink(Node *node, Node::LinkType linkType, const QString &arg);
    static bool isWorthWarningAbout(const Doc &doc);

protected:
    static void extractPageLinkAndDesc(QStringView arg, QString *link, QString *desc);
    QDocDatabase *m_qdb {};

private:
    static QList<CodeParser *> s_parsers;
};

QT_END_NAMESPACE

#endif
