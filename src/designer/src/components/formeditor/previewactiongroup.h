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

#ifndef PREVIEWACTIONGROUP_H
#define PREVIEWACTIONGROUP_H

#include <QtGui/qactiongroup.h>

QT_BEGIN_NAMESPACE

class QDesignerFormEditorInterface;

namespace qdesigner_internal {

/* PreviewActionGroup: To be used as a submenu for 'Preview in...'
 * Offers a menu of styles and device profiles. */

class PreviewActionGroup : public QActionGroup
{
    Q_DISABLE_COPY_MOVE(PreviewActionGroup)
    Q_OBJECT
public:
    explicit PreviewActionGroup(QDesignerFormEditorInterface *core, QObject *parent = nullptr);

signals:
    void preview(const QString &style, int deviceProfileIndex);

public slots:
    void updateDeviceProfiles();

private  slots:
    void slotTriggered(QAction *);

private:
    QDesignerFormEditorInterface *m_core;
};
}

QT_END_NAMESPACE

#endif // PREVIEWACTIONGROUP_H
