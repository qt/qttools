/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef VIEW3D_TOOL_H
#define VIEW3D_TOOL_H

#include "view3d_global.h"
#include "view3d.h"

#include <QtDesigner/QDesignerFormWindowToolInterface>
#include <QtCore/QPointer>

QT_BEGIN_NAMESPACE

class VIEW3D_EXPORT QView3DTool : public QDesignerFormWindowToolInterface
{
    Q_OBJECT

public:
    explicit QView3DTool(QDesignerFormWindowInterface *formWindow, QObject *parent = 0);
    QDesignerFormEditorInterface *core() const Q_DECL_OVERRIDE;
    QDesignerFormWindowInterface *formWindow() const Q_DECL_OVERRIDE;
    QWidget *editor() const Q_DECL_OVERRIDE;

    QAction *action() const Q_DECL_OVERRIDE;

    virtual void activated();
    virtual void deactivated();

    bool handleEvent(QWidget *widget, QWidget *managedWidget, QEvent *event) Q_DECL_OVERRIDE;

private:
    QDesignerFormWindowInterface *m_formWindow;
    mutable QPointer<QView3D> m_editor;
    QAction *m_action;
};

#endif // VIEW3D_TOOL_H

QT_END_NAMESPACE
