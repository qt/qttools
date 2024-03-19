// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef VIEW3D_H
#define VIEW3D_H

#include "view3d_global.h"
#include <QtWidgets/qwidget.h>

QT_BEGIN_NAMESPACE

class QScrollBar;
class QGLWidget;
class QDesignerFormWindowInterface;

class QView3DWidget;

class QView3D : public QWidget
{
    Q_OBJECT

public:
    QView3D(QDesignerFormWindowInterface *form_window, QWidget *parent);

public slots:
    void updateForm();

private:
    QView3DWidget *m_3d_widget;
    QDesignerFormWindowInterface *m_form_window;

    void addWidget(int depth, QWidget *w);
    void addTexture(QWidget *w);
};

#endif // VIEW3D_H


QT_END_NAMESPACE
