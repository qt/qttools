// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef SAVEFORMASTEMPLATE_H
#define SAVEFORMASTEMPLATE_H

#include "ui_saveformastemplate.h"

QT_BEGIN_NAMESPACE

class QDesignerFormEditorInterface;
class QDesignerFormWindowInterface;

class SaveFormAsTemplate: public QDialog
{
    Q_OBJECT
public:
    explicit SaveFormAsTemplate(QDesignerFormEditorInterface *m_core,
                                QDesignerFormWindowInterface *formWindow,
                                QWidget *parent = nullptr);
    ~SaveFormAsTemplate() override;

private slots:
    void accept() override;
    void updateOKButton(const QString &str);
    void checkToAddPath(int itemIndex);

private:
    static QString chooseTemplatePath(QWidget *parent);

    Ui::SaveFormAsTemplate ui;
    QDesignerFormEditorInterface *m_core;
    QDesignerFormWindowInterface *m_formWindow;
    int m_addPathIndex;
};

QT_END_NAMESPACE

#endif // SAVEFORMASTEMPLATE_H
