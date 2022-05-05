/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
******************************************************************************/

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef CSSHIGHLIGHTER_H
#define CSSHIGHLIGHTER_H

#include <QtGui/qsyntaxhighlighter.h>
#include <QtGui/qcolor.h>
#include "shared_global_p.h"

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

struct CssHighlightColors
{
    QColor selector;
    QColor property;
    QColor value;
    QColor pseudo1;
    QColor pseudo2;
    QColor quote;
    QColor comment;
};

class QDESIGNER_SHARED_EXPORT CssHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    explicit CssHighlighter(const CssHighlightColors &colors,
                            QTextDocument *document);

protected:
    void highlightBlock(const QString&) override;
    void highlight(const QString&, int, int, int/*State*/);

private:
    enum State { Selector, Property, Value, Pseudo, Pseudo1, Pseudo2, Quote,
                 MaybeComment, Comment, MaybeCommentEnd };

    const CssHighlightColors m_colors;
};

} // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // CSSHIGHLIGHTER_H
