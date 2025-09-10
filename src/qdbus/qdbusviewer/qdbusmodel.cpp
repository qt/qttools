// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "qdbusmodel.h"

#include <QtCore/QDebug>
#include <QtCore/QList>

#include <QtDBus/QDBusObjectPath>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>

#include <QtXml/QDomDocument>

using namespace Qt::StringLiterals;

struct QDBusItem
{
    inline QDBusItem(QDBusModel::Type aType, const QString &aName, QDBusItem *aParent = 0)
        : type(aType), parent(aParent), isPrefetched(type != QDBusModel::PathItem), name(aName)
        {}
    inline ~QDBusItem()
    {
        qDeleteAll(children);
    }

    QString path() const
    {
        Q_ASSERT(type == QDBusModel::PathItem);

        QString s;
        const QDBusItem *item = this;
        while (item) {
            s.prepend(item->name);
            item = item->parent;
        }
        if (s.size() > 1)
            s.chop(1); // remove tailing slash
        return s;
    }

    QDBusModel::Type type;
    QDBusItem *parent;
    QList<QDBusItem *> children;
    bool isPrefetched;
    QString name;
    QString caption;
    QString typeSignature;
};

QDomDocument QDBusModel::introspect(const QString &path)
{
    QDomDocument doc;

    QDBusInterface iface(service, path, "org.freedesktop.DBus.Introspectable"_L1, c);
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        emit busError(tr("Cannot introspect object %1 at %2:\n  %3 (%4)\n")
                              .arg(path)
                              .arg(service)
                              .arg(err.name())
                              .arg(err.message()));
        return doc;
    }

    QDBusReply<QString> xml = iface.call("Introspect"_L1);

    if (!xml.isValid()) {
        QDBusError err(xml.error());
        if (err.isValid()) {
            emit busError(tr("Call to object %1 at %2:\n  %3 (%4) failed\n")
                                  .arg(path)
                                  .arg(service)
                                  .arg(err.name())
                                  .arg(err.message()));
        } else {
            emit busError(tr("Invalid XML received from object %1 at %2\n").arg(path).arg(service));
        }
        return doc;
    }

    doc.setContent(xml.value());
    return doc;
}

void QDBusModel::addMethods(QDBusItem *parent, const QDomElement &iface)
{
    Q_ASSERT(parent);

    QDomElement child = iface.firstChildElement();
    while (!child.isNull()) {
        QDBusItem *item = nullptr;
        if (child.tagName() == "method"_L1) {
            item = new QDBusItem(QDBusModel::MethodItem, child.attribute("name"_L1), parent);
            item->caption = tr("Method: %1").arg(item->name);
            //get "type" from <arg> where "direction" is "in"
            QDomElement n = child.firstChildElement();
            while (!n.isNull()) {
                if (n.attribute("direction"_L1) == "in"_L1)
                    item->typeSignature += n.attribute("type"_L1);
                n = n.nextSiblingElement();
            }
        } else if (child.tagName() == "signal"_L1) {
            item = new QDBusItem(QDBusModel::SignalItem, child.attribute("name"_L1), parent);
            item->caption = tr("Signal: %1").arg(item->name);
        } else if (child.tagName() == "property"_L1) {
            item = new QDBusItem(QDBusModel::PropertyItem, child.attribute("name"_L1), parent);
            item->caption = tr("Property: %1").arg(item->name);
        } else {
            qDebug() << "addMethods: unknown tag:" << child.tagName();
        }
        if (item)
            parent->children.append(item);

        child = child.nextSiblingElement();
    }
}

void QDBusModel::addPath(QDBusItem *parent)
{
    Q_ASSERT(parent);

    QString path = parent->path();

    QDomDocument doc = introspect(path);
    QDomElement node = doc.documentElement();
    QDomElement child = node.firstChildElement();
    while (!child.isNull()) {
        if (child.tagName() == "node"_L1) {
            QDBusItem *item = new QDBusItem(QDBusModel::PathItem,
                                            child.attribute("name"_L1) + '/'_L1, parent);
            parent->children.append(item);

            addMethods(item, child);
        } else if (child.tagName() == "interface"_L1) {
            QDBusItem *item =
                    new QDBusItem(QDBusModel::InterfaceItem, child.attribute("name"_L1), parent);
            parent->children.append(item);

            addMethods(item, child);
        } else {
            qDebug() << "addPath: Unknown tag name:" << child.tagName();
        }
        child = child.nextSiblingElement();
    }

    parent->isPrefetched = true;
}

QDBusModel::QDBusModel(const QString &aService, const QDBusConnection &connection)
    : service(aService), c(connection), root(0)
{
    root = new QDBusItem(QDBusModel::PathItem, "/"_L1);
}

