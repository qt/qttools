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

#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H

#include "ui_previewwidget.h"

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

class PreviewWidget: public QWidget
{
    Q_OBJECT
public:
    explicit PreviewWidget(QWidget *parent);
    ~PreviewWidget() override;

private:
    Ui::PreviewWidget ui;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // PREVIEWWIDGET_H
