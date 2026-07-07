// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
// Qt-Security score:significant reason:default

//! [0]
    auto *formWindow = QDesignerFormWindowInterface::findFormWindow(myWidget);

    formWindow->cursor()->setProperty(myWidget, myProperty, newValue);
//! [0]


