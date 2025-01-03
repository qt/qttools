// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "taskmenu.h"

QT_BEGIN_NAMESPACE

QDesignerTaskMenuExtension::~QDesignerTaskMenuExtension() = default;

QAction *QDesignerTaskMenuExtension::preferredEditAction() const
{ return nullptr; }

QT_END_NAMESPACE
