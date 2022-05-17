// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "invisible_widget_p.h"

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

InvisibleWidget::InvisibleWidget(QWidget *parent)
    : QWidget()
{
    setAttribute(Qt::WA_NoChildEventsForParent);
    setParent(parent);
}

} // namespace qdesigner_internal

QT_END_NAMESPACE
