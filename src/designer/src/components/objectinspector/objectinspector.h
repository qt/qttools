// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

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
    explicit ObjectInspector(QDesignerFormEditorInterface *core, QWidget *parent = nullptr);
    ~ObjectInspector() override;

    QDesignerFormEditorInterface *core() const override;

    void getSelection(Selection &s) const override;
    bool selectObject(QObject *o) override;
    void clearSelection() override;

    void setFormWindow(QDesignerFormWindowInterface *formWindow) override;

public slots:
    void mainContainerChanged() override;

private slots:
    void slotSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void slotPopupContextMenu(const QPoint &pos);
    void slotHeaderDoubleClicked(int column);

protected:
    void dragEnterEvent (QDragEnterEvent * event) override;
    void dragMoveEvent(QDragMoveEvent * event) override;
    void dragLeaveEvent(QDragLeaveEvent * event) override;
    void dropEvent (QDropEvent * event) override;

private:
    class ObjectInspectorPrivate;
    ObjectInspectorPrivate *m_impl;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // OBJECTINSPECTOR_H
