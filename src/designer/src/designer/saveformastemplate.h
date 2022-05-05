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
