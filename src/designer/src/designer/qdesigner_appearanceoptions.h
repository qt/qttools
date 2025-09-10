// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QDESIGNER_APPEARANCEOPTIONS_H
#define QDESIGNER_APPEARANCEOPTIONS_H

#include "designer_enums.h"
#include "qdesigner_toolwindow.h"

#include <QtDesigner/abstractoptionspage.h>

#include <QtCore/qcompare.h>
#include <QtCore/qobject.h>
#include <QtCore/qpointer.h>
#include <QtWidgets/qwidget.h>

QT_BEGIN_NAMESPACE

class QDesignerFormEditorInterface;
class QDesignerSettings;

namespace Ui {
    class AppearanceOptionsWidget;
}

/* AppearanceOptions data */
struct AppearanceOptions
{
    void toSettings(QDesignerSettings &) const;
    void fromSettings(const QDesignerSettings &);

    UIMode uiMode{DockedMode};
    ToolWindowFontSettings toolWindowFontSettings;

    friend bool comparesEqual(const AppearanceOptions &lhs,
                              const AppearanceOptions &rhs) noexcept
    {
        return lhs.uiMode == rhs.uiMode
            && lhs.toolWindowFontSettings == rhs.toolWindowFontSettings;
    }
    Q_DECLARE_EQUALITY_COMPARABLE(AppearanceOptions)
};

/* QDesignerAppearanceOptionsWidget: Let the user edit AppearanceOptions */
class QDesignerAppearanceOptionsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QDesignerAppearanceOptionsWidget(QWidget *parent = nullptr);
    ~QDesignerAppearanceOptionsWidget();

    AppearanceOptions appearanceOptions() const;
    void setAppearanceOptions(const AppearanceOptions &ao);

signals:
    void uiModeChanged(bool modified);

private slots:
    void slotUiModeComboChanged();

private:
    UIMode uiMode() const;

    Ui::AppearanceOptionsWidget *m_ui;
    UIMode m_initialUIMode = NeutralMode;
};

/* The options page for appearance options. */

class QDesignerAppearanceOptionsPage : public QObject, public QDesignerOptionsPageInterface
{
    Q_OBJECT

public:
    QDesignerAppearanceOptionsPage(QDesignerFormEditorInterface *core);

    QString name() const override;
    QWidget *createPage(QWidget *parent) override;
    void apply() override;
    void finish() override;

signals:
    void settingsChanged();

private:
    QDesignerFormEditorInterface *m_core;
    QPointer<QDesignerAppearanceOptionsWidget> m_widget;
    AppearanceOptions m_initialOptions;
};

QT_END_NAMESPACE

#endif // QDESIGNER_APPEARANCEOPTIONS_H
