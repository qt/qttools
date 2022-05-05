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

#ifndef FONTPROPERTYMANAGER_H
#define FONTPROPERTYMANAGER_H

#include <QtCore/qmap.h>
#include <QtCore/qstringlist.h>
#include <QtGui/qfont.h>

QT_BEGIN_NAMESPACE

class QtProperty;
class QtVariantPropertyManager;

class QString;
class QVariant;

namespace qdesigner_internal {

/* FontPropertyManager: A mixin for DesignerPropertyManager that manages font
 * properties. Adds an antialiasing subproperty and reset flags/mask handling
 * for the other subproperties. It also modifies the font family
 * enumeration names, which it reads from an XML mapping file that
 * contains annotations indicating the platform the font is available on. */

class FontPropertyManager {
    Q_DISABLE_COPY_MOVE(FontPropertyManager)

public:
    FontPropertyManager();

    using ResetMap = QMap<QtProperty *, bool>;
    using NameMap = QMap<QString, QString>;

    // Call before QtVariantPropertyManager::initializeProperty.
    void preInitializeProperty(QtProperty *property, int type, ResetMap &resetMap);
    // Call after QtVariantPropertyManager::initializeProperty. This will trigger
    // a recursion for the sub properties
    void postInitializeProperty(QtVariantPropertyManager *vm, QtProperty *property, int type, int enumTypeId);

    bool uninitializeProperty(QtProperty *property);

    // Call from  QtPropertyManager's propertyDestroyed signal
    void slotPropertyDestroyed(QtProperty *property);

    bool resetFontSubProperty(QtVariantPropertyManager *vm, QtProperty *subProperty);

    // Call from slotValueChanged(), returns DesignerPropertyManager::ValueChangedResult
    int valueChanged(QtVariantPropertyManager *vm, QtProperty *property, const QVariant &value);

    // Call from setValue() before calling setValue() on  QtVariantPropertyManager.
    void setValue(QtVariantPropertyManager *vm, QtProperty *property, const QVariant &value);

    static bool readFamilyMapping(NameMap *rc, QString *errorMessage);

private:
    using PropertyToPropertyMap = QMap<QtProperty *, QtProperty *>;
    using PropertyList = QList<QtProperty *>;
    using PropertyToSubPropertiesMap = QMap<QtProperty *, PropertyList>;

    void removeAntialiasingProperty(QtProperty *);
    void updateModifiedState(QtProperty *property, const QVariant &value);
    static int antialiasingToIndex(QFont::StyleStrategy antialias);
    static QFont::StyleStrategy indexToAntialiasing(int idx);
    static unsigned fontFlag(int idx);

    PropertyToPropertyMap m_propertyToAntialiasing;
    PropertyToPropertyMap m_antialiasingToProperty;

    PropertyToSubPropertiesMap m_propertyToFontSubProperties;
    QMap<QtProperty *, int> m_fontSubPropertyToFlag;
    PropertyToPropertyMap m_fontSubPropertyToProperty;
    QtProperty *m_createdFontProperty = nullptr;
    QStringList m_aliasingEnumNames;
    // Font families with Designer annotations
    QStringList m_designerFamilyNames;
    NameMap m_familyMappings;
};

}

QT_END_NAMESPACE

#endif // FONTPROPERTYMANAGER_H
