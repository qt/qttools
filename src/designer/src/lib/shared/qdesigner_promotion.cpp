// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "qdesigner_promotion_p.h"
#include "widgetdatabase_p.h"
#include "metadatabase_p.h"
#include "widgetdatabase_p.h"

#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/abstractformwindow.h>
#include <QtDesigner/abstractformwindowmanager.h>
#include <QtDesigner/abstractobjectinspector.h>
#include <QtDesigner/abstractwidgetbox.h>
#include <QtDesigner/abstractwidgetdatabase.h>

#include <QtCore/qmap.h>
#include <QtCore/qcoreapplication.h>
#include <qdebug.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

namespace {
    // Return a set of on-promotable classes
    const QSet<QString> &nonPromotableClasses() {
        static const QSet<QString> rc = {
            u"Line"_s,
            u"QAction"_s,
            u"Spacer"_s,
            u"QMainWindow"_s,
            u"QDialog"_s,
            u"QMdiArea"_s,
            u"QMdiSubWindow"_s
        };
        return rc;
    }

    // Return widget database index of a promoted class or -1 with error message
    int promotedWidgetDataBaseIndex(const QDesignerWidgetDataBaseInterface *widgetDataBase,
                                                                 const QString &className,
                                                                 QString *errorMessage) {
        const int index = widgetDataBase->indexOfClassName(className);
        if (index == -1 || !widgetDataBase->item(index)->isPromoted()) {
            *errorMessage = QCoreApplication::tr("%1 is not a promoted class.").arg(className);
            return -1;
        }
        return index;
    }

    // Return widget database item of a promoted class or 0 with error message
    QDesignerWidgetDataBaseItemInterface *promotedWidgetDataBaseItem(const QDesignerWidgetDataBaseInterface *widgetDataBase,
                                                                     const QString &className,
                                                                     QString *errorMessage) {

        const int index =  promotedWidgetDataBaseIndex(widgetDataBase, className, errorMessage);
        if (index == -1)
            return nullptr;
        return widgetDataBase->item(index);
    }

    // extract class name from xml  "<widget class="QWidget" ...>". Quite a hack.
    QString classNameFromXml(QString xml)
    {
        constexpr auto tag = "class=\""_L1;
        const int pos = xml.indexOf(tag);
        if (pos == -1)
            return QString();
        xml.remove(0, pos + tag.size());
        const auto closingPos = xml.indexOf(u'"');
        if (closingPos == -1)
            return QString();
        xml.remove(closingPos, xml.size() - closingPos);
        return xml;
    }

    // return a list of class names in the scratch pad
    QStringList getScratchPadClasses(const QDesignerWidgetBoxInterface *wb) {
        QStringList rc;
        const int catCount =  wb->categoryCount();
        for (int c = 0; c <  catCount; c++) {
            const QDesignerWidgetBoxInterface::Category category = wb->category(c);
            if (category.type() == QDesignerWidgetBoxInterface::Category::Scratchpad) {
                const int widgetCount = category.widgetCount();
                for (int w = 0; w < widgetCount; w++) {
                    const QString className = classNameFromXml( category.widget(w).domXml());
                    if (!className.isEmpty())
                        rc += className;
                }
            }
        }
        return rc;
    }
}

static void markFormsDirty(const QDesignerFormEditorInterface *core)
{
    const QDesignerFormWindowManagerInterface *fwm = core->formWindowManager();
    for (int f = 0, count = fwm->formWindowCount(); f < count; ++f)
        fwm->formWindow(f)->setDirty(true);
}

namespace qdesigner_internal {

    QDesignerPromotion::QDesignerPromotion(QDesignerFormEditorInterface *core) :
        m_core(core)  {
    }

    bool  QDesignerPromotion::addPromotedClass(const QString &baseClass,
                                               const QString &className,
                                               const QString &includeFile,
                                               QString *errorMessage)
    {
        QDesignerWidgetDataBaseInterface *widgetDataBase = m_core->widgetDataBase();
        const int baseClassIndex = widgetDataBase->indexOfClassName(baseClass);

        if (baseClassIndex == -1) {
            *errorMessage = QCoreApplication::tr("The base class %1 is invalid.").arg(baseClass);
            return false;
        }

        const int existingClassIndex = widgetDataBase->indexOfClassName(className);

        if (existingClassIndex != -1) {
            *errorMessage = QCoreApplication::tr("The class %1 already exists.").arg(className);
            return false;
        }
        // Clone derived item.
        QDesignerWidgetDataBaseItemInterface *promotedItem = WidgetDataBaseItem::clone(widgetDataBase->item(baseClassIndex));
        // Also inherit the container flag in case of QWidget-derived classes
        // as it is most likely intended for stacked pages.
        // set new props
        promotedItem->setName(className);
        promotedItem->setGroup(QCoreApplication::tr("Promoted Widgets"));
        promotedItem->setCustom(true);
        promotedItem->setPromoted(true);
        promotedItem->setExtends(baseClass);
        promotedItem->setIncludeFile(includeFile);
        widgetDataBase->append(promotedItem);
        markFormsDirty(m_core);
        return true;
    }

