// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef OBJECTINSPECTORMODEL_H
#define OBJECTINSPECTORMODEL_H

#include <layoutinfo_p.h>

#include <QtGui/qstandarditemmodel.h>
#include <QtGui/qicon.h>
#include <QtCore/qcompare.h>
#include <QtCore/qstring.h>
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include <QtCore/qpointer.h>

QT_BEGIN_NAMESPACE

class QDesignerFormWindowInterface;

namespace qdesigner_internal {

    // Data structure containing the fixed item type icons
    struct ObjectInspectorIcons {
        QIcon layoutIcons[LayoutInfo::UnknownLayout + 1];
    };

    struct ModelRecursionContext;

    // Data structure representing one item of the object inspector.
    class ObjectData {
    public:
        enum Type {
            Object,
            Action,
            SeparatorAction,
            ChildWidget,         // A child widget
            LayoutableContainer, // A container that can be laid out
            LayoutWidget,        // A QLayoutWidget
            ExtensionContainer   // QTabWidget and the like, container extension
        };

        using StandardItemList = QList<QStandardItem *>;

        explicit ObjectData(QObject *parent, QObject *object, const ModelRecursionContext &ctx);
        ObjectData();

        inline Type     type()       const { return m_type; }
        inline QObject *object()     const { return m_object; }
        inline QObject *parent()     const { return m_parent; }
        inline QString  objectName() const { return m_objectName; }

        bool equals(const ObjectData & me) const;

        enum ChangedMask { ClassNameChanged = 1, ObjectNameChanged = 2,
                           ClassIconChanged = 4, TypeChanged = 8,
                           LayoutTypeChanged = 16};

        unsigned compare(const ObjectData & me) const;

        // Initially set up a row
        void setItems(const StandardItemList &row, const ObjectInspectorIcons &icons) const;
        // Update row data according to change mask
        void setItemsDisplayData(const StandardItemList &row, const ObjectInspectorIcons &icons, unsigned mask) const;

    private:
        friend bool comparesEqual(const ObjectData &lhs, const ObjectData &rhs) noexcept
        {
            return lhs.m_parent == rhs.m_parent && lhs.m_object == rhs.m_object;
        }
        Q_DECLARE_EQUALITY_COMPARABLE(ObjectData)

        void initObject(const ModelRecursionContext &ctx);
        void initWidget(QWidget *w, const ModelRecursionContext &ctx);

        QObject *m_parent = nullptr;
        QObject *m_object = nullptr;
        Type m_type = Object;
        QString m_className;
        QString m_objectName;
        QIcon m_classIcon;
        LayoutInfo::Type m_managedLayoutType = LayoutInfo::NoLayout;
    };

    using ObjectModel = QList<ObjectData>;

    // QStandardItemModel for ObjectInspector. Uses ObjectData/ObjectModel
    // internally for its updates.
    class ObjectInspectorModel : public QStandardItemModel {
    public:
        using StandardItemList = QList<QStandardItem *>;
        enum { ObjectNameColumn, ClassNameColumn, NumColumns };

        explicit ObjectInspectorModel(QObject *parent);

        enum UpdateResult { NoForm, Rebuilt, Updated };
        UpdateResult update(QDesignerFormWindowInterface *fw);

        const QModelIndexList indexesOf(QObject *o) const { return m_objectIndexMultiMap.values(o); }
        QObject *objectAt(const QModelIndex &index) const;

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    private:
        void rebuild(const ObjectModel &newModel);
        void updateItemContents(ObjectModel &oldModel, const ObjectModel &newModel);
        void clearItems();
        StandardItemList rowAt(QModelIndex index) const;

        ObjectInspectorIcons m_icons;
        QMultiMap<QObject *, QModelIndex> m_objectIndexMultiMap;
        ObjectModel m_model;
        QPointer<QDesignerFormWindowInterface> m_formWindow;
    };
}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // OBJECTINSPECTORMODEL_H
