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

#ifndef FORMWINDOWMANAGER_H
#define FORMWINDOWMANAGER_H

#include "formeditor_global.h"

#include <QtDesigner/private/qdesigner_formwindowmanager_p.h>

#include <QtCore/qobject.h>
#include <QtCore/qlist.h>
#include <QtCore/qpointer.h>
#include <QtCore/qmap.h>
#include <QtCore/qset.h>

QT_BEGIN_NAMESPACE

class QAction;
class QActionGroup;
class QUndoGroup;
class QDesignerFormEditorInterface;
class QDesignerWidgetBoxInterface;

namespace qdesigner_internal {

class FormWindow;
class PreviewManager;
class PreviewActionGroup;

class QT_FORMEDITOR_EXPORT FormWindowManager
    : public QDesignerFormWindowManager
{
    Q_OBJECT
public:
    explicit FormWindowManager(QDesignerFormEditorInterface *core, QObject *parent = nullptr);
    ~FormWindowManager() override;

    QDesignerFormEditorInterface *core() const override;

    QAction *action(Action action) const override;
    QActionGroup *actionGroup(ActionGroup actionGroup) const override;

    QDesignerFormWindowInterface *activeFormWindow() const override;

    int formWindowCount() const override;
    QDesignerFormWindowInterface *formWindow(int index) const override;

    QDesignerFormWindowInterface *createFormWindow(QWidget *parentWidget = nullptr, Qt::WindowFlags flags = {}) override;

    QPixmap createPreviewPixmap() const override;

    bool eventFilter(QObject *o, QEvent *e) override;

    void dragItems(const QList<QDesignerDnDItemInterface*> &item_list) override;

    QUndoGroup *undoGroup() const;

    PreviewManager *previewManager() const override { return m_previewManager; }

public slots:
    void addFormWindow(QDesignerFormWindowInterface *formWindow) override;
    void removeFormWindow(QDesignerFormWindowInterface *formWindow) override;
    void setActiveFormWindow(QDesignerFormWindowInterface *formWindow) override;
    void closeAllPreviews() override;
    void deviceProfilesChanged();

private slots:
#if QT_CONFIG(clipboard)
    void slotActionCutActivated();
    void slotActionCopyActivated();
    void slotActionPasteActivated();
#endif
    void slotActionDeleteActivated();
    void slotActionSelectAllActivated();
    void slotActionLowerActivated();
    void slotActionRaiseActivated();
    void createLayout();
    void slotActionBreakLayoutActivated();
    void slotActionAdjustSizeActivated();
    void slotActionSimplifyLayoutActivated();
    void showPreview() override;
    void slotActionGroupPreviewInStyle(const QString &style, int deviceProfileIndex);
    void slotActionShowFormWindowSettingsDialog();

    void slotUpdateActions();

private:
    void setupActions();
    FormWindow *findFormWindow(QWidget *w);
    QWidget *findManagedWidget(FormWindow *fw, QWidget *w);

    void setCurrentUndoStack(QUndoStack *stack);

private:
    enum CreateLayoutContext { LayoutContainer, LayoutSelection, MorphLayout };

    QDesignerFormEditorInterface *m_core;
    FormWindow *m_activeFormWindow;
    QList<FormWindow*> m_formWindows;

    PreviewManager *m_previewManager;

    /* Context of the layout actions and base for morphing layouts. Determined
     * in slotUpdateActions() and used later on in the action slots. */
    CreateLayoutContext m_createLayoutContext;
    QWidget *m_morphLayoutContainer;

    // edit actions
#if QT_CONFIG(clipboard)
    QAction *m_actionCut = nullptr;
    QAction *m_actionCopy = nullptr;
    QAction *m_actionPaste = nullptr;
#endif
    QAction *m_actionSelectAll = nullptr;
    QAction *m_actionDelete = nullptr;
    QAction *m_actionLower = nullptr;
    QAction *m_actionRaise = nullptr;
    // layout actions
    QAction *m_actionHorizontalLayout = nullptr;
    QAction *m_actionVerticalLayout = nullptr;
    QAction *m_actionFormLayout = nullptr;
    QAction *m_actionSplitHorizontal = nullptr;
    QAction *m_actionSplitVertical = nullptr;
    QAction *m_actionGridLayout = nullptr;
    QAction *m_actionBreakLayout = nullptr;
    QAction *m_actionSimplifyLayout = nullptr;
    QAction *m_actionAdjustSize = nullptr;
    // preview actions
    QAction *m_actionDefaultPreview = nullptr;
    mutable PreviewActionGroup *m_actionGroupPreviewInStyle = nullptr;
    QAction *m_actionShowFormWindowSettingsDialog = nullptr;

    QAction *m_actionUndo = nullptr;
    QAction *m_actionRedo = nullptr;

    QSet<QWidget *> getUnsortedLayoutsToBeBroken(bool firstOnly) const;
    bool hasLayoutsToBeBroken() const;
    QWidgetList layoutsToBeBroken(QWidget *w) const;
    QWidgetList layoutsToBeBroken() const;

    QUndoGroup *m_undoGroup = nullptr;

};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // FORMWINDOWMANAGER_H