    QList<QDesignerWidgetDataBaseItemInterface *> QDesignerPromotion::promotionBaseClasses() const
    {
        using SortedDatabaseItemMap = QMap<QString, QDesignerWidgetDataBaseItemInterface *>;
        SortedDatabaseItemMap sortedDatabaseItemMap;

        QDesignerWidgetDataBaseInterface *widgetDataBase = m_core->widgetDataBase();

        const int cnt = widgetDataBase->count();
        for (int i = 0; i <  cnt; i++) {
            QDesignerWidgetDataBaseItemInterface *dbItem = widgetDataBase->item(i);
            if (canBePromoted(dbItem)) {
                sortedDatabaseItemMap.insert(dbItem->name(), dbItem);
            }
        }

        return sortedDatabaseItemMap.values();
    }


    bool QDesignerPromotion::canBePromoted(const QDesignerWidgetDataBaseItemInterface *dbItem) const
    {
        if (dbItem->isPromoted() ||  !dbItem->extends().isEmpty())
            return false;

        const QString name = dbItem->name();

        if (nonPromotableClasses().contains(name))
            return false;

        if (name.startsWith("QDesigner"_L1) || name.startsWith("QLayout"_L1))
            return false;

        return true;
    }

    QDesignerPromotion::PromotedClasses QDesignerPromotion::promotedClasses()  const
    {
        using ClassNameItemMap = QMap<QString, QDesignerWidgetDataBaseItemInterface *>;
        // A map containing base classes and their promoted classes.
        QMap<QString, ClassNameItemMap> baseClassPromotedMap;

        QDesignerWidgetDataBaseInterface *widgetDataBase = m_core->widgetDataBase();
        // Look for promoted classes and insert into map according to base class.
        const  int cnt = widgetDataBase->count();
        for (int i = 0; i < cnt; i++) {
            QDesignerWidgetDataBaseItemInterface *dbItem = widgetDataBase->item(i);
            if (dbItem->isPromoted()) {
                const QString baseClassName = dbItem->extends();
                auto it = baseClassPromotedMap.find(baseClassName);
                if (it == baseClassPromotedMap.end()) {
                    it = baseClassPromotedMap.insert(baseClassName, ClassNameItemMap());
                }
                it.value().insert(dbItem->name(), dbItem);
            }
        }
        // convert map into list.
        PromotedClasses rc;

        if (baseClassPromotedMap.isEmpty())
            return rc;

        for (auto bit = baseClassPromotedMap.cbegin(), bcend = baseClassPromotedMap.cend(); bit != bcend; ++bit) {
            const int baseIndex = widgetDataBase->indexOfClassName(bit.key());
            Q_ASSERT(baseIndex >= 0);
            QDesignerWidgetDataBaseItemInterface *baseItem = widgetDataBase->item(baseIndex);
            // promoted
            for (auto pit = bit.value().cbegin(), pcend = bit.value().cend(); pit != pcend; ++pit) {
                PromotedClass item;
                item.baseItem = baseItem;
                item.promotedItem = pit.value();
                rc.push_back(item);
            }
        }

        return rc;
    }

    QSet<QString> QDesignerPromotion::referencedPromotedClassNames()  const {
        QSet<QString> rc;
        const MetaDataBase *metaDataBase = qobject_cast<const MetaDataBase*>(m_core->metaDataBase());
        if (!metaDataBase)
            return rc;

        const QObjectList &objects = metaDataBase->objects();
        for (QObject *object : objects) {
            const QString customClass = metaDataBase->metaDataBaseItem(object)->customClassName();
            if (!customClass.isEmpty())
                rc.insert(customClass);

        }
        // check the scratchpad of the widget box
        if (QDesignerWidgetBoxInterface *widgetBox = m_core->widgetBox()) {
            const QStringList scratchPadClasses = getScratchPadClasses(widgetBox);
            if (!scratchPadClasses.isEmpty()) {
                // Check whether these are actually promoted
                QDesignerWidgetDataBaseInterface *widgetDataBase = m_core->widgetDataBase();
                for (const auto &scItem : scratchPadClasses) {
                    const int index = widgetDataBase->indexOfClassName(scItem);
                    if (index != -1 && widgetDataBase->item(index)->isPromoted())
                        rc.insert(scItem);
                }
            }
        }
        return rc;
    }

