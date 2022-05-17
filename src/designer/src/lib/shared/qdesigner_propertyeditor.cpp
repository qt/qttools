// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "qdesigner_propertyeditor_p.h"
#include "pluginmanager_p.h"

#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/dynamicpropertysheet.h>
#include <QtDesigner/propertysheet.h>
#include <QtDesigner/qextensionmanager.h>
#include <widgetfactory_p.h>

#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qabstractbutton.h>

#include <QtGui/qaction.h>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {
using StringPropertyParameters = QDesignerPropertyEditor::StringPropertyParameters;
// A map of property name to type
using PropertyNameTypeMap = QHash<QString, StringPropertyParameters>;

// Compile a map of hard-coded string property types
static const PropertyNameTypeMap &stringPropertyTypes()
{
    static PropertyNameTypeMap propertyNameTypeMap;
    if (propertyNameTypeMap.isEmpty()) {
        const StringPropertyParameters richtext(ValidationRichText, true);
        // Accessibility. Both are texts the narrator reads
        propertyNameTypeMap.insert(QStringLiteral("accessibleDescription"), richtext);
        propertyNameTypeMap.insert(QStringLiteral("accessibleName"), richtext);
        // object names
        const StringPropertyParameters objectName(ValidationObjectName, false);
        propertyNameTypeMap.insert(QStringLiteral("buddy"), objectName);
        propertyNameTypeMap.insert(QStringLiteral("currentItemName"), objectName);
        propertyNameTypeMap.insert(QStringLiteral("currentPageName"), objectName);
        propertyNameTypeMap.insert(QStringLiteral("currentTabName"), objectName);
        propertyNameTypeMap.insert(QStringLiteral("layoutName"), objectName);
        propertyNameTypeMap.insert(QStringLiteral("spacerName"), objectName);
        // Style sheet
        propertyNameTypeMap.insert(QStringLiteral("styleSheet"), StringPropertyParameters(ValidationStyleSheet, false));
        // Buttons/  QCommandLinkButton
        const StringPropertyParameters multiline(ValidationMultiLine, true);
        propertyNameTypeMap.insert(QStringLiteral("description"), multiline);
        propertyNameTypeMap.insert(QStringLiteral("iconText"), multiline);
        // Tooltips, etc.
        propertyNameTypeMap.insert(QStringLiteral("toolTip"), richtext);
        propertyNameTypeMap.insert(QStringLiteral("whatsThis"), richtext);
        propertyNameTypeMap.insert(QStringLiteral("windowIconText"), richtext);
        propertyNameTypeMap.insert(QStringLiteral("html"), richtext);
        //  A QWizard page id
        propertyNameTypeMap.insert(QStringLiteral("pageId"), StringPropertyParameters(ValidationSingleLine, false));
        // QPlainTextEdit
        propertyNameTypeMap.insert(QStringLiteral("plainText"), StringPropertyParameters(ValidationMultiLine, true));
    }
    return propertyNameTypeMap;
}

QDesignerPropertyEditor::QDesignerPropertyEditor(QWidget *parent, Qt::WindowFlags flags) :
    QDesignerPropertyEditorInterface(parent, flags)
{
    // Make old signal work for  compatibility
    connect(this, &QDesignerPropertyEditorInterface::propertyChanged,
            this, &QDesignerPropertyEditor::slotPropertyChanged);
}

static inline bool isDynamicProperty(QDesignerFormEditorInterface *core, QObject *object,
                                     const QString &propertyName)
{
    if (const QDesignerDynamicPropertySheetExtension *dynamicSheet = qt_extension<QDesignerDynamicPropertySheetExtension*>(core->extensionManager(), object)) {
        if (dynamicSheet->dynamicPropertiesAllowed()) {
            if (QDesignerPropertySheetExtension *propertySheet = qt_extension<QDesignerPropertySheetExtension*>(core->extensionManager(), object)) {
                const int index = propertySheet->indexOf(propertyName);
                return index >= 0 && dynamicSheet->isDynamicProperty(index);
            }
        }
    }
    return false;
}

QDesignerPropertyEditor::StringPropertyParameters QDesignerPropertyEditor::textPropertyValidationMode(
        QDesignerFormEditorInterface *core, const QObject *object,
        const QString &propertyName, bool isMainContainer)
{
    // object name - no comment
    if (propertyName == QStringLiteral("objectName")) {
        const TextPropertyValidationMode vm =  isMainContainer ? ValidationObjectNameScope : ValidationObjectName;
        return StringPropertyParameters(vm, false);
    }

    // Check custom widgets by class.
    const QString className = WidgetFactory::classNameOf(core, object);
    const QDesignerCustomWidgetData customData = core->pluginManager()->customWidgetData(className);
    if (!customData.isNull()) {
        StringPropertyParameters customType;
        if (customData.xmlStringPropertyType(propertyName, &customType))
            return customType;
    }

    if (isDynamicProperty(core, const_cast<QObject *>(object), propertyName))
        return StringPropertyParameters(ValidationMultiLine, true);

    // Check hardcoded property ames
   const PropertyNameTypeMap::const_iterator hit = stringPropertyTypes().constFind(propertyName);
   if (hit != stringPropertyTypes().constEnd())
       return hit.value();

    // text: Check according to widget type.
    if (propertyName == QStringLiteral("text")) {
        if (qobject_cast<const QAction *>(object) || qobject_cast<const QLineEdit *>(object))
            return StringPropertyParameters(ValidationSingleLine, true);
        if (qobject_cast<const QAbstractButton *>(object))
            return StringPropertyParameters(ValidationMultiLine, true);
        return StringPropertyParameters(ValidationRichText, true);
    }

   // Fuzzy matching
    if (propertyName.endsWith(QStringLiteral("Name")))
        return StringPropertyParameters(ValidationSingleLine, true);

    if (propertyName.endsWith(QStringLiteral("ToolTip")))
        return StringPropertyParameters(ValidationRichText, true);

#ifdef Q_OS_WIN // No translation for the active X "control" property
    if (propertyName == QStringLiteral("control") && className == QStringLiteral("QAxWidget"))
        return StringPropertyParameters(ValidationSingleLine, false);
#endif

    // default to single
    return StringPropertyParameters(ValidationSingleLine, true);
}

void QDesignerPropertyEditor::emitPropertyValueChanged(const QString &name, const QVariant &value, bool enableSubPropertyHandling)
{
    // Avoid duplicate signal emission - see below
    m_propertyChangedForwardingBlocked = true;
    emit propertyValueChanged(name, value, enableSubPropertyHandling);
    emit propertyChanged(name, value);
    m_propertyChangedForwardingBlocked = false;
}

void QDesignerPropertyEditor::slotPropertyChanged(const QString &name, const QVariant &value)
{
    // Forward signal from Integration using the old interfaces.
    if (!m_propertyChangedForwardingBlocked)
        emit propertyValueChanged(name, value, true);
}

}

QT_END_NAMESPACE
