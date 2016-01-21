/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Designer of the Qt Toolkit.
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

#ifndef QDESIGNERUNDOSTACK_H
#define QDESIGNERUNDOSTACK_H

#include <QtCore/QObject>

QT_BEGIN_NAMESPACE
class QUndoStack;
class QUndoCommand;

namespace qdesigner_internal {

/* QDesignerUndoStack: A QUndoStack extended by a way of setting it to
 * "dirty" indepently of commands (by modifications without commands
 * such as resizing). Accomplished via bool m_fakeDirty flag. The
 * lifecycle of the QUndoStack is managed by the QUndoGroup. */
class QDesignerUndoStack : public QObject
{
    Q_DISABLE_COPY(QDesignerUndoStack)
    Q_OBJECT
public:
    explicit QDesignerUndoStack(QObject *parent = 0);
    virtual ~QDesignerUndoStack();

    void clear();
    void push(QUndoCommand * cmd);
    void beginMacro(const QString &text);
    void endMacro();
    int  index() const;

    const QUndoStack *qundoStack() const;
    QUndoStack *qundoStack();

    bool isDirty() const;

signals:
    void changed();

public slots:
    void setDirty(bool);

private:
    QUndoStack *m_undoStack;
    bool m_fakeDirty;
};

} // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // QDESIGNERUNDOSTACK_H
