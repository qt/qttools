// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [0]
        auto *manager = formEditor->formWindowManager();
        auto *formWindow = manager->formWindow(0);

        manager->setActiveFormWindow(formWindow);
//! [0]


