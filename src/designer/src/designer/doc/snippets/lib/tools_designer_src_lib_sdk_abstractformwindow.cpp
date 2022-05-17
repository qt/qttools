// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [0]
    QDesignerFormWindowInterface *formWindow;
    formWindow = QDesignerFormWindowInterface::findFormWindow(myWidget);
//! [0]


//! [1]
    QList<QDesignerFormWindowInterface *> forms;
    QDesignerFormWindowInterface *formWindow;

    QDesignerFormWindowManagerInterface *manager = formEditor->formWindowManager();

    for (int i = 0; i < manager->formWindowCount(); i++) {
        formWindow = manager->formWindow(i);
        forms.append(formWindow);
    }
//! [1]


//! [2]
        if (formWindow->isManaged(myWidget))
            formWindow->manageWidget(myWidget->childWidget);
//! [2]


