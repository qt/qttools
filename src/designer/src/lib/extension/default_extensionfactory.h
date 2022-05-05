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

#ifndef DEFAULT_EXTENSIONFACTORY_H
#define DEFAULT_EXTENSIONFACTORY_H

#include <QtDesigner/extension_global.h>
#include <QtDesigner/extension.h>

#include <QtCore/qmap.h>
#include <QtCore/qhash.h>
#include <QtCore/qpair.h>

QT_BEGIN_NAMESPACE

class QExtensionManager;

class QDESIGNER_EXTENSION_EXPORT QExtensionFactory : public QObject, public QAbstractExtensionFactory
{
    Q_OBJECT
    Q_INTERFACES(QAbstractExtensionFactory)
public:
    explicit QExtensionFactory(QExtensionManager *parent = nullptr);

    QObject *extension(QObject *object, const QString &iid) const override;
    QExtensionManager *extensionManager() const;

private Q_SLOTS:
    void objectDestroyed(QObject *object);

protected:
    virtual QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;

private:
    typedef QPair<QString,QObject*> IdObjectKey;
    typedef QMap< IdObjectKey, QObject*> ExtensionMap;
    mutable ExtensionMap m_extensions;
    typedef QHash<QObject*, bool> ExtendedSet;
    mutable ExtendedSet m_extended;
};

QT_END_NAMESPACE

#endif // DEFAULT_EXTENSIONFACTORY_H
