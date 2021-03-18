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

#include "parameters.h"

#include "codechunk.h"
#include "generator.h"
#include "tokenizer.h"

QT_BEGIN_NAMESPACE

QRegExp Parameters::varComment_("/\\*\\s*([a-zA-Z_0-9]+)\\s*\\*/");

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
    QString p = type_;
    if (!p.endsWith(QChar('*')) && !p.endsWith(QChar('&')) && !p.endsWith(QChar(' ')))
        p += QLatin1Char(' ');
    p += name_;
    if (includeValue && !defaultValue_.isEmpty())
        p += " = " + defaultValue_;
    return p;
}

/*!
  \class Parameters

  \brief A class for parsing and managing a function parameter list

  The constructor is passed a string that is the text inside the
  parentheses of a function declaration. The constructor parses
  the parameter list into a vector of class Parameter.

  The Parameters object is then used in function searches to find
  the correct function node given the function name and the signature
  of its parameters.
 */

Parameters::Parameters() : valid_(true), privateSignal_(false), tok_(0), tokenizer_(nullptr)
{
    // nothing.
}

Parameters::Parameters(const QString &signature)
    : valid_(true), privateSignal_(false), tok_(0), tokenizer_(nullptr)
{
    if (!signature.isEmpty()) {
        if (!parse(signature)) {
            parameters_.clear();
            valid_ = false;
        }
    }
}

/*!
  Get the next token from the string being parsed and store
  it in the token variable.
 */
void Parameters::readToken()
{
    tok_ = tokenizer_->getToken();
}

/*!
  Return the current lexeme from the string being parsed.
 */
QString Parameters::lexeme()
{
    return tokenizer_->lexeme();
}

/*!
  Return the previous lexeme read from the string being parsed.
 */
QString Parameters::previousLexeme()
{
    return tokenizer_->previousLexeme();
}

/*!
  If the current token is \a target, read the next token and
  return \c true. Otherwise, return false without reading the
  next token.
 */
bool Parameters::match(int target)
{
    if (tok_ == target) {
        readToken();
        return true;
    }
    return false;
}

/*!
  Match a template clause in angle brackets, append it to the
  \a type, and return \c true. If there is no template clause,
  or if an error is detected, return \c false.
 */
void Parameters::matchTemplateAngles(CodeChunk &type)
{
    if (tok_ == Tok_LeftAngle) {
        int leftAngleDepth = 0;
        int parenAndBraceDepth = 0;
        do {
            if (tok_ == Tok_LeftAngle) {
                leftAngleDepth++;
            } else if (tok_ == Tok_RightAngle) {
                leftAngleDepth--;
            } else if (tok_ == Tok_LeftParen || tok_ == Tok_LeftBrace) {
                ++parenAndBraceDepth;
            } else if (tok_ == Tok_RightParen || tok_ == Tok_RightBrace) {
                if (--parenAndBraceDepth < 0)
                    return;
            }
            type.append(lexeme());
            readToken();
        } while (leftAngleDepth > 0 && tok_ != Tok_Eoi);
    }
}

/*!
  Uses the current tokenizer to parse the \a name and \a type
  of the parameter.
 */
