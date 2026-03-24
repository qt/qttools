// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "parameter.h"

#include <QtCore/qregularexpression.h>

QT_BEGIN_NAMESPACE

/*!
  \class Parameter
  \brief The Parameter class describes one function parameter.

  A parameter can be a function parameter or a macro parameter.
  It has a name, a data type, and an optional default value.
  These are all stored as strings so they can be compared with
  a parameter in a function signature to find a match.
 */

/*!
  \fn Parameter::Parameter(const QString &type, const QString &name, const QString &defaultValue)

  Constructs the parameter from the \a type, the optional \a name,
  and the optional \a defaultValue.
 */

/*!
  Reconstructs the text signature for the parameter and returns
  it. If \a includeValue is true and there is a default value,
  the default value is appended with '='.
 */
QString Parameter::signature(bool includeValue) const
{
    QString p = m_type;
    if (!m_name.isEmpty()) {
        // For types with inside-out declarator syntax (such as references
        // to arrays, pointers to functions, or pointers to members), the
        // parameter name must be inserted inside the type rather than
        // appended after it.
        // Clang produces types like "const char (&)[Size]" or
        // "void (Cls::*)(int)" where the name belongs before the closing
        // paren: "const char (&data)[Size]", "void (Cls::*cb)(int)".
        static const QRegularExpression insideOutDeclarator(
                QStringLiteral(R"((\([^)]*[&*]\))\s*(\[|\())"));
        auto match = insideOutDeclarator.match(p);
        if (match.hasMatch()) {
            // Insert name before the closing paren of the declarator group.
            qsizetype insertPos = match.capturedStart(1) + match.capturedLength(1) - 1;
            p.insert(insertPos, m_name);
        } else {
            if (!p.isEmpty() && !p.endsWith(QChar('*')) && !p.endsWith(QChar('&'))
                && !p.endsWith(QChar(' '))) {
                p += QLatin1Char(' ');
            }
            p += m_name;
        }
    }
    if (includeValue && !m_defaultValue.isEmpty())
        p += " = " + m_defaultValue;
    return p;
}

QT_END_NAMESPACE

