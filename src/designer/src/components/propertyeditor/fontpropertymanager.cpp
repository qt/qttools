// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "fontpropertymanager.h"
#include "qtpropertymanager.h"
#include "designerpropertymanager.h"
#include "qtpropertybrowserutils_p.h"

#include <qdesigner_utils_p.h>

#include <QtCore/qcoreapplication.h>
#include <QtCore/qvariant.h>
#include <QtCore/qstring.h>
#include <QtCore/qdebug.h>
#include <QtCore/qfile.h>
#include <QtCore/qtextstream.h>
#include <QtCore/qxmlstream.h>

#include <utility>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

namespace qdesigner_internal {

    using DisambiguatedTranslation = std::pair<const char *, const char *>;

    static const char *aliasingC[] = {
        QT_TRANSLATE_NOOP("FontPropertyManager", "PreferDefault"),
        QT_TRANSLATE_NOOP("FontPropertyManager", "NoAntialias"),
        QT_TRANSLATE_NOOP("FontPropertyManager", "PreferAntialias")
    };

    static const DisambiguatedTranslation hintingPreferenceC[] = {
        QT_TRANSLATE_NOOP3("FontPropertyManager", "PreferDefaultHinting", "QFont::StyleStrategy combo"),
        QT_TRANSLATE_NOOP3("FontPropertyManager", "PreferNoHinting", "QFont::StyleStrategy combo"),
        QT_TRANSLATE_NOOP3("FontPropertyManager", "PreferVerticalHinting", "QFont::StyleStrategy combo"),
        QT_TRANSLATE_NOOP3("FontPropertyManager", "PreferFullHinting", "QFont::StyleStrategy combo")
    };

    FontPropertyManager::FontPropertyManager()
    {
        for (const auto *a : aliasingC)
            m_aliasingEnumNames.append(QCoreApplication::translate("FontPropertyManager", a));

        for (const auto &h : hintingPreferenceC)
            m_hintingPreferenceEnumNames.append(QCoreApplication::translate("FontPropertyManager", h.first, h.second));

        QString errorMessage;
        if (!readFamilyMapping(&m_familyMappings, &errorMessage)) {
            designerWarning(errorMessage);
        }

    }

    void FontPropertyManager::preInitializeProperty(QtProperty *property,
                                                    int type,
                                                    ResetMap &resetMap)
    {
        if (m_createdFontProperty) {
            auto it = m_propertyToFontSubProperties.find(m_createdFontProperty);
            if (it == m_propertyToFontSubProperties.end())
                it = m_propertyToFontSubProperties.insert(m_createdFontProperty, PropertyList());
            const int index = it.value().size();
            m_fontSubPropertyToFlag.insert(property, index);
            it.value().push_back(property);
            m_fontSubPropertyToProperty[property] = m_createdFontProperty;
            resetMap[property] = true;
        }

        if (type == QMetaType::QFont)
            m_createdFontProperty = property;
    }

    // Map the font family names to display names retrieved from the XML configuration
    static QStringList designerFamilyNames(QStringList families, const FontPropertyManager::NameMap &nm)
    {
        if (nm.isEmpty())
            return families;

        const auto ncend = nm.constEnd();
        for (auto it = families.begin(), end = families.end(); it != end; ++it) {
            const auto nit = nm.constFind(*it);
            if (nit != ncend)
                *it = nit.value();
        }
        return families;
    }