bool Parameters::matchTypeAndName(CodeChunk &type, QString &name, bool qProp)
{
    /*
      This code is really hard to follow... sorry. The loop is there to match
      Alpha::Beta::Gamma::...::Omega.
    */
    for (;;) {
        bool virgin = true;

        if (tok_ != Tok_Ident) {
            /*
              There is special processing for 'Foo::operator int()'
              and such elsewhere. This is the only case where we
              return something with a trailing gulbrandsen ('Foo::').
            */
            if (tok_ == Tok_operator)
                return true;

            /*
              People may write 'const unsigned short' or
              'short unsigned const' or any other permutation.
            */
            while (match(Tok_const) || match(Tok_volatile))
                type.append(previousLexeme());
            QString pending;
            while (tok_ == Tok_signed || tok_ == Tok_int || tok_ == Tok_unsigned
                   || tok_ == Tok_short || tok_ == Tok_long || tok_ == Tok_int64) {
                if (tok_ == Tok_signed)
                    pending = lexeme();
                else {
                    if (tok_ == Tok_unsigned && !pending.isEmpty())
                        type.append(pending);
                    pending.clear();
                    type.append(lexeme());
                }
                readToken();
                virgin = false;
            }
            if (!pending.isEmpty()) {
                type.append(pending);
                pending.clear();
            }
            while (match(Tok_const) || match(Tok_volatile))
                type.append(previousLexeme());

            if (match(Tok_Tilde))
                type.append(previousLexeme());
        }

        if (virgin) {
            if (match(Tok_Ident)) {
                /*
                  This is a hack until we replace this "parser"
                  with the real one used in Qt Creator.
                  Is it still needed? mws 11/12/2018
                 */
                if (lexeme() == "("
                    && ((previousLexeme() == "QT_PREPEND_NAMESPACE")
                        || (previousLexeme() == "NS"))) {
                    readToken();
                    readToken();
                    type.append(previousLexeme());
                    readToken();
                } else
                    type.append(previousLexeme());
            } else if (match(Tok_void) || match(Tok_int) || match(Tok_char) || match(Tok_double)
                       || match(Tok_Ellipsis)) {
                type.append(previousLexeme());
            } else {
                return false;
            }
        } else if (match(Tok_int) || match(Tok_char) || match(Tok_double)) {
            type.append(previousLexeme());
        }

        matchTemplateAngles(type);

        while (match(Tok_const) || match(Tok_volatile))
            type.append(previousLexeme());

        if (match(Tok_Gulbrandsen))
            type.append(previousLexeme());
        else
            break;
    }

    while (match(Tok_Ampersand) || match(Tok_Aster) || match(Tok_const) || match(Tok_Caret)
           || match(Tok_Ellipsis))
        type.append(previousLexeme());

    if (match(Tok_LeftParenAster)) {
        /*
          A function pointer. This would be rather hard to handle without a
          tokenizer hack, because a type can be followed with a left parenthesis
          in some cases (e.g., 'operator int()'). The tokenizer recognizes '(*'
          as a single token.
        */
        type.append(" "); // force a space after the type
        type.append(previousLexeme());
        type.appendHotspot();
        if (match(Tok_Ident))
            name = previousLexeme();
        if (!match(Tok_RightParen))
            return false;
        type.append(previousLexeme());
        if (!match(Tok_LeftParen))
            return false;
        type.append(previousLexeme());

        /* parse the parameters. Ignore the parameter name from the type */
        while (tok_ != Tok_RightParen && tok_ != Tok_Eoi) {
            QString dummy;
            if (!matchTypeAndName(type, dummy))
                return false;
            if (match(Tok_Comma))
                type.append(previousLexeme());
        }
        if (!match(Tok_RightParen))
            return false;
        type.append(previousLexeme());
    } else {
        /*
          The common case: Look for an optional identifier, then for
          some array brackets.
        */
        type.appendHotspot();

        if (match(Tok_Ident)) {
            name = previousLexeme();
        } else if (match(Tok_Comment)) {
            /*
              A neat hack: Commented-out parameter names are
              recognized by qdoc. It's impossible to illustrate
              here inside a C-style comment, because it requires
              an asterslash. It's also impossible to illustrate
              inside a C++-style comment, because the explanation
              does not fit on one line.
            */
            if (varComment_.exactMatch(previousLexeme()))
                name = varComment_.cap(1);
        } else if (match(Tok_LeftParen)) {
            name = "(";
            while (tok_ != Tok_RightParen && tok_ != Tok_Eoi) {
                name.append(lexeme());
                readToken();
            }
            name.append(")");
            readToken();
            if (match(Tok_LeftBracket)) {
                name.append("[");
                while (tok_ != Tok_RightBracket && tok_ != Tok_Eoi) {
                    name.append(lexeme());
                    readToken();
                }
                name.append("]");
                readToken();
            }
        } else if (qProp && (match(Tok_default) || match(Tok_final) || match(Tok_override))) {
            // Hack to make 'default', 'final' and 'override'  work again in Q_PROPERTY
            name = previousLexeme();
        }

        if (tok_ == Tok_LeftBracket) {
            int bracketDepth0 = tokenizer_->bracketDepth();
            while ((tokenizer_->bracketDepth() >= bracketDepth0 && tok_ != Tok_Eoi)
                   || tok_ == Tok_RightBracket) {
                type.append(lexeme());
                readToken();
            }
        }
    }
    return true;
}

/*!
  Parse the next function parameter, if there is one, and
  append it to the internal parameter vector. Return true
  if a parameter is parsed correctly. Otherwise return false.
 */
bool Parameters::matchParameter()
{
    if (match(Tok_QPrivateSignal)) {
        privateSignal_ = true;
        return true;
    }

    CodeChunk chunk;
    QString name;
    if (!matchTypeAndName(chunk, name))
        return false;
    QString type = chunk.toString();
    QString defaultValue;
    match(Tok_Comment);
    if (match(Tok_Equal)) {
        chunk.clear();
        int pdepth = tokenizer_->parenDepth();
        while (tokenizer_->parenDepth() >= pdepth
               && (tok_ != Tok_Comma || (tokenizer_->parenDepth() > pdepth)) && tok_ != Tok_Eoi) {
            chunk.append(lexeme());
            readToken();
        }
        defaultValue = chunk.toString();
    }
    append(type, name, defaultValue);
    return true;
}

