// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "itemview_propertysheet.h"

#include <QtDesigner/abstractformeditor.h>

#include <QtWidgets/qabstractitemview.h>
#include <QtWidgets/qheaderview.h>
#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

namespace qdesigner_internal {

struct Property {
    Property() = default;
    Property(QDesignerPropertySheetExtension *sheet, int id)
        : m_sheet(sheet), m_id(id) {}

    QDesignerPropertySheetExtension *m_sheet{nullptr};
    int m_id{-1};
};

struct ItemViewPropertySheetPrivate {
    ItemViewPropertySheetPrivate(QDesignerFormEditorInterface *core,
                                 QHeaderView *horizontalHeader,
                                 QHeaderView *verticalHeader);

    inline QStringList realPropertyNames();
    inline QString fakePropertyName(const QString &prefix, const QString &realName);

    // Maps index of fake property to index of real property in respective sheet
    QMap<int, Property> m_propertyIdMap;

    // Maps name of fake property to name of real property
    QHash<QString, QString> m_propertyNameMap;

    QHash<QHeaderView *, QDesignerPropertySheetExtension *> m_propertySheet;
    QStringList m_realPropertyNames;
};

// Name of the fake group
static constexpr auto headerGroup = "Header"_L1;

// Name of the real properties
static constexpr auto visibleProperty = "visible"_L1;
static constexpr auto cascadingSectionResizesProperty = "cascadingSectionResizes"_L1;
static constexpr auto defaultSectionSizeProperty = "defaultSectionSize"_L1;
static constexpr auto highlightSectionsProperty = "highlightSections"_L1;
static constexpr auto minimumSectionSizeProperty = "minimumSectionSize"_L1;
static constexpr auto showSortIndicatorProperty = "showSortIndicator"_L1;
static constexpr auto stretchLastSectionProperty = "stretchLastSection"_L1;

/***************** ItemViewPropertySheetPrivate *********************/

ItemViewPropertySheetPrivate::ItemViewPropertySheetPrivate(QDesignerFormEditorInterface *core,
                                                           QHeaderView *horizontalHeader,
                                                           QHeaderView *verticalHeader)
{
    if (horizontalHeader)
        m_propertySheet.insert(horizontalHeader,
                               qt_extension<QDesignerPropertySheetExtension*>
                               (core->extensionManager(), horizontalHeader));
    if (verticalHeader)
        m_propertySheet.insert(verticalHeader,
                               qt_extension<QDesignerPropertySheetExtension*>
                               (core->extensionManager(), verticalHeader));
}

QStringList ItemViewPropertySheetPrivate::realPropertyNames()
{
    if (m_realPropertyNames.isEmpty())
        m_realPropertyNames = {
            visibleProperty, cascadingSectionResizesProperty,
            defaultSectionSizeProperty, highlightSectionsProperty,
            minimumSectionSizeProperty, showSortIndicatorProperty,
            stretchLastSectionProperty
        };
    return m_realPropertyNames;
}

QString ItemViewPropertySheetPrivate::fakePropertyName(const QString &prefix,
                                                       const QString &realName)
{
    // prefix = "header", realPropertyName = "isVisible" returns "headerIsVisible"
    QString fakeName = prefix + realName.at(0).toUpper() + realName.mid(1);
    m_propertyNameMap.insert(fakeName, realName);
    return fakeName;
}

/***************** ItemViewPropertySheet *********************/

/*!
  \class qdesigner_internal::ItemViewPropertySheet

  \brief
    Adds header fake properties to QTreeView and QTableView objects

    QHeaderView objects are currently not shown in the object inspector.
    This class adds some fake properties to the property sheet
    of QTreeView and QTableView objects that nevertheless allow the manipulation
    of the headers attached to the item view object.

    Currently the defaultAlignment property is not shown because the property sheet
    would only show integers, instead of the Qt::Alignment enumeration.

    The fake properties here need special handling in QDesignerResource, uiloader and uic.
  */

ItemViewPropertySheet::ItemViewPropertySheet(QTreeView *treeViewObject, QObject *parent)
        : QDesignerPropertySheet(treeViewObject, parent),
        d(new ItemViewPropertySheetPrivate(core(), treeViewObject->header(), nullptr))
{
    initHeaderProperties(treeViewObject->header(), u"header"_s);
}

ItemViewPropertySheet::ItemViewPropertySheet(QTableView *tableViewObject, QObject *parent)
        : QDesignerPropertySheet(tableViewObject, parent),
        d(new ItemViewPropertySheetPrivate(core(),
                                           tableViewObject->horizontalHeader(),
                                           tableViewObject->verticalHeader()))
{
    initHeaderProperties(tableViewObject->horizontalHeader(), u"horizontalHeader"_s);
    initHeaderProperties(tableViewObject->verticalHeader(), u"verticalHeader"_s);
}

ItemViewPropertySheet::~ItemViewPropertySheet()
{
    delete d;
}

void ItemViewPropertySheet::initHeaderProperties(QHeaderView *hv, const QString &prefix)
{
    QDesignerPropertySheetExtension *headerSheet = d->m_propertySheet.value(hv);
    Q_ASSERT(headerSheet);
    const QString headerGroupS = headerGroup;
    const QStringList &realPropertyNames = d->realPropertyNames();
    for (const QString &realPropertyName : realPropertyNames) {
        const int headerIndex = headerSheet->indexOf(realPropertyName);
        Q_ASSERT(headerIndex != -1);
        const QVariant defaultValue = realPropertyName == visibleProperty ?
                                      QVariant(true) : headerSheet->property(headerIndex);
        const QString fakePropertyName = d->fakePropertyName(prefix, realPropertyName);
        const int fakeIndex = createFakeProperty(fakePropertyName, defaultValue);
        d->m_propertyIdMap.insert(fakeIndex, Property(headerSheet, headerIndex));
        setAttribute(fakeIndex, true);
        setPropertyGroup(fakeIndex, headerGroupS);
    }
}

/*!
  Returns the mapping of fake property names to real property names
  */
QHash<QString,QString> ItemViewPropertySheet::propertyNameMap() const
{
    return d->m_propertyNameMap;
}

QVariant ItemViewPropertySheet::property(int index) const
{
    const auto it = d->m_propertyIdMap.constFind(index);
    if (it != d->m_propertyIdMap.constEnd())
        return it.value().m_sheet->property(it.value().m_id);
    return QDesignerPropertySheet::property(index);
}

void ItemViewPropertySheet::setProperty(int index, const QVariant &value)
{
    const auto it = d->m_propertyIdMap.find(index);
    if (it != d->m_propertyIdMap.end()) {
        it.value().m_sheet->setProperty(it.value().m_id, value);
    } else {
        QDesignerPropertySheet::setProperty(index, value);
    }
}

void ItemViewPropertySheet::setChanged(int index, bool changed)
{
    const auto it = d->m_propertyIdMap.find(index);
    if (it != d->m_propertyIdMap.end()) {
        it.value().m_sheet->setChanged(it.value().m_id, changed);
    } else {
        QDesignerPropertySheet::setChanged(index, changed);
    }
}

bool ItemViewPropertySheet::isChanged(int index) const
{
    const auto it = d->m_propertyIdMap.constFind(index);
    if (it != d->m_propertyIdMap.constEnd())
        return it.value().m_sheet->isChanged(it.value().m_id);
    return QDesignerPropertySheet::isChanged(index);
}

bool ItemViewPropertySheet::hasReset(int index) const
{
    const auto it = d->m_propertyIdMap.constFind(index);
    if (it != d->m_propertyIdMap.constEnd())
        return it.value().m_sheet->hasReset(it.value().m_id);
    return QDesignerPropertySheet::hasReset(index);
}

bool ItemViewPropertySheet::reset(int index)
{
    const auto it = d->m_propertyIdMap.find(index);
    if (it != d->m_propertyIdMap.end()) {
       QDesignerPropertySheetExtension *headerSheet = it.value().m_sheet;
       const int headerIndex = it.value().m_id;
       const bool resetRC = headerSheet->reset(headerIndex);
       // Resetting for "visible" might fail and the stored default
       // of the Widget database is "false" due to the widget not being
       // visible at the time it was determined. Reset to "true" manually.
       if (!resetRC && headerSheet->propertyName(headerIndex) == visibleProperty) {
           headerSheet->setProperty(headerIndex, QVariant(true));
           headerSheet->setChanged(headerIndex, false);
           return true;
       }
       return resetRC;
    }
    return QDesignerPropertySheet::reset(index);
}

} // namespace qdesigner_internal

QT_END_NAMESPACE
