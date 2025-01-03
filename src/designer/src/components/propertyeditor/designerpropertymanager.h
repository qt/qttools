// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef DESIGNERPROPERTYMANAGER_H
#define DESIGNERPROPERTYMANAGER_H

#include "qtvariantproperty.h"
#include "brushpropertymanager.h"
#include "fontpropertymanager.h"

#include <qdesigner_utils_p.h>
#include <shared_enums_p.h>

#include <QtCore/qurl.h>
#include <QtCore/qhash.h>
#include <QtGui/qfont.h>
#include <QtGui/qicon.h>

QT_BEGIN_NAMESPACE

using DesignerIntPair = QPair<QString, uint>;
using DesignerFlagList = QList<DesignerIntPair>;

class QDesignerFormEditorInterface;
class QLineEdit;
class QUrl;
class QKeySequenceEdit;

namespace qdesigner_internal
{

class ResetWidget;

class TextEditor;
class PaletteEditorButton;
class PixmapEditor;
class StringListEditorButton;
class FormWindowBase;

class ResetDecorator : public QObject
{
    Q_OBJECT
public:
    explicit ResetDecorator(const QDesignerFormEditorInterface *core, QObject *parent = nullptr);
    ~ResetDecorator();

    void connectPropertyManager(QtAbstractPropertyManager *manager);
    QWidget *editor(QWidget *subEditor, bool resettable, QtAbstractPropertyManager *manager, QtProperty *property,
                QWidget *parent);
    void disconnectPropertyManager(QtAbstractPropertyManager *manager);
    void setSpacing(int spacing);
signals:
    void resetProperty(QtProperty *property);
private slots:
    void slotPropertyChanged(QtProperty *property);
    void slotEditorDestroyed(QObject *object);
private:
    QHash<QtProperty *, QList<ResetWidget *>> m_createdResetWidgets;
    QHash<ResetWidget *, QtProperty *> m_resetWidgetToProperty;
    int m_spacing;
    const QDesignerFormEditorInterface *m_core;
};

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
    QHash<QtProperty *, PropertySheetValue> m_values;
    QHash<QtProperty *, QtProperty *> m_valueToComment;
    QHash<QtProperty *, QtProperty *> m_valueToTranslatable;
    QHash<QtProperty *, QtProperty *> m_valueToDisambiguation;
    QHash<QtProperty *, QtProperty *> m_valueToId;

    QHash<QtProperty *, QtProperty *> m_commentToValue;
    QHash<QtProperty *, QtProperty *> m_translatableToValue;
    QHash<QtProperty *, QtProperty *> m_disambiguationToValue;
    QHash<QtProperty *, QtProperty *> m_idToValue;
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

    QHash<QtProperty *, bool> m_resetMap;

    struct FlagData
    {
        uint val{0};
        DesignerFlagList flags;
        QList<uint> values;
    };

    QHash<QtProperty *, FlagData> m_flagValues;
    QHash<QtProperty *, QList<QtProperty *>> m_propertyToFlags;
    QHash<QtProperty *, QtProperty *> m_flagToProperty;

    int alignToIndexH(uint align) const;
    int alignToIndexV(uint align) const;
    uint indexHToAlign(int idx) const;
    uint indexVToAlign(int idx) const;
    QString indexHToString(int idx) const;
    QString indexVToString(int idx) const;
    QHash<QtProperty *, uint> m_alignValues;
    using PropertyToPropertyMap = QHash<QtProperty *, QtProperty *>;
    PropertyToPropertyMap m_propertyToAlignH;
    PropertyToPropertyMap m_propertyToAlignV;
    PropertyToPropertyMap m_alignHToProperty;
    PropertyToPropertyMap m_alignVToProperty;
    QHash<const QtProperty *, Qt::Alignment> m_alignDefault;

    QHash<QtProperty *, QMap<QPair<QIcon::Mode, QIcon::State>, QtProperty *>> m_propertyToIconSubProperties;
    QHash<QtProperty *, QPair<QIcon::Mode, QIcon::State>> m_iconSubPropertyToState;
    PropertyToPropertyMap m_iconSubPropertyToProperty;
    PropertyToPropertyMap m_propertyToTheme;