    void FontPropertyManager::postInitializeProperty(QtVariantPropertyManager *vm,
                                                     QtProperty *property,
                                                     int type,
                                                     int enumTypeId)
    {
        if (type != QMetaType::QFont)
            return;

        // This will cause a recursion
        QtVariantProperty *antialiasing = vm->addProperty(enumTypeId, QCoreApplication::translate("FontPropertyManager", "Antialiasing"));
        const QFont font = qvariant_cast<QFont>(vm->variantProperty(property)->value());

        antialiasing->setAttribute(u"enumNames"_s, m_aliasingEnumNames);
        antialiasing->setValue(antialiasingToIndex(font.styleStrategy()));
        property->addSubProperty(antialiasing);

        m_propertyToAntialiasing[property] = antialiasing;
        m_antialiasingToProperty[antialiasing] = property;

        QtVariantProperty *hintingPreference = vm->addProperty(enumTypeId, QCoreApplication::translate("FontPropertyManager", "HintingPreference"));
        hintingPreference->setAttribute(u"enumNames"_s, m_hintingPreferenceEnumNames);
        hintingPreference->setValue(hintingPreferenceToIndex(font.hintingPreference()));
        property->addSubProperty(hintingPreference);

        m_propertyToHintingPreference[property] = hintingPreference;
        m_hintingPreferenceToProperty[hintingPreference] = property;

        // Fiddle family names
        if (!m_familyMappings.isEmpty()) {
            const auto it = m_propertyToFontSubProperties.find(m_createdFontProperty);
            QtVariantProperty *familyProperty = vm->variantProperty(it.value().constFirst());
            const QString enumNamesAttribute = u"enumNames"_s;
            QStringList plainFamilyNames = familyProperty->attributeValue(enumNamesAttribute).toStringList();
            // Did someone load fonts or something?
            if (m_designerFamilyNames.size() != plainFamilyNames.size())
                m_designerFamilyNames = designerFamilyNames(plainFamilyNames, m_familyMappings);
            familyProperty->setAttribute(enumNamesAttribute, m_designerFamilyNames);
        }
        // Next
        m_createdFontProperty = nullptr;
    }

    bool FontPropertyManager::uninitializeProperty(QtProperty *property)
    {
        const auto ait =  m_propertyToAntialiasing.find(property);
        if (ait != m_propertyToAntialiasing.end()) {
            QtProperty *antialiasing = ait.value();
            m_antialiasingToProperty.remove(antialiasing);
            m_propertyToAntialiasing.erase(ait);
            delete antialiasing;
        }

        const auto hit =  m_propertyToHintingPreference.find(property);
        if (hit != m_propertyToHintingPreference.end()) {
            QtProperty *hintingPreference = hit.value();
            m_hintingPreferenceToProperty.remove(hintingPreference);
            m_propertyToHintingPreference.erase(hit);
            delete hintingPreference;
        }

        const auto sit = m_propertyToFontSubProperties.find(property);
        if (sit == m_propertyToFontSubProperties.end())
            return false;

        m_propertyToFontSubProperties.erase(sit);
        m_fontSubPropertyToFlag.remove(property);
        m_fontSubPropertyToProperty.remove(property);

        return true;
    }

    void FontPropertyManager::slotPropertyDestroyed(QtProperty *property)
    {
        removeAntialiasingProperty(property);
        removeHintingPreferenceProperty(property);
    }

    void FontPropertyManager::removeAntialiasingProperty(QtProperty *property)
    {
        const auto ait =  m_antialiasingToProperty.find(property);
        if (ait == m_antialiasingToProperty.end())
            return;
        m_propertyToAntialiasing[ait.value()] = 0;
        m_antialiasingToProperty.erase(ait);
    }

    void FontPropertyManager::removeHintingPreferenceProperty(QtProperty *property)
    {
        const auto hit =  m_hintingPreferenceToProperty.find(property);
        if (hit == m_hintingPreferenceToProperty.end())
            return;
        m_propertyToHintingPreference[hit.value()] = nullptr;
        m_hintingPreferenceToProperty.erase(hit);
    }

    bool FontPropertyManager::resetFontSubProperty(QtVariantPropertyManager *vm, QtProperty *property)
    {
        const auto it = m_fontSubPropertyToProperty.find(property);
        if (it == m_fontSubPropertyToProperty.end())
            return false;

        QtVariantProperty *fontProperty = vm->variantProperty(it.value());

        QVariant v = fontProperty->value();
        QFont font = qvariant_cast<QFont>(v);
        unsigned mask = font.resolveMask();
        const unsigned flag = fontFlag(m_fontSubPropertyToFlag.value(property));

        mask &= ~flag;
        font.setResolveMask(mask);
        v.setValue(font);
        fontProperty->setValue(v);
        return true;
    }

    int FontPropertyManager::antialiasingToIndex(QFont::StyleStrategy antialias)
    {
        switch (antialias) {
        case QFont::PreferDefault:   return 0;
        case QFont::NoAntialias:     return 1;
        case QFont::PreferAntialias: return 2;
        default: break;
        }
        return 0;
    }

