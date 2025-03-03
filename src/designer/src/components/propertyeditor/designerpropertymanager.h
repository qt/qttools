// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef DESIGNERPROPERTYMANAGER_H
#define DESIGNERPROPERTYMANAGER_H

#include "qtvariantproperty_p.h"
#include "brushpropertymanager.h"
#include "fontpropertymanager.h"

#include <qdesigner_utils_p.h>
#include <shared_enums_p.h>

#include <QtCore/qurl.h>
#include <QtCore/qhash.h>
#include <QtGui/qfont.h>
#include <QtGui/qicon.h>

QT_BEGIN_NAMESPACE

using DesignerIntPair = std::pair<QString, uint>;
using DesignerFlagList = QList<DesignerIntPair>;

class QComboBox;
class QDesignerFormEditorInterface;
class QLineEdit;
class QUrl;
class QKeySequenceEdit;

namespace qdesigner_internal
{

class TextEditor;
class PaletteEditorButton;
class PixmapEditor;
class ResetDecorator;
class StringListEditorButton;
class FormWindowBase;

// Helper for handling sub-properties of properties inheriting PropertySheetTranslatableData
// (translatable, disambiguation, comment).
template <class PropertySheetValue>
class TranslatablePropertyManager
{
public:
    void initialize(QtVariantPropertyManager *m, QtProperty *property, const PropertySheetValue &value);
    bool uninitialize(QtProperty *property);
    bool destroy(QtProperty *subProperty);

    bool value(const QtProperty *property, QVariant *rc) const;
    int valueChanged(QtVariantPropertyManager *m, QtProperty *property,
                                    const QVariant &value);

    int setValue(QtVariantPropertyManager *m, QtProperty *property,
                 int expectedTypeId, const QVariant &value);

private:
    QHash<const QtProperty *, PropertySheetValue> m_values;
    QHash<const QtProperty *, QtProperty *> m_valueToComment;
    QHash<const QtProperty *, QtProperty *> m_valueToTranslatable;
    QHash<const QtProperty *, QtProperty *> m_valueToDisambiguation;
    QHash<const QtProperty *, QtProperty *> m_valueToId;

    QHash<const QtProperty *, QtProperty *> m_commentToValue;
    QHash<const QtProperty *, QtProperty *> m_translatableToValue;
    QHash<const QtProperty *, QtProperty *> m_disambiguationToValue;
    QHash<const QtProperty *, QtProperty *> m_idToValue;
};

class DesignerPropertyManager : public QtVariantPropertyManager
{
    Q_OBJECT
public:
    enum ValueChangedResult { NoMatch, Unchanged, Changed };

    explicit DesignerPropertyManager(QDesignerFormEditorInterface *core, QObject *parent = nullptr);
    ~DesignerPropertyManager();

    QStringList attributes(int propertyType) const override;
    int attributeType(int propertyType, const QString &attribute) const override;

    QVariant attributeValue(const QtProperty *property, const QString &attribute) const override;
    bool isPropertyTypeSupported(int propertyType) const override;
    QVariant value(const QtProperty *property) const override;
    int valueType(int propertyType) const override;
    QString valueText(const QtProperty *property) const override;
    QIcon valueIcon(const QtProperty *property) const override;

    bool resetTextAlignmentProperty(QtProperty *property);
    bool resetFontSubProperty(QtProperty *property);
    bool resetIconSubProperty(QtProperty *subProperty);

    void reloadResourceProperties();

    static int designerFlagTypeId();
    static int designerFlagListTypeId();
    static int designerAlignmentTypeId();
    static int designerPixmapTypeId();
    static int designerIconTypeId();
    static int designerStringTypeId();
    static int designerStringListTypeId();
    static int designerKeySequenceTypeId();

    void setObject(QObject *object) { m_object = object; }

    static void setUseIdBasedTranslations(bool v)
        { m_IdBasedTranslations = v; }
    static bool useIdBasedTranslations()
        { return m_IdBasedTranslations; }

    static QString alignDefaultAttribute();

