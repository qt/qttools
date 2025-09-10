// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "codechunk.h"

QT_BEGIN_NAMESPACE

enum { Other, Alnum, Gizmo, Comma, LBrace, RBrace, RAngle, Colon, Paren };

// entries 128 and above are Other
static const int charCategory[256] = { Other, Other, Other, Other, Other, Other, Other, Other,
                                       Other, Other, Other, Other, Other, Other, Other, Other,
                                       Other, Other, Other, Other, Other, Other, Other, Other,
                                       Other, Other, Other, Other, Other, Other, Other, Other,
                                       //          !       "       #       $       %       &       '
                                       Other, Other, Other, Other, Other, Gizmo, Gizmo, Other,
                                       //  (       )       *       +       ,       -       .       /
                                       Paren, Paren, Gizmo, Gizmo, Comma, Other, Other, Gizmo,
                                       //  0       1       2       3       4       5       6       7
                                       Alnum, Alnum, Alnum, Alnum, Alnum, Alnum, Alnum, Alnum,
                                       //  8       9       :       ;       <       =       >       ?
                                       Alnum, Alnum, Colon, Other, Other, Gizmo, RAngle, Gizmo,
                                       //  @       A       B       C       D       E       F       G
                                       Other, Alnum, Alnum, Alnum, Alnum, Alnum, Alnum, Alnum,
                                       //  H       I       J       K       L       M       N       O
                                       Alnum, Alnum, Alnum, Alnum, Alnum, Alnum, Alnum, Alnum,
                                       //  P       Q       R       S       T       U       V       W
                                       Alnum, Alnum, Alnum, Alnum, Alnum, Alnum, Alnum, Alnum,
                                       //  X       Y       Z       [       \       ]       ^       _
                                       Alnum, Alnum, Alnum, Other, Other, Other, Gizmo, Alnum,
                                       //  `       a       b       c       d       e       f       g
                                       Other, Alnum, Alnum, Alnum, Alnum, Alnum, Alnum, Alnum,
                                       //  h       i       j       k       l       m       n       o
                                       Alnum, Alnum, Alnum, Alnum, Alnum, Alnum, Alnum, Alnum,
                                       //  p       q       r       s       t       u       v       w
                                       Alnum, Alnum, Alnum, Alnum, Alnum, Alnum, Alnum, Alnum,
                                       //  x       y       z       {       |       }       ~
                                       Alnum, Alnum, Alnum, LBrace, Gizmo, RBrace, Other, Other };

static const bool needSpace[9][9] = {
    /*        [      a      +      ,      {       }     >      :      )    */
    /* [ */ { false, false, false, false, false, true, false, false, false },
    /* a */ { false, true, true, false, false, true, false, false, false },
    /* + */ { false, true, false, false, false, true, false, true, false },
    /* , */ { true, true, true, true, true, true, true, true, false },
    /* { */ { false, false, false, false, false, false, false, false, false },
    /* } */ { false, false, false, false, false, false, false, false, false },
    /* > */ { true, true, true, false, true, true, true, false, false },
    /* : */ { false, false, true, true, true, true, true, false, false },
    /* ( */ { false, false, false, false, false, false, false, false, false },
};

static int category(QChar ch)
{
    return charCategory[static_cast<int>(ch.toLatin1())];
}

/*!
  \class CodeChunk

  \brief The CodeChunk class represents a tiny piece of C++ code.

  \note I think this class should be eliminated (mws 11/12/2018

  The class provides conversion between a list of lexemes and a string.  It adds
  spaces at the right place for consistent style.  The tiny pieces of code it
  represents are data types, enum values, and default parameter values.

  Apart from the piece of code itself, there are two bits of metainformation
  stored in CodeChunk: the base and the hotspot.  The base is the part of the
  piece that may be a hypertext link.  The base of

      QMap<QString, QString>

  is QMap.

  The hotspot is the place the variable name should be inserted in the case of a
  variable (or parameter) declaration.  The hotspot of

      char * []

  is between '*' and '[]'.
*/

/*!
  Appends \a lexeme to the current string contents, inserting
  a space if appropriate.
 */
void CodeChunk::append(const QString &lexeme)
{
    if (!m_str.isEmpty() && !lexeme.isEmpty()) {
        /*
          Should there be a space or not between the code chunk so far and the
          new lexeme?
        */
        int cat1 = category(m_str.at(m_str.size() - 1));
        int cat2 = category(lexeme[0]);
        if (needSpace[cat1][cat2])
            m_str += QLatin1Char(' ');
    }
    m_str += lexeme;
}

QT_END_NAMESPACE