QDBusModel::~QDBusModel()
{
    delete root;
}

QModelIndex QDBusModel::index(int row, int column, const QModelIndex &parent) const
{
    const QDBusItem *item = static_cast<QDBusItem *>(parent.internalPointer());
    if (!item)
        item = root;

    if (column != 0 || row < 0 || row >= item->children.size())
        return QModelIndex();

    return createIndex(row, 0, item->children.at(row));
}

QModelIndex QDBusModel::parent(const QModelIndex &child) const
{
    QDBusItem *item = static_cast<QDBusItem *>(child.internalPointer());
    if (!item || !item->parent || !item->parent->parent)
        return QModelIndex();

    return createIndex(item->parent->parent->children.indexOf(item->parent), 0, item->parent);
}

int QDBusModel::rowCount(const QModelIndex &parent) const
{
    QDBusItem *item = static_cast<QDBusItem *>(parent.internalPointer());
    if (!item)
        item = root;
    if (!item->isPrefetched)
        const_cast<QDBusModel *>(this)->addPath(item);

    return item->children.size();
}

int QDBusModel::columnCount(const QModelIndex &) const
{
    return 1;
}

QVariant QDBusModel::data(const QModelIndex &index, int role) const
{
    const QDBusItem *item = static_cast<QDBusItem *>(index.internalPointer());
    if (!item)
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    return item->caption.isEmpty() ? item->name : item->caption;
}

QVariant QDBusModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation == Qt::Vertical || section != 0)
        return QVariant();

    return tr("Methods");
}

QDBusModel::Type QDBusModel::itemType(const QModelIndex &index) const
{
    const QDBusItem *item = static_cast<QDBusItem *>(index.internalPointer());
    return item ? item->type : PathItem;
}

void QDBusModel::refresh(const QModelIndex &aIndex)
{
    QModelIndex index = aIndex;
    while (index.isValid() && static_cast<QDBusItem *>(index.internalPointer())->type != PathItem) {
        index = index.parent();
    }

    QDBusItem *item = static_cast<QDBusItem *>(index.internalPointer());
    if (!item)
        item = root;

    if (!item->children.isEmpty()) {
        beginRemoveRows(index, 0, item->children.size() - 1);
        qDeleteAll(item->children);
        item->children.clear();
        endRemoveRows();
    }

    addPath(item);
    if (!item->children.isEmpty()) {
        beginInsertRows(index, 0, item->children.size() - 1);
        endInsertRows();
    }
}

QString QDBusModel::dBusPath(const QModelIndex &aIndex) const
{
    QModelIndex index = aIndex;
    while (index.isValid() && static_cast<QDBusItem *>(index.internalPointer())->type != PathItem) {
        index = index.parent();
    }

    QDBusItem *item = static_cast<QDBusItem *>(index.internalPointer());
    if (!item)
        item = root;

    return item->path();
}

QString QDBusModel::dBusInterface(const QModelIndex &index) const
{
    QDBusItem *item = static_cast<QDBusItem *>(index.internalPointer());
    if (!item)
        return QString();
    if (item->type == InterfaceItem)
        return item->name;
    if (item->parent && item->parent->type == InterfaceItem)
        return item->parent->name;
    return QString();
}

QString QDBusModel::dBusMethodName(const QModelIndex &index) const
{
    QDBusItem *item = static_cast<QDBusItem *>(index.internalPointer());
    return item ? item->name : QString();
}

QString QDBusModel::dBusTypeSignature(const QModelIndex &index) const
{
    QDBusItem *item = static_cast<QDBusItem *>(index.internalPointer());
    return item ? item->typeSignature : QString();
}

QModelIndex QDBusModel::findObject(const QDBusObjectPath &objectPath)
{
    QStringList path = objectPath.path().split('/'_L1, Qt::SkipEmptyParts);

    QDBusItem *item = root;
    int childIdx = -1;
    while (item && !path.isEmpty()) {
        const QString branch = path.takeFirst() + '/'_L1;
        childIdx = -1;

        // do a linear search over all the children
        for (int i = 0; i < item->children.size(); ++i) {
            QDBusItem *child = item->children.at(i);
            if (child->type == PathItem && child->name == branch) {
                item = child;
                childIdx = i;

                // prefetch the found branch
                if (!item->isPrefetched)
                    addPath(item);
                break;
            }
        }

        // branch not found - bail out
        if (childIdx == -1)
            return QModelIndex();
    }

    // found the right item
    if (childIdx != -1 && item && path.isEmpty())
        return createIndex(childIdx, 0, item);

    return QModelIndex();
}

