// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef FORMWINDOWSETTINGS_H
#define FORMWINDOWSETTINGS_H

#include <QtWidgets/qdialog.h>

QT_BEGIN_NAMESPACE

namespace Ui {
    class FormWindowSettings;
}

class QDesignerFormWindowInterface;

namespace qdesigner_internal {

struct FormWindowData;
class FormWindowBase;

/* Dialog to edit the settings of a QDesignerFormWindowInterface.
 * It sets the dirty flag on the form window if something was changed. */

class FormWindowSettings: public QDialog
{
    Q_DISABLE_COPY_MOVE(FormWindowSettings)
    Q_OBJECT
public:
    explicit FormWindowSettings(QDesignerFormWindowInterface *formWindow);
    ~FormWindowSettings() override;

    void accept() override;

private:
    FormWindowData data() const;
    void setData(const FormWindowData&);

    QT_PREPEND_NAMESPACE(Ui)::FormWindowSettings *m_ui;
    FormWindowBase *m_formWindow;
    FormWindowData *m_oldData;
};
}

QT_END_NAMESPACE

#endif // FORMWINDOWSETTINGS_H
