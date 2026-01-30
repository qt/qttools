// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef RESETDECORATOR_H
#define RESETDECORATOR_H

#include <QtWidgets/qwidget.h>

QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QToolButton)
QT_FORWARD_DECLARE_CLASS(QDesignerFormEditorInterface)

QT_BEGIN_NAMESPACE

class QtProperty;
class QtAbstractPropertyManager;

namespace qdesigner_internal
{

class ResetWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ResetWidget(QWidget *editor, QWidget *parent = nullptr);

    void setSpacing(int spacing);

public Q_SLOTS:
    void setResetEnabled(bool enabled);

Q_SIGNALS:
    void reset();

private:
    QToolButton *m_button;
};

class PropertyResetWidget : public ResetWidget
{
    Q_OBJECT
public:
    explicit PropertyResetWidget(const QDesignerFormEditorInterface *core, QtProperty *property,
                                 QWidget *editor, QWidget *parent = nullptr);

public Q_SLOTS:
    void propertyChanged(QtProperty *property);

Q_SIGNALS:
    void resetProperty(QtProperty *property);

private Q_SLOTS:
    void emitResetProperty();

private:
    const QDesignerFormEditorInterface *m_core;
    QtProperty *m_property;
};

// A dummy editor used for parent properties of sub properties
// that can be reset (for example, QSize, which does not have an editor).
// It merely displays icon and value text of the property.
class DummyEditor : public QWidget
{
    Q_OBJECT
public:
    explicit DummyEditor(QtProperty *property, QWidget *parent = nullptr);

    void setSpacing(int spacing);

public Q_SLOTS:
    void propertyChanged(QtProperty *property);

private:
    void setValueIcon(const QIcon &icon);

    QtProperty *m_property;
    QLabel *m_textLabel;
    QLabel *m_iconLabel;
};
} // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // RESETDECORATOR_H
