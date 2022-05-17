// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef BUDDYEDITOR_H
#define BUDDYEDITOR_H

#include "buddyeditor_global.h"

#include <connectionedit_p.h>
#include <QtCore/qpointer.h>
#include <QtCore/qset.h>

QT_BEGIN_NAMESPACE

class QDesignerFormWindowInterface;

class QLabel;

namespace qdesigner_internal {

class QT_BUDDYEDITOR_EXPORT BuddyEditor : public ConnectionEdit
{
    Q_OBJECT

public:
    BuddyEditor(QDesignerFormWindowInterface *form, QWidget *parent);

    QDesignerFormWindowInterface *formWindow() const;
    void setBackground(QWidget *background) override;
    void deleteSelected() override;

public slots:
    void updateBackground() override;
    void widgetRemoved(QWidget *w) override;
    void autoBuddy();

protected:
    QWidget *widgetAt(const QPoint &pos) const override;
    Connection *createConnection(QWidget *source, QWidget *destination) override;
    void endConnection(QWidget *target, const QPoint &pos) override;
    void createContextMenu(QMenu &menu) override;

private:
    QWidget *findBuddy(QLabel *l, const QWidgetList &existingBuddies) const;

    QPointer<QDesignerFormWindowInterface> m_formWindow;
    bool m_updating;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif
