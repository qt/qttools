// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [0]
    auto *formWindow = QDesignerFormWindowInterface::findFormWindow(myWidget);
//! [0]


//! [1]
    QList<QDesignerFormWindowInterface *> forms;

    auto *manager = formEditor->formWindowManager();

    for (int i = 0; i < manager->formWindowCount(); ++i)
        forms.append(manager->formWindow(i));
//! [1]


//! [2]
        if (formWindow->isManaged(myWidget))
            formWindow->manageWidget(myWidget->childWidget);
//! [2]


