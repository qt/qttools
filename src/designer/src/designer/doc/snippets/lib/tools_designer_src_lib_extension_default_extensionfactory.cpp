// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [0]
        QObject *ANewExtensionFactory::createExtension(QObject *object,
                const QString &iid, QObject *parent) const
        {
            if (iid != Q_TYPEID(QDesignerContainerExtension))
                return nullptr;

            if (auto *widget = qobject_cast<MyCustomWidget*>(object))
                return new MyContainerExtension(widget, parent);

            return nullptr;
        }
//! [0]


//! [1]
        QObject *AGeneralExtensionFactory::createExtension(QObject *object,
                const QString &iid, QObject *parent) const
        {
            auto *widget = qobject_cast<MyCustomWidget*>(object);
            if (!widget)
                return nullptr;

            if (iid == Q_TYPEID(QDesignerTaskMenuExtension))
                return new MyTaskMenuExtension(widget, parent);

            if (iid == Q_TYPEID(QDesignerContainerExtension))
                return new MyContainerExtension(widget, parent);

            return nullptr;
        }
//! [1]