    TranslatablePropertyManager<PropertySheetStringValue> m_stringManager;
    TranslatablePropertyManager<PropertySheetKeySequenceValue> m_keySequenceManager;
    TranslatablePropertyManager<PropertySheetStringListValue> m_stringListManager;

    struct PaletteData
    {
        QPalette val;
        QPalette superPalette;
    };
    QHash<QtProperty *, PaletteData> m_paletteValues;

    QHash<QtProperty *, qdesigner_internal::PropertySheetPixmapValue> m_pixmapValues;
    QHash<QtProperty *, qdesigner_internal::PropertySheetIconValue> m_iconValues;

    QHash<QtProperty *, uint> m_uintValues;
    QHash<QtProperty *, qlonglong> m_longLongValues;
    QHash<QtProperty *, qulonglong> m_uLongLongValues;
    QHash<QtProperty *, QUrl> m_urlValues;
    QHash<QtProperty *, QByteArray> m_byteArrayValues;

    QHash<QtProperty *, int> m_stringAttributes;
    QHash<QtProperty *, QFont> m_stringFontAttributes;
    QHash<QtProperty *, bool> m_stringThemeAttributes;

    BrushPropertyManager m_brushManager;
    FontPropertyManager m_fontManager;

    QHash<QtProperty *, QPixmap> m_defaultPixmaps;
    QHash<QtProperty *, QIcon> m_defaultIcons;

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
    void slotUintChanged(const QString &value);
    void slotLongLongChanged(const QString &value);
    void slotULongLongChanged(const QString &value);
    void slotUrlChanged(const QString &value);
    void slotByteArrayChanged(const QString &value);
    void slotStringListChanged(const QStringList &value);
private:
    TextEditor *createTextEditor(QWidget *parent, TextPropertyValidationMode vm, const QString &value);

    ResetDecorator *m_resetDecorator;
    bool m_changingPropertyValue;
    QDesignerFormEditorInterface *m_core;
    FormWindowBase *m_fwb;

    int m_spacing;

    QHash<QtProperty *, QList<TextEditor *>>             m_stringPropertyToEditors;
    QHash<TextEditor *, QtProperty *>                    m_editorToStringProperty;
    QHash<QtProperty *, QList<QKeySequenceEdit *>>       m_keySequencePropertyToEditors;
    QHash<QKeySequenceEdit *, QtProperty *>              m_editorToKeySequenceProperty;
    QHash<QtProperty *, QList<PaletteEditorButton *>>    m_palettePropertyToEditors;
    QHash<PaletteEditorButton *, QtProperty *>           m_editorToPaletteProperty;
    QHash<QtProperty *, QList<PixmapEditor *>>           m_pixmapPropertyToEditors;
    QHash<PixmapEditor *, QtProperty *>                  m_editorToPixmapProperty;
    QHash<QtProperty *, QList<PixmapEditor *>>           m_iconPropertyToEditors;
    QHash<PixmapEditor *, QtProperty *>                  m_editorToIconProperty;
    QHash<QtProperty *, QList<QLineEdit *>>              m_uintPropertyToEditors;
    QHash<QLineEdit *, QtProperty *>                     m_editorToUintProperty;
    QHash<QtProperty *, QList<QLineEdit *>>              m_longLongPropertyToEditors;
    QHash<QLineEdit *, QtProperty *>                     m_editorToLongLongProperty;
    QHash<QtProperty *, QList<QLineEdit *>>              m_uLongLongPropertyToEditors;
    QHash<QLineEdit *, QtProperty *>                     m_editorToULongLongProperty;
    QHash<QtProperty *, QList<TextEditor *>>             m_urlPropertyToEditors;
    QHash<TextEditor *, QtProperty *>                    m_editorToUrlProperty;
    QHash<QtProperty *, QList<TextEditor *>>             m_byteArrayPropertyToEditors;
    QHash<TextEditor *, QtProperty *>                    m_editorToByteArrayProperty;
    QHash<QtProperty *, QList<StringListEditorButton *>> m_stringListPropertyToEditors;
    QHash<StringListEditorButton *, QtProperty *>        m_editorToStringListProperty;
};

} // namespace qdesigner_internal

QT_END_NAMESPACE

Q_DECLARE_METATYPE(DesignerIntPair)
Q_DECLARE_METATYPE(DesignerFlagList)

#endif

