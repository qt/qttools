// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef SYSTEMSETTINGSDIALOG_H
#define SYSTEMSETTINGSDIALOG_H

#include <QtWidgets/qdialog.h>
#include <QtCore/qstringlist.h>

QT_BEGIN_NAMESPACE

namespace Ui {
    class DeviceProfileDialog;
}

class QDesignerDialogGuiInterface;

class QDialogButtonBox;

namespace qdesigner_internal {

class DeviceProfile;

/* DeviceProfileDialog: Widget to edit system settings for embedded design */

class DeviceProfileDialog : public QDialog
{
    Q_DISABLE_COPY_MOVE(DeviceProfileDialog)
    Q_OBJECT
public:
    explicit DeviceProfileDialog(QDesignerDialogGuiInterface *dlgGui, QWidget *parent = nullptr);
    ~DeviceProfileDialog();

    DeviceProfile deviceProfile() const;
    void setDeviceProfile(const DeviceProfile &s);

    bool showDialog(const QStringList &existingNames);

private slots:
    void setOkButtonEnabled(bool);
    void nameChanged(const QString &name);
    void save();
    void open() override;

private:
    void critical(const QString &title, const QString &msg);
    Ui::DeviceProfileDialog *m_ui;
    QDesignerDialogGuiInterface *m_dlgGui;
    QStringList m_existingNames;
};
}

QT_END_NAMESPACE

#endif // SYSTEMSETTINGSDIALOG_H
