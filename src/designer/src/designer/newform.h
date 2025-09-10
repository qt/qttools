// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef NEWFORM_H
#define NEWFORM_H

#include <QtWidgets/qdialog.h>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {
    class DeviceProfile;
}

class QDesignerFormEditorInterface;
class QDesignerNewFormWidgetInterface;
class QDesignerWorkbench;

class QCheckBox;
class QAbstractButton;
class QPushButton;
class QDialogButtonBox;
class QImage;
class QIODevice;

class NewForm: public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(NewForm)

public:
    NewForm(QDesignerWorkbench *workbench,
            QWidget *parentWidget,
            // Use that file name instead of a temporary one
            const QString &fileName = QString());

    ~NewForm() override;

    // Convenience for implementing file dialogs with preview
    static QImage grabForm(QDesignerFormEditorInterface *core,
                           QIODevice &file,
                           const QString &workingDir,
                           const qdesigner_internal::DeviceProfile &dp);

private slots:
    void slotButtonBoxClicked(QAbstractButton *btn);
    void recentFileChosen();
    void slotCurrentTemplateChanged(bool templateSelected);
    void slotTemplateActivated();

private:
    QDialogButtonBox *createButtonBox();
    bool openTemplate(QString *ptrToErrorMessage);

    QString m_fileName;
    QDesignerNewFormWidgetInterface *m_newFormWidget;
    QDesignerWorkbench *m_workbench;
    QCheckBox *m_chkShowOnStartup;
    QPushButton *m_createButton;
    QPushButton *m_recentButton;
    QDialogButtonBox *m_buttonBox = nullptr;
};

QT_END_NAMESPACE

#endif // NEWFORM_H
