// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
// Qt-Security score:significant reason:default

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef MENUACTIONLINEEDIT_P_H
#define MENUACTIONLINEEDIT_P_H

#include <QtWidgets/qlineedit.h>

QT_BEGIN_NAMESPACE

class QLineEdit;

namespace qdesigner_internal {

class MenuActionLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    using QLineEdit::QLineEdit;

Q_SIGNALS:
    void focusOut();

protected:
    void focusOutEvent(QFocusEvent *event) override
    {
        Q_EMIT focusOut();
        QLineEdit::focusOutEvent(event);
    }
};

} // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // MENUACTIONLINEEDIT_P_H