/*!
  This function uses a Tokenizer to parse the \a signature,
  which is a comma-separated list of parameter declarations.
  If an error is detected, the Parameters object is cleared
  and \c false is returned. Otherwise \c true is returned.
 */
bool Parameters::parse(const QString &signature)
{
    Tokenizer *outerTokenizer = tokenizer_;
    int outerTok = tok_;

    QByteArray latin1 = signature.toLatin1();
    Tokenizer stringTokenizer(Location(), latin1);
    stringTokenizer.setParsingFnOrMacro(true);
    tokenizer_ = &stringTokenizer;

    readToken();
    do {
        if (!matchParameter()) {
            parameters_.clear();
            valid_ = false;
            break;
        }
    } while (match(Tok_Comma));

    tokenizer_ = outerTokenizer;
    tok_ = outerTok;
    return valid_;
}

/*!
  Append a Parameter constructed from \a type, \a name, and \a value
  to the parameter vector.
 */
void Parameters::append(const QString &type, const QString &name, const QString &value)
{
    parameters_.append(Parameter(type, name, value));
}

/*!
  Returns the list of reconstructed parameters. If \a includeValues
  is true, the default values are included, if any are present.
 */
QString Parameters::signature(bool includeValues) const
{
    QString result;
    if (parameters_.size() > 0) {
        for (int i = 0; i < parameters_.size(); i++) {
            if (i > 0)
                result += ", ";
            result += parameters_.at(i).signature(includeValues);
        }
    }
    return result;
}

/*!
  Returns the signature of all the parameters with all the
  spaces and commas removed. It is unintelligible, but that
  is what the caller wants.

  If \a names is true, the parameter names are included. If
  \a values is true, the default values are included.
 */
QString Parameters::rawSignature(bool names, bool values) const
{
    QString raw;
    const auto params = parameters_;
    for (const auto &parameter : params) {
        raw += parameter.type();
        if (names)
            raw += parameter.name();
        if (values)
            raw += parameter.defaultValue();
    }
    return raw;
}

/*!
  Parse the parameter \a signature by splitting the string,
  and store the individual parameters in the parameter vector.

  This method of parsing is naive but sufficient for QML methods
  and macros.
 */
void Parameters::set(const QString &signature)
{
    clear();
    if (!signature.isEmpty()) {
        QStringList commaSplit = signature.split(',');
        parameters_.resize(commaSplit.size());
        int i = 0;
        for (const auto &item : qAsConst(commaSplit)) {
            QStringList blankSplit = item.split(' ', Qt::SkipEmptyParts);
            QString pDefault;
            int defaultIdx = blankSplit.indexOf(QStringLiteral("="));
            if (defaultIdx != -1) {
                if (++defaultIdx < blankSplit.size())
                    pDefault = blankSplit.mid(defaultIdx).join(' ');
                blankSplit = blankSplit.mid(0, defaultIdx - 1);
            }
            QString pName = blankSplit.takeLast();
            QString pType = blankSplit.join(' ');
            if (pType.isEmpty() && pName == QLatin1String("..."))
                qSwap(pType, pName);
            else {
                int j = 0;
                while (j < pName.length() && !pName.at(j).isLetter())
                    j++;
                if (j > 0) {
                    pType += QChar(' ') + pName.left(j);
                    pName = pName.mid(j);
                }
            }
            parameters_[i++].set(pType, pName, pDefault);
        }
    }
}

/*!
  Insert all the parameter names into names.
 */
QSet<QString> Parameters::getNames() const
{
    QSet<QString> names;
    const auto params = parameters_;
    for (const auto &parameter : params) {
        if (!parameter.name().isEmpty())
            names.insert(parameter.name());
    }
    return names;
}

/*!
  Construct a list of the parameter types and return it.
 */
QString Parameters::generateTypeList() const
{
    QString out;
    if (count() > 0) {
        for (int i = 0; i < count(); ++i) {
            if (i > 0)
                out += ", ";
            out += parameters_.at(i).type();
        }
    }
    return out;
}

/*!
  Construct a list of the parameter type/name pairs and
  return it.
*/
QString Parameters::generateTypeAndNameList() const
{
    QString out;
    if (count() > 0) {
        for (int i = 0; i < count(); ++i) {
            if (i != 0)
                out += ", ";
            const Parameter &p = parameters_.at(i);
            out += p.type();
            if (out[out.size() - 1].isLetterOrNumber())
                out += QLatin1Char(' ');
            out += p.name();
        }
    }
    return out;
}

/*!
  Returns true if \a parameters contains the same parameter
  signature as this.
 */
bool Parameters::match(const Parameters &parameters) const
{
    if (count() != parameters.count())
        return false;
    if (count() == 0)
        return true;
    for (int i = 0; i < count(); i++) {
        if (parameters.at(i).type() != parameters_.at(i).type())
            return false;
    }
    return true;
}

QT_END_NAMESPACE
