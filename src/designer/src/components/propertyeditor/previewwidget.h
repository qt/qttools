// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

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
