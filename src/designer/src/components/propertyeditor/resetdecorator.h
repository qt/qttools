// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef RESETDECORATOR_H
#define RESETDECORATOR_H

#include <QtWidgets/qwidget.h>
#include <QtCore/qhash.h>

QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QToolButton)
QT_FORWARD_DECLARE_CLASS(QDesignerFormEditorInterface)

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

class QtProperty;
class QtAbstractPropertyManager;

namespace qdesigner_internal
{

class ResetWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ResetWidget(QtProperty *property, QWidget *parent = nullptr);

    void setWidget(QWidget *widget);
    void setResetEnabled(bool enabled);
    void setValueText(const QString &text);
    void setValueIcon(const QIcon &icon);
    void setSpacing(int spacing);

signals:
    void resetProperty(QtProperty *property);

private slots:
    void slotClicked();

private:
    QtProperty *m_property;
    QLabel *m_textLabel;
    QLabel *m_iconLabel;
    QToolButton *m_button;
    int m_spacing = -1;
};

class ResetDecorator : public QObject
{
    Q_OBJECT
public:
    explicit ResetDecorator(const QDesignerFormEditorInterface *core, QObject *parent = nullptr);
    ~ResetDecorator();

    void connectPropertyManager(QtAbstractPropertyManager *manager);
    QWidget *editor(QWidget *subEditor, bool resettable, QtAbstractPropertyManager *manager, QtProperty *property,
                QWidget *parent);
    void disconnectPropertyManager(QtAbstractPropertyManager *manager);
    void setSpacing(int spacing);

signals:
    void resetProperty(QtProperty *property);

private slots:
    void slotPropertyChanged(QtProperty *property);
    void slotEditorDestroyed(QObject *object);

private:
    QHash<const QtProperty *, QList<ResetWidget *>> m_createdResetWidgets;
    QHash<ResetWidget *, QtProperty *> m_resetWidgetToProperty;
    int m_spacing = -1;
    const QDesignerFormEditorInterface *m_core;
};
} // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // RESETDECORATOR_H
