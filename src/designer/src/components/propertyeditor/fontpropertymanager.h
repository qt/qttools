// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef FONTPROPERTYMANAGER_H
#define FONTPROPERTYMANAGER_H

#include <QtCore/qhash.h>
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

    using ResetMap = QHash<const QtProperty *, bool>;
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
    using PropertyToPropertyMap = QHash<QtProperty *, QtProperty *>;
    using PropertyList = QList<QtProperty *>;

    void removeAntialiasingProperty(QtProperty *);
    void removeHintingPreferenceProperty(QtProperty *);
    int antialiasingValueChanged(QtVariantPropertyManager *vm,
                                 QtProperty *antialiasingProperty, const QVariant &value);
    int hintingPreferenceValueChanged(QtVariantPropertyManager *vm,
                                      QtProperty *hintingPreferenceProperty,
                                      const QVariant &value);
    void updateModifiedState(QtProperty *property, const QVariant &value);
    static int antialiasingToIndex(QFont::StyleStrategy antialias);
    static QFont::StyleStrategy indexToAntialiasing(int idx);
    static int hintingPreferenceToIndex(QFont::HintingPreference h);
    static QFont::HintingPreference indexToHintingPreference(int idx);

    static unsigned fontFlag(int idx);

    PropertyToPropertyMap m_propertyToAntialiasing;
    PropertyToPropertyMap m_antialiasingToProperty;
    PropertyToPropertyMap m_propertyToHintingPreference;
    PropertyToPropertyMap m_hintingPreferenceToProperty;


    QHash<QtProperty *, PropertyList> m_propertyToFontSubProperties;
    QHash<QtProperty *, int> m_fontSubPropertyToFlag;
    QtProperty *m_createdFontProperty = nullptr;
    QStringList m_aliasingEnumNames;
    QStringList m_hintingPreferenceEnumNames;
    // Font families with Designer annotations
    QStringList m_designerFamilyNames;
    NameMap m_familyMappings;
};

}

QT_END_NAMESPACE

#endif // FONTPROPERTYMANAGER_H
