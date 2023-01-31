// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [0]
        auto *propertyEditor = formEditor->propertyEditor();

        connect(propertyEditor, &QDesignerPropertyEditorInterface::propertyChanged,
                this, &MyClass::checkProperty);
//! [0]


//! [1]
        void checkProperty(const QString &property, const QVariant &value)
        {
            auto *propertyEditor = formEditor->propertyEditor();

            auto *object = propertyeditor->object();
            auto *widget = qobject_cast<MyCustomWidget *>(object);

            if (widget && property == aProperty && value != expectedValue)
                {...}
        }
//! [1]


