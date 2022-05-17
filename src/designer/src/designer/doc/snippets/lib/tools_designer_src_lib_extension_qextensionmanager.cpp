// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [0]
        void MyPlugin::initialize(QDesignerFormEditorInterface *formEditor)
        {
            if (initialized)
                return;

            QExtensionManager *manager = formEditor->extensionManager();
            Q_ASSERT(manager != 0);

            manager->registerExtensions(new MyExtensionFactory(manager),
                                        Q_TYPEID(QDesignerTaskMenuExtension));

            initialized = true;
        }
//! [0]


