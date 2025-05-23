// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial

#include "taskmenu.h"

QT_BEGIN_NAMESPACE

QDesignerTaskMenuExtension::~QDesignerTaskMenuExtension() = default;

QAction *QDesignerTaskMenuExtension::preferredEditAction() const
{ return nullptr; }

QT_END_NAMESPACE
