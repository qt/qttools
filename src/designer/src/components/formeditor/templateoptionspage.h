/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
******************************************************************************/

#ifndef QDESIGNER_TEMPLATEOPTIONS_H
#define QDESIGNER_TEMPLATEOPTIONS_H

#include <QtDesigner/abstractoptionspage.h>

#include <QtCore/qpointer.h>
#include <QtCore/qstringlist.h>

#include <QtWidgets/qwidget.h>

QT_BEGIN_NAMESPACE

class QDesignerFormEditorInterface;

namespace qdesigner_internal {

namespace Ui {
    class TemplateOptionsWidget;
}

/* Present the user with a list of form template paths to save
 * form templates. */
class TemplateOptionsWidget : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(TemplateOptionsWidget)
public:
    explicit TemplateOptionsWidget(QDesignerFormEditorInterface *core,
                                              QWidget *parent = nullptr);
    ~TemplateOptionsWidget();


    QStringList templatePaths() const;
    void setTemplatePaths(const QStringList &l);

    static QString chooseTemplatePath(QDesignerFormEditorInterface *core, QWidget *parent);

private slots:
    void addTemplatePath();
    void removeTemplatePath();
    void templatePathSelectionChanged();

private:
    QDesignerFormEditorInterface *m_core;
    Ui::TemplateOptionsWidget *m_ui;
};

class TemplateOptionsPage : public QDesignerOptionsPageInterface
{
     Q_DISABLE_COPY_MOVE(TemplateOptionsPage)
public:
    explicit TemplateOptionsPage(QDesignerFormEditorInterface *core);

    QString name() const override;
    QWidget *createPage(QWidget *parent) override;
    void apply() override;
    void finish() override;

private:
    QDesignerFormEditorInterface *m_core;
    QStringList m_initialTemplatePaths;
    QPointer<TemplateOptionsWidget> m_widget;
};

}

QT_END_NAMESPACE

#endif // QDESIGNER_TEMPLATEOPTIONS_H
