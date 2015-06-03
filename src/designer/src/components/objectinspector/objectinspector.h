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

#ifndef OBJECTINSPECTOR_H
#define OBJECTINSPECTOR_H

#include "objectinspector_global.h"
#include "qdesigner_objectinspector_p.h"

QT_BEGIN_NAMESPACE

class QDesignerFormEditorInterface;
class QDesignerFormWindowInterface;

class QItemSelection;

namespace qdesigner_internal {

class QT_OBJECTINSPECTOR_EXPORT ObjectInspector: public QDesignerObjectInspector
{
    Q_OBJECT
public:
    explicit ObjectInspector(QDesignerFormEditorInterface *core, QWidget *parent = 0);
    virtual ~ObjectInspector();

    QDesignerFormEditorInterface *core() const Q_DECL_OVERRIDE;

    void getSelection(Selection &s) const Q_DECL_OVERRIDE;
    bool selectObject(QObject *o) Q_DECL_OVERRIDE;
    virtual void clearSelection();

    void setFormWindow(QDesignerFormWindowInterface *formWindow);

public slots:
    virtual void mainContainerChanged();

private slots:
    void slotSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void slotPopupContextMenu(const QPoint &pos);
    void slotHeaderDoubleClicked(int column);

protected:
    void dragEnterEvent (QDragEnterEvent * event) Q_DECL_OVERRIDE;
    void dragMoveEvent(QDragMoveEvent * event) Q_DECL_OVERRIDE;
    void dragLeaveEvent(QDragLeaveEvent * event) Q_DECL_OVERRIDE;
    void dropEvent (QDropEvent * event) Q_DECL_OVERRIDE;

private:
    class ObjectInspectorPrivate;
    ObjectInspectorPrivate *m_impl;
};

}  // namespace qdesigner_internal

#endif // OBJECTINSPECTOR_H

QT_END_NAMESPACE
