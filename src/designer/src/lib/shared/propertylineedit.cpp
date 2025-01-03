// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "propertylineedit_p.h"

#include <QtGui/qevent.h>
#include <QtWidgets/qmenu.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

namespace qdesigner_internal {
    PropertyLineEdit::PropertyLineEdit(QWidget *parent) :
        QLineEdit(parent), m_wantNewLine(false)
    {
    }

    bool PropertyLineEdit::event(QEvent *e)
    {
        // handle 'Select all' here as it is not done in the QLineEdit
        if (e->type() == QEvent::ShortcutOverride && !isReadOnly()) {
            QKeyEvent* ke = static_cast<QKeyEvent*> (e);
            if (ke->modifiers() & Qt::ControlModifier) {
                if(ke->key() == Qt::Key_A) {
                    ke->accept();
                    return true;
                }
            }
        }
        return QLineEdit::event(e);
    }

    void PropertyLineEdit::insertNewLine() {
        insertText(u"\\n"_s);
    }

    void PropertyLineEdit::insertText(const QString &text) {
        // position cursor after new text and grab focus
        const int oldCursorPosition = cursorPosition ();
        insert(text);
        setCursorPosition (oldCursorPosition + text.size());
        setFocus(Qt::OtherFocusReason);
    }

    void PropertyLineEdit::contextMenuEvent(QContextMenuEvent *event) {
        QMenu  *menu = createStandardContextMenu ();

        if (m_wantNewLine) {
            menu->addSeparator();
            menu->addAction(tr("Insert line break"), this, &PropertyLineEdit::insertNewLine);
        }

        menu->exec(event->globalPos());
    }
}

QT_END_NAMESPACE