    static uint alignDefault(const QtVariantProperty *prop);

public Q_SLOTS:
    void setAttribute(QtProperty *property, const QString &attribute, const QVariant &value) override;
    void setValue(QtProperty *property, const QVariant &value) override;
Q_SIGNALS:
    // sourceOfChange - a subproperty (or just property) which caused a change
    //void valueChanged(QtProperty *property, const QVariant &value, QtProperty *sourceOfChange);
    void valueChanged(QtProperty *property, const QVariant &value, bool enableSubPropertyHandling);
protected:
    void initializeProperty(QtProperty *property) override;
    void uninitializeProperty(QtProperty *property) override;
private Q_SLOTS:
    void slotValueChanged(QtProperty *property, const QVariant &value);
    void slotPropertyDestroyed(QtProperty *property);
private:
    void createIconSubProperty(QtProperty *iconProperty, QIcon::Mode mode, QIcon::State state, const QString &subName);

    QHash<const QtProperty *, bool> m_resetMap;

    struct FlagData
    {
        uint val{0};
        DesignerFlagList flags;
        QList<uint> values;
    };

    QHash<const QtProperty *, FlagData> m_flagValues;
    QHash<const QtProperty *, QList<QtProperty *>> m_propertyToFlags;

    int alignToIndexH(uint align) const;
    int alignToIndexV(uint align) const;
    uint indexHToAlign(int idx) const;
    uint indexVToAlign(int idx) const;
    QString indexHToString(int idx) const;
    QString indexVToString(int idx) const;
    QHash<const QtProperty *, uint> m_alignValues;
    using PropertyToPropertyMap = QHash<const QtProperty *, QtProperty *>;
    PropertyToPropertyMap m_propertyToAlignH;
    PropertyToPropertyMap m_propertyToAlignV;
    PropertyToPropertyMap m_alignHToProperty;
    PropertyToPropertyMap m_alignVToProperty;
    QHash<const QtProperty *, Qt::Alignment> m_alignDefault;

    QHash<const QtProperty *, QMap<std::pair<QIcon::Mode, QIcon::State>, QtProperty *>> m_propertyToIconSubProperties;
    QHash<const QtProperty *, std::pair<QIcon::Mode, QIcon::State>> m_iconSubPropertyToState;
    PropertyToPropertyMap m_propertyToTheme;
    PropertyToPropertyMap m_propertyToThemeEnum;

    TranslatablePropertyManager<PropertySheetStringValue> m_stringManager;
    TranslatablePropertyManager<PropertySheetKeySequenceValue> m_keySequenceManager;
    TranslatablePropertyManager<PropertySheetStringListValue> m_stringListManager;

    struct PaletteData
    {
        QPalette val;
        QPalette superPalette;
    };
    QHash<const QtProperty *, PaletteData> m_paletteValues;

    QHash<const QtProperty *, qdesigner_internal::PropertySheetPixmapValue> m_pixmapValues;
    QHash<const QtProperty *, qdesigner_internal::PropertySheetIconValue> m_iconValues;

    QHash<const QtProperty *, int> m_intValues;
    QHash<const QtProperty *, uint> m_uintValues;
    QHash<const QtProperty *, qlonglong> m_longLongValues;
    QHash<const QtProperty *, qulonglong> m_uLongLongValues;
    QHash<const QtProperty *, QUrl> m_urlValues;
    QHash<const QtProperty *, QByteArray> m_byteArrayValues;

    QHash<const QtProperty *, int> m_stringAttributes;
    QHash<const QtProperty *, QFont> m_stringFontAttributes;
    QHash<const QtProperty *, bool> m_stringThemeAttributes;
    QHash<const QtProperty *, bool> m_intThemeEnumAttributes;

    BrushPropertyManager m_brushManager;
    FontPropertyManager m_fontManager;

    QHash<const QtProperty *, QPixmap> m_defaultPixmaps;
    QHash<const QtProperty *, QIcon> m_defaultIcons;

    bool m_changingSubValue;
    QDesignerFormEditorInterface *m_core;

    QObject *m_object;

