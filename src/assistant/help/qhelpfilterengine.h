/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Assistant of the Qt Toolkit.
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
**
**
**
**
**
**
**
**
**
**
**
******************************************************************************/

#ifndef QHELPFILTERENGINE_H
#define QHELPFILTERENGINE_H

#include <QtHelp/qhelp_global.h>

#include <QtCore/QObject>

QT_BEGIN_NAMESPACE

template <class K, class T>
class QMap;
class QVersionNumber;

class QHelpCollectionHandler;
class QHelpEngineCore;
class QHelpFilterData;
class QHelpFilterEnginePrivate;

class QHELP_EXPORT QHelpFilterEngine : public QObject
{
    Q_OBJECT
public:
    QMap<QString, QString> namespaceToComponent() const;
    QMap<QString, QVersionNumber> namespaceToVersion() const;

    QStringList filters() const;

    QString activeFilter() const;
    bool setActiveFilter(const QString &filterName);

    QStringList availableComponents() const;
    QList<QVersionNumber> availableVersions() const;

    QHelpFilterData filterData(const QString &filterName) const;
    bool setFilterData(const QString &filterName, const QHelpFilterData &filterData);

    bool removeFilter(const QString &filterName);

    QStringList namespacesForFilter(const QString &filterName) const;

    QStringList indices() const;
    QStringList indices(const QString &filterName) const;

Q_SIGNALS:
    void filterActivated(const QString &newFilter);

protected:
    explicit QHelpFilterEngine(QHelpEngineCore *helpEngine);
    virtual ~QHelpFilterEngine();

private:
    void setCollectionHandler(QHelpCollectionHandler *collectionHandler);

    QHelpFilterEnginePrivate *d;
    friend class QHelpEngineCore;
    friend class QHelpEngineCorePrivate;
};

QT_END_NAMESPACE

#endif // QHELPFILTERENGINE_H
