// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [0]
        QDesignerPropertyEditorInterface *propertyEditor = nullptr;
        propertyEditor = formEditor->propertyEditor();

        connect(propertyEditor, SIGNAL(propertyChanged(QString,QVariant)),
                this, SLOT(checkProperty(QString,QVariant)));
//! [0]


//! [1]
        void checkProperty(QString property, QVariant value) {
            QDesignerPropertyEditorInterface *propertyEditor = nullptr;
            propertyEditor = formEditor->propertyEditor();

            QObject *object = propertyeditor->object();
            MyCustomWidget *widget = qobject_cast<MyCustomWidget>(object);

            if (widget && property == aProperty && value != expectedValue)
                {...}
        }
//! [1]