    QFont::StyleStrategy FontPropertyManager::indexToAntialiasing(int idx)
    {
        switch (idx) {
        case 0: return QFont::PreferDefault;
        case 1: return QFont::NoAntialias;
        case 2: return QFont::PreferAntialias;
        }
        return QFont::PreferDefault;
    }

    int FontPropertyManager::hintingPreferenceToIndex(QFont::HintingPreference h)
    {
        switch (h) {
        case QFont::PreferDefaultHinting:
            return 0;
        case QFont::PreferNoHinting:
            return 1;
        case QFont::PreferVerticalHinting:
            return 2;
        case QFont::PreferFullHinting:
            return 3;
        }
        return 0;
    }

    QFont::HintingPreference FontPropertyManager::indexToHintingPreference(int idx)
    {
        switch (idx) {
        case 0:
            return QFont::PreferDefaultHinting;
        case 1:
            return QFont::PreferNoHinting;
        case 2:
            return QFont::PreferVerticalHinting;
        case 3:
            return QFont::PreferFullHinting;
        }
        return QFont::PreferDefaultHinting;
    }

    unsigned FontPropertyManager::fontFlag(int idx)
    {
        switch (idx) {
        case 0:
            return QFont::FamilyResolved | QFont::FamiliesResolved;
        case 1:
            return QFont::SizeResolved;
        case 2:
        case 7:
            return QFont::WeightResolved;
        case 3:
            return QFont::StyleResolved;
        case 4:
            return QFont::UnderlineResolved;
        case 5:
            return QFont::StrikeOutResolved;
        case 6:
            return QFont::KerningResolved;
        case 8:
            return QFont::StyleStrategyResolved;
        case 9:
            return QFont::HintingPreferenceResolved;
        }
        return 0;
    }

    int FontPropertyManager::valueChanged(QtVariantPropertyManager *vm, QtProperty *property, const QVariant &value)
    {
        if (auto *antialiasingProperty = m_antialiasingToProperty.value(property, nullptr))
            return antialiasingValueChanged(vm, antialiasingProperty, value);

        if (auto *hintingPreferenceProperty = m_hintingPreferenceToProperty.value(property, nullptr))
            return hintingPreferenceValueChanged(vm, hintingPreferenceProperty, value);

        if (m_propertyToFontSubProperties.contains(property))
            updateModifiedState(property, value);

        return DesignerPropertyManager::NoMatch;
    }

    int FontPropertyManager::antialiasingValueChanged(QtVariantPropertyManager *vm,
                                                      QtProperty *antialiasingProperty,
                                                      const QVariant &value)
    {
        QtVariantProperty *fontProperty = vm->variantProperty(antialiasingProperty);
        const QFont::StyleStrategy newValue = indexToAntialiasing(value.toInt());

        QFont font = qvariant_cast<QFont>(fontProperty->value());
        const QFont::StyleStrategy oldValue = font.styleStrategy();
        if (newValue == oldValue)
            return DesignerPropertyManager::Unchanged;

        font.setStyleStrategy(newValue);
        fontProperty->setValue(QVariant::fromValue(font));
        return DesignerPropertyManager::Changed;
    }

    int FontPropertyManager::hintingPreferenceValueChanged(QtVariantPropertyManager *vm,
                                                           QtProperty *hintingPreferenceProperty,
                                                           const QVariant &value)
    {
        QtVariantProperty *fontProperty = vm->variantProperty(hintingPreferenceProperty);
        const QFont::HintingPreference newValue = indexToHintingPreference(value.toInt());

        QFont font = qvariant_cast<QFont>(fontProperty->value());
        const QFont::HintingPreference oldValue = font.hintingPreference();
        if (newValue == oldValue)
            return DesignerPropertyManager::Unchanged;

        font.setHintingPreference(newValue);
        fontProperty->setValue(QVariant::fromValue(font));
        return DesignerPropertyManager::Changed;
    }

