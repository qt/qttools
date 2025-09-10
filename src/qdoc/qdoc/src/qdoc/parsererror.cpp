// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "parsererror.h"
#include "node.h"
#include "qdocdatabase.h"
#include "config.h"
#include "utilities.h"

#include <QtCore/qregularexpression.h>

using namespace Qt::Literals::StringLiterals;

QT_BEGIN_NAMESPACE

/*!
    \class FnMatchError
    \brief Encapsulates information about \fn match error during parsing.
*/

/*!
    \variable FnMatchError::signature

    Signature for the \fn topic that failed to match.
*/

/*!
    \relates FnMatchError

    Returns \c true if any parent of a C++ function represented by
    \a signature is documented as \\internal.
*/
bool isParentInternal(const QString &signature)
{
    const QRegularExpression scoped_fn{R"((?:\w+(?:<[^>]+>)?::)+~?\w\S*\()"};
    auto match = scoped_fn.match(signature);
    if (!match.isValid())
        return false;

    auto scope = match.captured().split("::"_L1);
    scope.removeLast(); // Drop function name

    for (auto &s : scope)
        if (qsizetype pos = s.indexOf('<'); pos >= 0)
            s.truncate(pos);

    auto parent = QDocDatabase::qdocDB()->findNodeByNameAndType(scope, &Node::isCppNode);
    if (parent && !(parent->isClassNode() || parent->isNamespace())) {
        qCDebug(lcQdoc).noquote()
                << "Invalid scope:" << qPrintable(parent->nodeTypeString())
                << qPrintable(parent->fullName())
                << "for \\fn" << qPrintable(signature);
        return false;
    }

    while (parent) {
        if (parent->isInternal())
            return true;
        parent = parent->parent();
    }

    return false;
}

/*!
    \class ParserErrorHandler
    \brief Processes parser errors and outputs warnings for them.
*/

/*!
    Generates a warning specific to FnMatchError.

    Warnings for internal documentation are omitted. Specifically, this
    (omission) happens if:

    \list
        \li \c {--showinternal} command line option is \b not
            used, and
        \li The warning is for an \\fn that is declared
            under a namespace/class that is documented as
            \\internal.
    \endlist
*/
void ParserErrorHandler::operator()(const FnMatchError &e) const
{
    if (Config::instance().showInternal() || !isParentInternal(e.signature))
        e.location.warning("Failed to find function when parsing \\fn %1"_L1.arg(e.signature));
}

QT_END_NAMESPACE
