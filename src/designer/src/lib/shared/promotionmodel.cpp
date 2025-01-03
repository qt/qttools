// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "promotionmodel_p.h"
#include "widgetdatabase_p.h"

#include <QtDesigner/abstractwidgetdatabase.h>
#include <QtDesigner/abstractpromotioninterface.h>
#include <QtDesigner/abstractformeditor.h>

#include <QtGui/qstandarditemmodel.h>
#include <QtCore/qcoreapplication.h>

QT_BEGIN_NAMESPACE

namespace {
    using StandardItemList = QList<QStandardItem *>;

    // Model columns.
    enum { ClassNameColumn, IncludeFileColumn, IncludeTypeColumn, ReferencedColumn, NumColumns };

    // Create a model row.
    StandardItemList modelRow() {
        StandardItemList rc;
        for (int i = 0; i < NumColumns; i++) {
            rc.push_back(new QStandardItem());
        }
        return rc;
    }

    // Create a model row for a base class (read-only, cannot be selected).
    StandardItemList baseModelRow(const QDesignerWidgetDataBaseItemInterface *dbItem) {
        StandardItemList rc =  modelRow();

        rc[ClassNameColumn]->setText(dbItem->name());
        for (int i = 0; i < NumColumns; i++) {
            rc[i]->setFlags(Qt::ItemIsEnabled);
        }
        return rc;
    }

    // Create an editable model row for a promoted class.
    StandardItemList promotedModelRow(QDesignerWidgetDataBaseItemInterface *baseItem,
                                      QDesignerWidgetDataBaseItemInterface *dbItem,
                                      bool referenced)
    {
        qdesigner_internal::PromotionModel::ModelData data;
        data.baseItem = baseItem;
        data.promotedItem = dbItem;
        data.referenced = referenced;

        const QVariant userData = QVariant::fromValue(data);

        StandardItemList rc =  modelRow();
        // name
        rc[ClassNameColumn]->setText(dbItem->name());
        rc[ClassNameColumn]->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled|Qt::ItemIsEditable);
        rc[ClassNameColumn]->setData(userData);
        // header
        const qdesigner_internal::IncludeSpecification spec = qdesigner_internal::includeSpecification(dbItem->includeFile());
        rc[IncludeFileColumn]->setText(spec.first);
        rc[IncludeFileColumn]->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled|Qt::ItemIsEditable);
        rc[IncludeFileColumn]->setData(userData);
        // global include
        rc[IncludeTypeColumn]->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled|Qt::ItemIsEditable|Qt::ItemIsUserCheckable);
        rc[IncludeTypeColumn]->setData(userData);
        rc[IncludeTypeColumn]->setCheckState(spec.second == qdesigner_internal::IncludeGlobal ? Qt::Checked : Qt::Unchecked);
        // referenced
        rc[ReferencedColumn]->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        rc[ClassNameColumn]->setData(userData);
        if (!referenced) {
            //: Usage of promoted widgets
            static const QString notUsed = QCoreApplication::translate("PromotionModel", "Not used");
            rc[ReferencedColumn]->setText(notUsed);
        }
        return rc;
    }
}

namespace qdesigner_internal {

    PromotionModel::PromotionModel(QDesignerFormEditorInterface *core) :
        m_core(core)
    {
        connect(this, &QStandardItemModel::itemChanged, this, &PromotionModel::slotItemChanged);
    }

    void PromotionModel::initializeHeaders() {
        setColumnCount(NumColumns);
        QStringList horizontalLabels(tr("Name"));
        horizontalLabels += tr("Header file");
        horizontalLabels += tr("Global include");
        horizontalLabels += tr("Usage");
        setHorizontalHeaderLabels (horizontalLabels);
    }

    void PromotionModel::updateFromWidgetDatabase() {
        using PromotedClasses = QDesignerPromotionInterface::PromotedClasses;

        clear();
        initializeHeaders();

        // retrieve list of pairs from DB and convert into a tree structure.
        // Set the item index as user data on the item.
        const PromotedClasses promotedClasses = m_core->promotion()->promotedClasses();

        if (promotedClasses.isEmpty())
            return;

        const QSet<QString> usedPromotedClasses = m_core->promotion()->referencedPromotedClassNames();

        QDesignerWidgetDataBaseItemInterface *baseClass = nullptr;
        QStandardItem *baseItem = nullptr;

        for (auto &pi : promotedClasses) {
            // Start a new base class?
            if (baseClass != pi.baseItem) {
                baseClass = pi.baseItem;
                const StandardItemList baseRow = baseModelRow(pi.baseItem);
                baseItem = baseRow.constFirst();
                appendRow(baseRow);
            }
            Q_ASSERT(baseItem);
            // Append derived
            baseItem->appendRow(promotedModelRow(pi.baseItem, pi.promotedItem,
                                                 usedPromotedClasses.contains(pi.promotedItem->name())));
        }
    }

    void PromotionModel::slotItemChanged(QStandardItem * changedItem) {
        // Retrieve DB item
        const ModelData data = modelData(changedItem);
        Q_ASSERT(data.isValid());
        QDesignerWidgetDataBaseItemInterface *dbItem = data.promotedItem;
        // Change header or type
        switch (changedItem->column()) {
        case ClassNameColumn:
            emit classNameChanged(dbItem,  changedItem->text());
            break;
        case IncludeTypeColumn:
        case IncludeFileColumn: {
            // Get both file and type items via parent.
            const QStandardItem *baseClassItem = changedItem->parent();
            const QStandardItem *fileItem = baseClassItem->child(changedItem->row(), IncludeFileColumn);
            const QStandardItem *typeItem =  baseClassItem->child(changedItem->row(), IncludeTypeColumn);
            emit includeFileChanged(dbItem, buildIncludeFile(fileItem->text(), typeItem->checkState() == Qt::Checked ? IncludeGlobal : IncludeLocal));
        }
            break;
        }
    }

    PromotionModel::ModelData PromotionModel::modelData(const QStandardItem *item) const
    {
        const QVariant userData = item->data();
        return userData.canConvert<ModelData>() ? userData.value<ModelData>() : ModelData();
    }

    PromotionModel::ModelData PromotionModel::modelData(const QModelIndex &index) const
    {
        return index.isValid() ? modelData(itemFromIndex(index)) : ModelData();
    }

    QModelIndex PromotionModel::indexOfClass(const QString &className) const {
        const StandardItemList matches = findItems (className, Qt::MatchFixedString|Qt::MatchCaseSensitive|Qt::MatchRecursive);
        return matches.isEmpty() ? QModelIndex() : indexFromItem (matches.constFirst());
    }
} // namespace qdesigner_internal

QT_END_NAMESPACE