    bool QDesignerPromotion::removePromotedClass(const QString &className, QString *errorMessage) {
        // check if it exists and is promoted
        WidgetDataBase *widgetDataBase = qobject_cast<WidgetDataBase *>(m_core->widgetDataBase());
        if (!widgetDataBase) {
            *errorMessage = QCoreApplication::tr("The class %1 cannot be removed").arg(className);
            return false;
        }

        const int index = promotedWidgetDataBaseIndex(widgetDataBase, className, errorMessage);
        if (index == -1)
            return false;

        if (referencedPromotedClassNames().contains(className)) {
            *errorMessage = QCoreApplication::tr("The class %1 cannot be removed because it is still referenced.").arg(className);
            return false;
        }
        // QTBUG-52963: Check for classes that specify the to-be-removed class as
        // base class of a promoted class. This should not happen in the normal case
        // as promoted classes cannot serve as base for further promotion. It is possible
        // though if a class provided by a plugin (say Qt WebKit's QWebView) is used as
        // a base class for a promoted widget B and the plugin is removed in the next
        // launch. QWebView will then appear as promoted class itself and the promoted
        // class B will depend on it. When removing QWebView, the base class of B will
        // be changed to that of QWebView by the below code.
        const PromotedClasses promotedList = promotedClasses();
        for (const auto &pc : promotedList) {
            if (pc.baseItem->name() == className) {
                const QString extends = widgetDataBase->item(index)->extends();
                qWarning().nospace() << "Warning: Promoted class " << pc.promotedItem->name()
                    << " extends " << className << ", changing its base class to " <<  extends << '.';
                pc.promotedItem->setExtends(extends);
            }
        }
        widgetDataBase->remove(index);
        markFormsDirty(m_core);
        return true;
    }

    bool QDesignerPromotion::changePromotedClassName(const QString &oldclassName, const QString &newClassName, QString *errorMessage) {
        const MetaDataBase *metaDataBase = qobject_cast<const MetaDataBase*>(m_core->metaDataBase());
        if (!metaDataBase) {
            *errorMessage = QCoreApplication::tr("The class %1 cannot be renamed").arg(oldclassName);
            return false;
        }
        QDesignerWidgetDataBaseInterface *widgetDataBase = m_core->widgetDataBase();

        // check the new name
        if (newClassName.isEmpty()) {
            *errorMessage = QCoreApplication::tr("The class %1 cannot be renamed to an empty name.").arg(oldclassName);
            return false;
        }
        const int existingIndex = widgetDataBase->indexOfClassName(newClassName);
        if (existingIndex != -1) {
            *errorMessage = QCoreApplication::tr("There is already a class named %1.").arg(newClassName);
            return false;
        }
        // Check old class
        QDesignerWidgetDataBaseItemInterface *dbItem = promotedWidgetDataBaseItem(widgetDataBase, oldclassName, errorMessage);
        if (!dbItem)
            return false;

        // Change the name in the data base and change all referencing objects in the meta database
        dbItem->setName(newClassName);
        bool foundReferences = false;
        const QObjectList &dbObjects = metaDataBase->objects();
        for (QObject* object : dbObjects) {
            MetaDataBaseItem *item =  metaDataBase->metaDataBaseItem(object);
            Q_ASSERT(item);
            if (item->customClassName() == oldclassName) {
                item->setCustomClassName(newClassName);
                foundReferences = true;
            }
        }
        // set state
        if (foundReferences)
            refreshObjectInspector();

        markFormsDirty(m_core);
        return true;
    }

    bool QDesignerPromotion::setPromotedClassIncludeFile(const QString &className, const QString &includeFile, QString *errorMessage) {
        // check file
        if (includeFile.isEmpty()) {
            *errorMessage = QCoreApplication::tr("Cannot set an empty include file.");
            return false;
        }
        // check item
        QDesignerWidgetDataBaseInterface *widgetDataBase = m_core->widgetDataBase();
        QDesignerWidgetDataBaseItemInterface *dbItem = promotedWidgetDataBaseItem(widgetDataBase, className, errorMessage);
        if (!dbItem)
            return false;
        if (dbItem->includeFile() != includeFile) {
            dbItem->setIncludeFile(includeFile);
            markFormsDirty(m_core);
        }
        return true;
    }

    void QDesignerPromotion::refreshObjectInspector() {
        if (QDesignerFormWindowManagerInterface *fwm = m_core->formWindowManager()) {
            if (QDesignerFormWindowInterface *fw = fwm->activeFormWindow())
                if ( QDesignerObjectInspectorInterface *oi = m_core->objectInspector()) {
                    oi->setFormWindow(fw);
                }
        }
    }
}

QT_END_NAMESPACE
