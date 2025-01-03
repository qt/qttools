// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef FORMEDITOR_OPTIONSPAGE_H
#define FORMEDITOR_OPTIONSPAGE_H

#include <QtDesigner/abstractoptionspage.h>
#include <QtCore/qpointer.h>

QT_BEGIN_NAMESPACE

class QComboBox;
class QDesignerFormEditorInterface;

namespace qdesigner_internal {

class PreviewConfigurationWidget;
class GridPanel;
class ZoomSettingsWidget;

class FormEditorOptionsPage : public QDesignerOptionsPageInterface
{
public:
    explicit FormEditorOptionsPage(QDesignerFormEditorInterface *core);

    QString name() const override;
    QWidget *createPage(QWidget *parent) override;
    void apply() override;
    void finish() override;

private:
    QDesignerFormEditorInterface *m_core;
    QPointer<PreviewConfigurationWidget> m_previewConf;
    QPointer<GridPanel> m_defaultGridConf;
    QPointer<ZoomSettingsWidget> m_zoomSettingsWidget;
    QPointer<QComboBox> m_namingComboBox;
};

} // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // FORMEDITOR_OPTIONSPAGE_H
