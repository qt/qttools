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

#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QtWidgets/qdialog.h>

QT_BEGIN_NAMESPACE

class QPushButton;
class QDesignerFormEditorInterface;
class QDesignerOptionsPageInterface;

namespace Ui {
    class PreferencesDialog;
}

class PreferencesDialog: public QDialog
{
    Q_OBJECT
public:
    explicit PreferencesDialog(QDesignerFormEditorInterface *core, QWidget *parentWidget = nullptr);
    ~PreferencesDialog();


private slots:
    void slotAccepted();
    void slotRejected();
    void slotApply();
    void slotUiModeChanged(bool modified);

private:
    QPushButton *applyButton() const;
    void closeOptionPages();

    Ui::PreferencesDialog *m_ui;
    QDesignerFormEditorInterface *m_core;
    QList<QDesignerOptionsPageInterface*> m_optionsPages;
};

QT_END_NAMESPACE

#endif // PREFERENCESDIALOG_H
