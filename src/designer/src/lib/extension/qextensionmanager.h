/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
******************************************************************************/

#ifndef QEXTENSIONMANAGER_H
#define QEXTENSIONMANAGER_H

#include <QtDesigner/extension_global.h>
#include <QtDesigner/extension.h>
#include <QtCore/qhash.h>

QT_BEGIN_NAMESPACE

class QObject; // Fool syncqt

class QDESIGNER_EXTENSION_EXPORT QExtensionManager: public QObject, public QAbstractExtensionManager
{
    Q_OBJECT
    Q_INTERFACES(QAbstractExtensionManager)
public:
    explicit QExtensionManager(QObject *parent = nullptr);
    ~QExtensionManager();

    void registerExtensions(QAbstractExtensionFactory *factory, const QString &iid = QString()) override;
    void unregisterExtensions(QAbstractExtensionFactory *factory, const QString &iid = QString()) override;

    QObject *extension(QObject *object, const QString &iid) const override;

private:
    using FactoryList = QList<QAbstractExtensionFactory *>;
    typedef QHash<QString, FactoryList> FactoryMap;
    FactoryMap m_extensions;
    FactoryList m_globalExtension;
};

QT_END_NAMESPACE

#endif // QEXTENSIONMANAGER_H
