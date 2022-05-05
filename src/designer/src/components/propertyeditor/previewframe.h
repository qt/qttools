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

#ifndef PREVIEWFRAME_H
#define PREVIEWFRAME_H

#include <QtWidgets/qframe.h>
#include <QtCore/qpointer.h>

QT_BEGIN_NAMESPACE

class QMdiArea;
class QMdiSubWindow;

namespace qdesigner_internal {

class PreviewFrame: public QFrame
{
    Q_OBJECT
public:
    explicit PreviewFrame(QWidget *parent);

    void setPreviewPalette(const QPalette &palette);
    void setSubWindowActive(bool active);

private:
    // The user can on some platforms close the mdi child by invoking the system menu.
    // Ensure a child is present.
    QMdiSubWindow *ensureMdiSubWindow();
    QMdiArea *m_mdiArea;
    QPointer<QMdiSubWindow> m_mdiSubWindow;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif
