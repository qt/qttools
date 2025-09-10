// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef FORMWINDOWCURSOR_H
#define FORMWINDOWCURSOR_H

#include "formeditor_global.h"
#include "formwindow.h"
#include <QtDesigner/abstractformwindowcursor.h>

#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

class QT_FORMEDITOR_EXPORT FormWindowCursor: public QObject, public QDesignerFormWindowCursorInterface
{
    Q_OBJECT
public:
    explicit FormWindowCursor(FormWindow *fw, QObject *parent = nullptr);
    ~FormWindowCursor() override;

    QDesignerFormWindowInterface *formWindow() const override;

    bool movePosition(MoveOperation op, MoveMode mode) override;

    int position() const override;
    void setPosition(int pos, MoveMode mode) override;

    QWidget *current() const override;

    int widgetCount() const override;
    QWidget *widget(int index) const override;

    bool hasSelection() const override;
    int selectedWidgetCount() const override;
    QWidget *selectedWidget(int index) const override;

    void setProperty(const QString &name, const QVariant &value) override;
    void setWidgetProperty(QWidget *widget, const QString &name, const QVariant &value) override;
    void resetWidgetProperty(QWidget *widget, const QString &name) override;

public slots:
    void update();

private:
    FormWindow *m_formWindow;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // FORMWINDOWCURSOR_H