    void FontPropertyManager::updateModifiedState(QtProperty *property, const QVariant &value)
    {
        const auto it = m_propertyToFontSubProperties.find(property);
        if (it == m_propertyToFontSubProperties.end())
            return;

        const PropertyList &subProperties = it.value();

        QFont font = qvariant_cast<QFont>(value);
        const unsigned mask = font.resolveMask();

        const int count = subProperties.size();
        for (int index = 0; index < count; index++) {
             const unsigned flag = fontFlag(index);
             subProperties.at(index)->setModified(mask & flag);
        }
    }

    void FontPropertyManager::setValue(QtVariantPropertyManager *vm, QtProperty *property, const QVariant &value)
    {
        updateModifiedState(property, value);

        if (QtProperty *antialiasingProperty = m_propertyToAntialiasing.value(property, 0)) {
            QtVariantProperty *antialiasing = vm->variantProperty(antialiasingProperty);
            if (antialiasing) {
                QFont font = qvariant_cast<QFont>(value);
                antialiasing->setValue(antialiasingToIndex(font.styleStrategy()));
            }
        }

        if (QtProperty *hintingPreferenceProperty = m_propertyToHintingPreference.value(property, nullptr)) {
            if (auto *hintingPreference = vm->variantProperty(hintingPreferenceProperty)) {
                QFont font = qvariant_cast<QFont>(value);
                hintingPreference->setValue(hintingPreferenceToIndex(font.hintingPreference()));
            }
        }

    }

    /* Parse a mappings file of the form:
     * <fontmappings>
     * <mapping><family>DejaVu Sans</family><display>DejaVu Sans [CE]</display></mapping>
     * ... which is used to display on which platforms fonts are available.*/

    static const char rootTagC[] = "fontmappings";
    static const char mappingTagC[] = "mapping";
    static const char familyTagC[] = "family";
    static const char displayTagC[] = "display";

    static QString msgXmlError(const QXmlStreamReader &r, const QString& fileName)
    {
        return u"An error has been encountered at line %1 of %2: %3:"_s.arg(r.lineNumber()).arg(fileName, r.errorString());
    }

    /* Switch stages when encountering a start element (state table) */
    enum ParseStage { ParseBeginning, ParseWithinRoot, ParseWithinMapping, ParseWithinFamily,
                      ParseWithinDisplay, ParseError };

    static ParseStage nextStage(ParseStage currentStage, QStringView startElement)
    {
        switch (currentStage) {
        case ParseBeginning:
            return startElement == QLatin1StringView(rootTagC) ? ParseWithinRoot : ParseError;
        case ParseWithinRoot:
        case ParseWithinDisplay: // Next mapping, was in <display>
            return startElement == QLatin1StringView(mappingTagC) ? ParseWithinMapping : ParseError;
        case ParseWithinMapping:
            return startElement == QLatin1StringView(familyTagC) ? ParseWithinFamily : ParseError;
        case ParseWithinFamily:
            return startElement == QLatin1StringView(displayTagC) ? ParseWithinDisplay : ParseError;
        case ParseError:
            break;
        }
        return  ParseError;
    }

    bool FontPropertyManager::readFamilyMapping(NameMap *rc, QString *errorMessage)
    {
        rc->clear();
        const QString fileName = u":/qt-project.org/propertyeditor/fontmapping.xml"_s;
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            *errorMessage = "Unable to open %1: %2"_L1.arg(fileName, file.errorString());
            return false;
        }

        QXmlStreamReader reader(&file);
        QXmlStreamReader::TokenType token;

        QString family;
        ParseStage stage = ParseBeginning;
        do {
            token = reader.readNext();
            switch (token) {
            case QXmlStreamReader::Invalid:
                *errorMessage = msgXmlError(reader, fileName);
                 return false;
            case QXmlStreamReader::StartElement:
                stage = nextStage(stage, reader.name());
                switch (stage) {
                case ParseError:
                    reader.raiseError("Unexpected element <%1>."_L1.arg(reader.name()));
                    *errorMessage = msgXmlError(reader, fileName);
                    return false;
                case ParseWithinFamily:
                    family = reader.readElementText();
                    break;
                case ParseWithinDisplay:
                    rc->insert(family, reader.readElementText());
                    break;
                default:
                    break;
                }
            default:
                break;
            }
        } while (token != QXmlStreamReader::EndDocument);
        return true;
    }

}

QT_END_NAMESPACE