    QtProperty *m_sourceOfChange;
    static bool m_IdBasedTranslations;
};

class DesignerEditorFactory : public QtVariantEditorFactory
{
    Q_OBJECT
public:
    explicit DesignerEditorFactory(QDesignerFormEditorInterface *core, QObject *parent = nullptr);
    ~DesignerEditorFactory();
    void setSpacing(int spacing);
    void setFormWindowBase(FormWindowBase *fwb);
signals:
    void resetProperty(QtProperty *property);
protected:
    void connectPropertyManager(QtVariantPropertyManager *manager) override;
    QWidget *createEditor(QtVariantPropertyManager *manager, QtProperty *property,
                QWidget *parent) override;
    void disconnectPropertyManager(QtVariantPropertyManager *manager) override;
private slots:
    void slotEditorDestroyed(QObject *object);
    void slotAttributeChanged(QtProperty *property, const QString &attribute, const QVariant &value);
    void slotPropertyChanged(QtProperty *property);
    void slotValueChanged(QtProperty *property, const QVariant &value);
    void slotStringTextChanged(const QString &value);
    void slotKeySequenceChanged(const QKeySequence &value);
    void slotPaletteChanged(const QPalette &value);
    void slotPixmapChanged(const QString &value);
    void slotIconChanged(const QString &value);
    void slotIconThemeChanged(const QString &value);
    void slotIconThemeEnumChanged(int value);
    void slotUintChanged(const QString &value);
    void slotIntChanged(int);
    void slotLongLongChanged(const QString &value);
    void slotULongLongChanged(const QString &value);
    void slotUrlChanged(const QString &value);
    void slotByteArrayChanged(const QString &value);
    void slotStringListChanged(const QStringList &value);
private:
    TextEditor *createTextEditor(QWidget *parent, TextPropertyValidationMode vm, const QString &value);

    ResetDecorator *m_resetDecorator;
    bool m_changingPropertyValue = false;
    QDesignerFormEditorInterface *m_core;
    FormWindowBase *m_fwb = nullptr;

    int m_spacing = -1;

    QHash<const QtProperty *, QList<TextEditor *>>             m_stringPropertyToEditors;
    QHash<TextEditor *, QtProperty *>                          m_editorToStringProperty;
    QHash<const QtProperty *, QList<QKeySequenceEdit *>>       m_keySequencePropertyToEditors;
    QHash<QKeySequenceEdit *, QtProperty *>                    m_editorToKeySequenceProperty;
    QHash<const QtProperty *, QList<PaletteEditorButton *>>    m_palettePropertyToEditors;
    QHash<PaletteEditorButton *, QtProperty *>                 m_editorToPaletteProperty;
    QHash<const QtProperty *, QList<PixmapEditor *>>           m_pixmapPropertyToEditors;
    QHash<PixmapEditor *, QtProperty *>                        m_editorToPixmapProperty;
    QHash<const QtProperty *, QList<PixmapEditor *>>           m_iconPropertyToEditors;
    QHash<PixmapEditor *, QtProperty *>                        m_editorToIconProperty;
    QHash<const QtProperty *, QList<QComboBox *>>              m_intPropertyToComboEditors;
    QHash<QComboBox *, QtProperty *>                           m_comboEditorToIntProperty;
    QHash<const QtProperty *, QList<QLineEdit *>>              m_uintPropertyToEditors;
    QHash<QLineEdit *, QtProperty *>                           m_editorToUintProperty;
    QHash<const QtProperty *, QList<QLineEdit *>>              m_longLongPropertyToEditors;
    QHash<QLineEdit *, QtProperty *>                           m_editorToLongLongProperty;
    QHash<const QtProperty *, QList<QLineEdit *>>              m_uLongLongPropertyToEditors;
    QHash<QLineEdit *, QtProperty *>                           m_editorToULongLongProperty;
    QHash<const QtProperty *, QList<TextEditor *>>             m_urlPropertyToEditors;
    QHash<TextEditor *, QtProperty *>                          m_editorToUrlProperty;
    QHash<const QtProperty *, QList<TextEditor *>>             m_byteArrayPropertyToEditors;
    QHash<TextEditor *, QtProperty *>                          m_editorToByteArrayProperty;
    QHash<const QtProperty *, QList<StringListEditorButton *>> m_stringListPropertyToEditors;
    QHash<StringListEditorButton *, QtProperty *>              m_editorToStringListProperty;
};

} // namespace qdesigner_internal

QT_END_NAMESPACE

Q_DECLARE_METATYPE(DesignerIntPair)
Q_DECLARE_METATYPE(DesignerFlagList)

#endif

