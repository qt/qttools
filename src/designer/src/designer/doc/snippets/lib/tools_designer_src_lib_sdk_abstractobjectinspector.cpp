// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [0]
        QDesignerObjectInspectorInterface *objectInspector = nullptr;
        objectInspector = formEditor->objectInspector();

        QDesignerFormWindowManagerInterface *manager = nullptr;
        manager = formEditor->formWindowManager();

        objectInspector->setFormWindow(manager->formWindow(0));
//! [0]


