// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "qdesigner_resource.h"
#include "formwindow.h"
#include "dynamicpropertysheet.h"
#include "qdesigner_tabwidget_p.h"
#include "iconloader_p.h"
#include "qdesigner_toolbox_p.h"
#include "qdesigner_stackedbox_p.h"
#include "qdesigner_toolbar_p.h"
#include "qdesigner_dockwidget_p.h"
#include "qdesigner_menu_p.h"
#include "qdesigner_menubar_p.h"
#include "qdesigner_membersheet_p.h"
#include "qtresourcemodel_p.h"
#include "qmdiarea_container.h"
#include "qwizard_container.h"
#include "layout_propertysheet.h"

#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/abstractintegration.h>
#include <QtDesigner/private/ui4_p.h>
#include <QtDesigner/private/formbuilderextra_p.h>
#include <QtDesigner/private/resourcebuilder_p.h>
#include <QtDesigner/private/textbuilder_p.h>
#include <qdesigner_widgetitem_p.h>

// shared
#include <widgetdatabase_p.h>
#include <metadatabase_p.h>
#include <layout_p.h>
#include <layoutinfo_p.h>
#include <spacer_widget_p.h>
#include <pluginmanager_p.h>
#include <widgetfactory_p.h>
#include <abstractlanguage.h>
#include <abstractintrospection_p.h>

#include <qlayout_widget_p.h>
#include <qdesigner_utils_p.h>
#include <QtDesigner/private/ui4_p.h>

// sdk
#include <QtDesigner/propertysheet.h>
#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/extrainfo.h>
#include <QtDesigner/abstractformwindowtool.h>
#include <QtDesigner/qextensionmanager.h>
#include <QtDesigner/container.h>
#include <abstractdialoggui_p.h>

#include <QtWidgets/qmenu.h>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/qlayout.h>
#include <QtWidgets/qformlayout.h>
#include <QtWidgets/qtabwidget.h>
#include <QtWidgets/qtoolbox.h>
#include <QtWidgets/qstackedwidget.h>
#include <QtWidgets/qtoolbar.h>
#include <QtWidgets/qtabbar.h>
#include <QtWidgets/qbuttongroup.h>
#include <QtWidgets/qapplication.h>
#include <QtWidgets/qmainwindow.h>
#include <QtWidgets/qsplitter.h>
#include <QtWidgets/qmdiarea.h>
#include <QtWidgets/qmenubar.h>
#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qheaderview.h>
#include <QtWidgets/qwizard.h>
#include <private/qlayoutengine_p.h>

#include <QtGui/qaction.h>
#include <QtGui/qactiongroup.h>

#include <QtCore/qbuffer.h>
#include <QtCore/qdir.h>
#include <QtCore/qlibraryinfo.h>
#include <QtCore/qmetaobject.h>
#include <QtCore/qdebug.h>
#include <QtCore/qversionnumber.h>
#include <QtCore/qxmlstream.h>

#include <algorithm>
#include <iterator>

Q_DECLARE_METATYPE(QWidgetList)

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

using QFBE = QFormBuilderExtra;

namespace {
    using DomPropertyList = QList<DomProperty *>;
}

static constexpr auto currentUiVersion = "4.0"_L1;
static constexpr auto clipboardObjectName = "__qt_fake_top_level"_L1;

#define OLD_RESOURCE_FORMAT // Support pre 4.4 format.

namespace qdesigner_internal {

static QVersionNumber qtVersion(const QDesignerFormEditorInterface *core)
{
    const QVariant v = core->integration()->property("qtVersion");
    return v.isValid() && v.canConvert<QVersionNumber>()
           ? v.value<QVersionNumber>() : QLibraryInfo::version();
}

static bool supportsQualifiedEnums(const QVersionNumber &qtVersion)
{
    if (qtVersion >= QVersionNumber{6, 6, 2})
        return true;

    switch (qtVersion.majorVersion()) {
    case 6: // Qt 6
        switch (qtVersion.minorVersion()) {
        case 5: // 6.5 LTS
            if (qtVersion.microVersion() >= 4)
                return true;
            break;
        case 2: // 6.2 LTS
            if (qtVersion.microVersion() >= 13)
                return true;
            break;
        }
        break;

    case 5: // Qt 5 LTS
        if (qtVersion >= QVersionNumber{5, 15, 18})
            return true;
        break;
    }
    return false;
}

// -------------------- QDesignerResourceBuilder: A resource builder that works on the property sheet icon types.
class QDesignerResourceBuilder : public QResourceBuilder
{
public:
    QDesignerResourceBuilder(QDesignerFormEditorInterface *core, DesignerPixmapCache *pixmapCache, DesignerIconCache *iconCache);

    void setPixmapCache(DesignerPixmapCache *pixmapCache) { m_pixmapCache = pixmapCache; }
    void setIconCache(DesignerIconCache *iconCache)       { m_iconCache = iconCache; }
    bool isSaveRelative() const                           { return m_saveRelative; }
    void setSaveRelative(bool relative)                   { m_saveRelative = relative; }
    QStringList usedQrcFiles() const                      { return m_usedQrcFiles.keys(); }
#ifdef OLD_RESOURCE_FORMAT
    QStringList loadedQrcFiles() const                    { return m_loadedQrcFiles.keys(); } // needed only for loading old resource attribute of <iconset> tag.
#endif

    QVariant loadResource(const QDir &workingDirectory, const DomProperty *icon) const override;

    QVariant toNativeValue(const QVariant &value) const override;

    DomProperty *saveResource(const QDir &workingDirectory, const QVariant &value) const override;

    bool isResourceType(const QVariant &value) const override;
private:

    QDesignerFormEditorInterface *m_core;
    DesignerPixmapCache  *m_pixmapCache;
    DesignerIconCache    *m_iconCache;
    const QDesignerLanguageExtension *m_lang;
    bool                  m_saveRelative;
    mutable QMap<QString, bool>   m_usedQrcFiles;
    mutable QMap<QString, bool>   m_loadedQrcFiles;
};

QDesignerResourceBuilder::QDesignerResourceBuilder(QDesignerFormEditorInterface *core, DesignerPixmapCache *pixmapCache, DesignerIconCache *iconCache) :
    m_core(core),
    m_pixmapCache(pixmapCache),
    m_iconCache(iconCache),
    m_lang(qt_extension<QDesignerLanguageExtension *>(core->extensionManager(), core)),
    m_saveRelative(true)
{
}

static inline void setIconPixmap(QIcon::Mode m, QIcon::State s, const QDir &workingDirectory,
                                 QString path, PropertySheetIconValue &icon,
                                 const QDesignerLanguageExtension *lang = nullptr)
{
    if (lang == nullptr || !lang->isLanguageResource(path))
        path = QFileInfo(workingDirectory, path).absoluteFilePath();
    icon.setPixmap(m, s, PropertySheetPixmapValue(path));
}

QVariant QDesignerResourceBuilder::loadResource(const QDir &workingDirectory, const DomProperty *property) const
{
    switch (property->kind()) {
        case DomProperty::Pixmap: {
            PropertySheetPixmapValue pixmap;
            DomResourcePixmap *dp = property->elementPixmap();
            if (!dp->text().isEmpty()) {
                if (m_lang != nullptr && m_lang->isLanguageResource(dp->text())) {
                    pixmap.setPath(dp->text());
                } else {
                    pixmap.setPath(QFileInfo(workingDirectory, dp->text()).absoluteFilePath());
                }
#ifdef OLD_RESOURCE_FORMAT
                if (dp->hasAttributeResource())
                    m_loadedQrcFiles.insert(QFileInfo(workingDirectory, dp->attributeResource()).absoluteFilePath(), false);
#endif
            }
            return QVariant::fromValue(pixmap);
        }

        case DomProperty::IconSet: {
            PropertySheetIconValue icon;
            DomResourceIcon *di = property->elementIconSet();
            const bool hasTheme = di->hasAttributeTheme();
            if (hasTheme) {
                const QString &theme = di->attributeTheme();
                const qsizetype themeEnum = theme.startsWith("QIcon::"_L1)
                    ? QDesignerResourceBuilder::themeIconIndex(theme) : -1;
                if (themeEnum != -1)
                    icon.setThemeEnum(themeEnum);
                else
                    icon.setTheme(theme);
            }
            if (const int flags = iconStateFlags(di)) { // new, post 4.4 format
                if (flags & NormalOff)
                    setIconPixmap(QIcon::Normal, QIcon::Off, workingDirectory, di->elementNormalOff()->text(), icon, m_lang);
                if (flags & NormalOn)
                    setIconPixmap(QIcon::Normal, QIcon::On, workingDirectory, di->elementNormalOn()->text(), icon, m_lang);
                if (flags & DisabledOff)
                    setIconPixmap(QIcon::Disabled, QIcon::Off, workingDirectory, di->elementDisabledOff()->text(), icon, m_lang);
                if (flags & DisabledOn)
                    setIconPixmap(QIcon::Disabled, QIcon::On, workingDirectory, di->elementDisabledOn()->text(), icon, m_lang);
                if (flags & ActiveOff)
                    setIconPixmap(QIcon::Active, QIcon::Off, workingDirectory, di->elementActiveOff()->text(), icon, m_lang);
                if (flags & ActiveOn)
                    setIconPixmap(QIcon::Active, QIcon::On, workingDirectory, di->elementActiveOn()->text(), icon, m_lang);
                if (flags & SelectedOff)
                    setIconPixmap(QIcon::Selected, QIcon::Off, workingDirectory, di->elementSelectedOff()->text(), icon, m_lang);
                if (flags & SelectedOn)
                    setIconPixmap(QIcon::Selected, QIcon::On, workingDirectory, di->elementSelectedOn()->text(), icon, m_lang);
            } else if (!hasTheme) {
#ifdef OLD_RESOURCE_FORMAT
                setIconPixmap(QIcon::Normal, QIcon::Off, workingDirectory, di->text(), icon, m_lang);
                if (di->hasAttributeResource())
                    m_loadedQrcFiles.insert(QFileInfo(workingDirectory, di->attributeResource()).absoluteFilePath(), false);
#endif
            }
            return QVariant::fromValue(icon);
        }
        default:
            break;
    }
    return QVariant();
}

QVariant QDesignerResourceBuilder::toNativeValue(const QVariant &value) const
{
    if (value.canConvert<PropertySheetPixmapValue>()) {
        if (m_pixmapCache)
            return m_pixmapCache->pixmap(qvariant_cast<PropertySheetPixmapValue>(value));
    } else if (value.canConvert<PropertySheetIconValue>()) {
        if (m_iconCache)
            return m_iconCache->icon(qvariant_cast<PropertySheetIconValue>(value));
    }
    return value;
}

DomProperty *QDesignerResourceBuilder::saveResource(const QDir &workingDirectory, const QVariant &value) const
{
    DomProperty *p = new DomProperty;
    if (value.canConvert<PropertySheetPixmapValue>()) {
        const PropertySheetPixmapValue pix = qvariant_cast<PropertySheetPixmapValue>(value);
        DomResourcePixmap *rp = new DomResourcePixmap;
        const QString pixPath = pix.path();
        switch (pix.pixmapSource(m_core)) {
        case PropertySheetPixmapValue::LanguageResourcePixmap:
            rp->setText(pixPath);
            break;
        case PropertySheetPixmapValue::ResourcePixmap: {
            rp->setText(pixPath);
            const QString qrcFile = m_core->resourceModel()->qrcPath(pixPath);
            if (!qrcFile.isEmpty()) {
                m_usedQrcFiles.insert(qrcFile, false);
#ifdef OLD_RESOURCE_FORMAT  // Legacy: Add qrc path
                rp->setAttributeResource(workingDirectory.relativeFilePath(qrcFile));
#endif
            }
        }
            break;
        case PropertySheetPixmapValue::FilePixmap:
            rp->setText(m_saveRelative ? workingDirectory.relativeFilePath(pixPath) : pixPath);
            break;
        }
        p->setElementPixmap(rp);
        return p;
    }
    if (value.canConvert<PropertySheetIconValue>()) {
        const PropertySheetIconValue icon = qvariant_cast<PropertySheetIconValue>(value);
        const auto &pixmaps = icon.paths();
        const int themeEnum = icon.themeEnum();
        const QString theme = themeEnum != -1
            ? QDesignerResourceBuilder::fullyQualifiedThemeIconName(themeEnum) : icon.theme();
        if (!pixmaps.isEmpty() || !theme.isEmpty()) {
            DomResourceIcon *ri = new DomResourceIcon;
            if (!theme.isEmpty())
                ri->setAttributeTheme(theme);
            for (auto itPix = pixmaps.cbegin(), end = pixmaps.cend(); itPix != end; ++itPix) {
                const QIcon::Mode mode = itPix.key().first;
                const QIcon::State state = itPix.key().second;
                DomResourcePixmap *rp = new DomResourcePixmap;
                const PropertySheetPixmapValue &pix = itPix.value();
                const PropertySheetPixmapValue::PixmapSource ps = pix.pixmapSource(m_core);
                const QString pixPath = pix.path();
                rp->setText(ps == PropertySheetPixmapValue::FilePixmap && m_saveRelative ? workingDirectory.relativeFilePath(pixPath) : pixPath);
                if (state == QIcon::Off) {
                    switch (mode) {
                        case QIcon::Normal:
                            ri->setElementNormalOff(rp);
#ifdef OLD_RESOURCE_FORMAT  // Legacy: Set Normal off as text/path in old format.
                            ri->setText(rp->text());
#endif
                            if (ps == PropertySheetPixmapValue::ResourcePixmap) {
                                // Be sure that ri->text() file comes from active resourceSet (i.e. make appropriate
                                // resourceSet active before calling this method).
                                const QString qrcFile = m_core->resourceModel()->qrcPath(ri->text());
                                if (!qrcFile.isEmpty()) {
                                    m_usedQrcFiles.insert(qrcFile, false);
#ifdef OLD_RESOURCE_FORMAT  // Legacy: Set Normal off as text/path in old format.
                                    ri->setAttributeResource(workingDirectory.relativeFilePath(qrcFile));
#endif
                                }
                            }
                        break;
                        case QIcon::Disabled: ri->setElementDisabledOff(rp); break;
                        case QIcon::Active:   ri->setElementActiveOff(rp);   break;
                        case QIcon::Selected: ri->setElementSelectedOff(rp); break;
                    }
                } else {
                    switch (mode) {
                        case QIcon::Normal:   ri->setElementNormalOn(rp);   break;
                        case QIcon::Disabled: ri->setElementDisabledOn(rp); break;
                        case QIcon::Active:   ri->setElementActiveOn(rp);   break;
                        case QIcon::Selected: ri->setElementSelectedOn(rp); break;
                    }
                }
            }
            p->setElementIconSet(ri);
            return p;
        }
    }
    delete p;
    return nullptr;
}

bool QDesignerResourceBuilder::isResourceType(const QVariant &value) const
{
    return value.canConvert<PropertySheetPixmapValue>()
        || value.canConvert<PropertySheetIconValue>();
}
// ------------------------- QDesignerTextBuilder

template <class DomElement> // for DomString, potentially DomStringList
inline void translationParametersToDom(const PropertySheetTranslatableData &data, DomElement *e)
{
    const QString propertyComment = data.disambiguation();
    if (!propertyComment.isEmpty())
        e->setAttributeComment(propertyComment);
    const QString propertyExtracomment = data.comment();
    if (!propertyExtracomment.isEmpty())
        e->setAttributeExtraComment(propertyExtracomment);
    const QString &id = data.id();
    if (!id.isEmpty())
        e->setAttributeId(id);
    if (!data.translatable())
        e->setAttributeNotr(u"true"_s);
}

template <class DomElement> // for DomString, potentially DomStringList
inline void translationParametersFromDom(const DomElement *e, PropertySheetTranslatableData *data)
{
    if (e->hasAttributeComment())
        data->setDisambiguation(e->attributeComment());
    if (e->hasAttributeExtraComment())
        data->setComment(e->attributeExtraComment());
    if (e->hasAttributeId())
        data->setId(e->attributeId());
    if (e->hasAttributeNotr()) {
        const QString notr = e->attributeNotr();
        const bool translatable = !(notr == "true"_L1 || notr == "yes"_L1);
        data->setTranslatable(translatable);
    }
}

class QDesignerTextBuilder : public QTextBuilder
{
public:
    QDesignerTextBuilder()  = default;

    QVariant loadText(const DomProperty *icon) const override;

    QVariant toNativeValue(const QVariant &value) const override;

    DomProperty *saveText(const QVariant &value) const override;
};

QVariant QDesignerTextBuilder::loadText(const DomProperty *text) const
{
    if (const DomString *domString = text->elementString()) {
        PropertySheetStringValue stringValue(domString->text());
        translationParametersFromDom(domString, &stringValue);
        return QVariant::fromValue(stringValue);
    }
    return QVariant(QString());
}

QVariant QDesignerTextBuilder::toNativeValue(const QVariant &value) const
{
    if (value.canConvert<PropertySheetStringValue>())
        return QVariant::fromValue(qvariant_cast<PropertySheetStringValue>(value).value());
    return value;
}

static inline DomProperty *stringToDomProperty(const QString &value)
{
    DomString *domString = new DomString();
    domString->setText(value);
    DomProperty *property = new DomProperty();
    property->setElementString(domString);
    return property;
}

static inline DomProperty *stringToDomProperty(const QString &value,
                                               const PropertySheetTranslatableData &translatableData)
{
    DomString *domString = new DomString();
    domString->setText(value);
    translationParametersToDom(translatableData, domString);
    DomProperty *property = new DomProperty();
    property->setElementString(domString);
    return property;
}

DomProperty *QDesignerTextBuilder::saveText(const QVariant &value) const
{
    if (value.canConvert<PropertySheetStringValue>()) {
        const PropertySheetStringValue str = qvariant_cast<PropertySheetStringValue>(value);
        return stringToDomProperty(str.value(), str);
    }
    if (value.canConvert<QString>())
        return stringToDomProperty(value.toString());
    return nullptr;
}

QDesignerResource::QDesignerResource(FormWindow *formWindow)  :
    QEditorFormBuilder(formWindow->core()),
    m_formWindow(formWindow),
    m_copyWidget(false),
    m_selected(nullptr),
    m_resourceBuilder(new QDesignerResourceBuilder(m_formWindow->core(), m_formWindow->pixmapCache(), m_formWindow->iconCache()))
{
    // Check language unless extension present (Jambi)
    QDesignerFormEditorInterface *core = m_formWindow->core();
    if (const QDesignerLanguageExtension *le = qt_extension<QDesignerLanguageExtension*>(core->extensionManager(), core))
        d->m_language = le->name();

    setWorkingDirectory(formWindow->absoluteDir());
    setResourceBuilder(m_resourceBuilder);
    setTextBuilder(new QDesignerTextBuilder());

    // ### generalise
    const QString designerWidget = u"QDesignerWidget"_s;
    const QString layoutWidget   = u"QLayoutWidget"_s;
    const QString widget = u"QWidget"_s;
    m_internal_to_qt.insert(layoutWidget, widget);
    m_internal_to_qt.insert(designerWidget, widget);
    m_internal_to_qt.insert(u"QDesignerDialog"_s, u"QDialog"_s);
    m_internal_to_qt.insert(u"QDesignerMenuBar"_s, u"QMenuBar"_s);
    m_internal_to_qt.insert(u"QDesignerMenu"_s, u"QMenu"_s);
    m_internal_to_qt.insert(u"QDesignerDockWidget"_s, u"QDockWidget"_s);

    // invert
    for (auto it = m_internal_to_qt.cbegin(), cend = m_internal_to_qt.cend(); it != cend; ++it )  {
        if (it.value() != designerWidget  && it.value() != layoutWidget)
            m_qt_to_internal.insert(it.value(), it.key());

    }
}

QDesignerResource::~QDesignerResource() = default;

DomUI *QDesignerResource::readUi(QIODevice *dev)
{
    return d->readUi(dev);
}

static inline QString messageBoxTitle()
{
    return QApplication::translate("Designer", "Qt Widgets Designer");
}

void QDesignerResource::save(QIODevice *dev, QWidget *widget)
{
    // Do not write fully qualified enumerations for spacer/line orientations
    // and other enum/flag properties for older Qt versions since that breaks
    // older uic.
    d->m_fullyQualifiedEnums = supportsQualifiedEnums(qtVersion(m_formWindow->core()));
    QAbstractFormBuilder::save(dev, widget);
}

void QDesignerResource::saveDom(DomUI *ui, QWidget *widget)
{
    QAbstractFormBuilder::saveDom(ui, widget);

    QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(core()->extensionManager(), widget);
    Q_ASSERT(sheet != nullptr);

    const QVariant classVar = sheet->property(sheet->indexOf(u"objectName"_s));
    QString classStr;
    if (classVar.canConvert<QString>())
        classStr = classVar.toString();
    else
        classStr = qvariant_cast<PropertySheetStringValue>(classVar).value();
    ui->setElementClass(classStr);

    for (int index = 0; index < m_formWindow->toolCount(); ++index) {
        QDesignerFormWindowToolInterface *tool = m_formWindow->tool(index);
        Q_ASSERT(tool != nullptr);
        tool->saveToDom(ui, widget);
    }

    const QString author = m_formWindow->author();
    if (!author.isEmpty()) {
        ui->setElementAuthor(author);
    }

    const QString comment = m_formWindow->comment();
    if (!comment.isEmpty()) {
        ui->setElementComment(comment);
    }

    const QString exportMacro = m_formWindow->exportMacro();
    if (!exportMacro.isEmpty()) {
        ui->setElementExportMacro(exportMacro);
    }

    if (m_formWindow->useIdBasedTranslations())
        ui->setAttributeIdbasedtr(true);
    if (!m_formWindow->connectSlotsByName()) // Don't write out if true (default)
        ui->setAttributeConnectslotsbyname(false);

    const QVariantMap designerFormData = m_formWindow->formData();
    if (!designerFormData.isEmpty()) {
        DomPropertyList domPropertyList;
        for (auto it = designerFormData.cbegin(), cend = designerFormData.cend(); it != cend; ++it) {
            if (DomProperty *prop = variantToDomProperty(this, widget->metaObject(), it.key(), it.value()))
                domPropertyList += prop;
        }
        if (!domPropertyList.isEmpty()) {
            DomDesignerData* domDesignerFormData = new DomDesignerData;
            domDesignerFormData->setElementProperty(domPropertyList);
            ui->setElementDesignerdata(domDesignerFormData);
        }
    }

    if (!m_formWindow->includeHints().isEmpty()) {
        const QString local = u"local"_s;
        const QString global = u"global"_s;
        QList<DomInclude *> ui_includes;
        const QStringList &includeHints = m_formWindow->includeHints();
        ui_includes.reserve(includeHints.size());
        for (QString includeHint : includeHints) {
            if (includeHint.isEmpty())
                continue;
            DomInclude *incl = new DomInclude;
            const QString location = includeHint.at(0) == u'<' ? global : local;
            includeHint.remove(u'"');
            includeHint.remove(u'<');
            includeHint.remove(u'>');
            incl->setAttributeLocation(location);
            incl->setText(includeHint);
            ui_includes.append(incl);
        }

        DomIncludes *includes = new DomIncludes;
        includes->setElementInclude(ui_includes);
        ui->setElementIncludes(includes);
    }

    int defaultMargin = INT_MIN, defaultSpacing = INT_MIN;
    m_formWindow->layoutDefault(&defaultMargin, &defaultSpacing);

    if (defaultMargin != INT_MIN || defaultSpacing != INT_MIN) {
        DomLayoutDefault *def = new DomLayoutDefault;
        if (defaultMargin != INT_MIN)
            def->setAttributeMargin(defaultMargin);
        if (defaultSpacing != INT_MIN)
            def->setAttributeSpacing(defaultSpacing);
        ui->setElementLayoutDefault(def);
    }

    QString marginFunction, spacingFunction;
    m_formWindow->layoutFunction(&marginFunction, &spacingFunction);
    if (!marginFunction.isEmpty() || !spacingFunction.isEmpty()) {
        DomLayoutFunction *def = new DomLayoutFunction;

        if (!marginFunction.isEmpty())
            def->setAttributeMargin(marginFunction);
        if (!spacingFunction.isEmpty())
            def->setAttributeSpacing(spacingFunction);
        ui->setElementLayoutFunction(def);
    }

    QString pixFunction = m_formWindow->pixmapFunction();
    if (!pixFunction.isEmpty()) {
        ui->setElementPixmapFunction(pixFunction);
    }

    if (QDesignerExtraInfoExtension *extra = qt_extension<QDesignerExtraInfoExtension*>(core()->extensionManager(), core()))
        extra->saveUiExtraInfo(ui);

    if (MetaDataBase *metaDataBase = qobject_cast<MetaDataBase *>(core()->metaDataBase())) {
        const MetaDataBaseItem *item = metaDataBase->metaDataBaseItem(m_formWindow->mainContainer());
        const QStringList fakeSlots = item->fakeSlots();
        const QStringList fakeSignals  =item->fakeSignals();
        if (!fakeSlots.isEmpty() || !fakeSignals.isEmpty()) {
            DomSlots *domSlots = new DomSlots();
            domSlots->setElementSlot(fakeSlots);
            domSlots->setElementSignal(fakeSignals);
            ui->setElementSlots(domSlots);
        }
    }
}

QWidget *QDesignerResource::load(QIODevice *dev, QWidget *parentWidget)
{
    QScopedPointer<DomUI> ui(readUi(dev));
    return ui.isNull() ? nullptr : loadUi(ui.data(), parentWidget);
}

QWidget *QDesignerResource::loadUi(DomUI *ui, QWidget *parentWidget)
{
    QWidget *widget = create(ui, parentWidget);
    // Store the class name as 'reset' value for the main container's object name.
    if (widget)
        widget->setProperty("_q_classname", widget->objectName());
    else if (d->m_errorString.isEmpty())
        d->m_errorString = QFormBuilderExtra::msgInvalidUiFile();
    return widget;
}

bool QDesignerResource::saveRelative() const
{
    return m_resourceBuilder->isSaveRelative();
}

void QDesignerResource::setSaveRelative(bool relative)
{
    m_resourceBuilder->setSaveRelative(relative);
}

QWidget *QDesignerResource::create(DomUI *ui, QWidget *parentWidget)
{
    // Load extra info extension. This is used by Jambi for preventing
    // C++ UI files from being loaded
    if (QDesignerExtraInfoExtension *extra = qt_extension<QDesignerExtraInfoExtension*>(core()->extensionManager(), core())) {
        if (!extra->loadUiExtraInfo(ui)) {
            const QString errorMessage = QApplication::translate("Designer", "This file cannot be read because the extra info extension failed to load.");
            core()->dialogGui()->message(parentWidget->window(), QDesignerDialogGuiInterface::FormLoadFailureMessage,
                                         QMessageBox::Warning, messageBoxTitle(), errorMessage, QMessageBox::Ok);
            return nullptr;
        }
    }

    qdesigner_internal::WidgetFactory *factory = qobject_cast<qdesigner_internal::WidgetFactory*>(core()->widgetFactory());
    Q_ASSERT(factory != nullptr);

    QDesignerFormWindowInterface *previousFormWindow = factory->currentFormWindow(m_formWindow);

    m_isMainWidget = true;
    QDesignerWidgetItemInstaller wii; // Make sure we use QDesignerWidgetItem.
    QWidget *mainWidget = QAbstractFormBuilder::create(ui, parentWidget);

    if (m_formWindow) {
        m_formWindow->setUseIdBasedTranslations(ui->attributeIdbasedtr());
        // Default to true unless set.
        const bool connectSlotsByName = !ui->hasAttributeConnectslotsbyname() || ui->attributeConnectslotsbyname();
        m_formWindow->setConnectSlotsByName(connectSlotsByName);
    }

    if (mainWidget && m_formWindow) {
        m_formWindow->setAuthor(ui->elementAuthor());
        m_formWindow->setComment(ui->elementComment());
        m_formWindow->setExportMacro(ui->elementExportMacro());

        // Designer data
        QVariantMap designerFormData;
        if (ui->hasElementDesignerdata()) {
            const DomPropertyList domPropertyList = ui->elementDesignerdata()->elementProperty();
            for (auto *prop : domPropertyList) {
                const QVariant vprop = domPropertyToVariant(this, mainWidget->metaObject(), prop);
                if (vprop.metaType().id() != QMetaType::UnknownType)
                    designerFormData.insert(prop->attributeName(), vprop);
            }
        }
        m_formWindow->setFormData(designerFormData);

        m_formWindow->setPixmapFunction(ui->elementPixmapFunction());

        if (DomLayoutDefault *def = ui->elementLayoutDefault()) {
            m_formWindow->setLayoutDefault(def->attributeMargin(), def->attributeSpacing());
        }

        if (DomLayoutFunction *fun = ui->elementLayoutFunction()) {
            m_formWindow->setLayoutFunction(fun->attributeMargin(), fun->attributeSpacing());
        }

        if (DomIncludes *includes = ui->elementIncludes()) {
            const auto global = "global"_L1;
            QStringList includeHints;
            const auto &elementInclude = includes->elementInclude();
            for (DomInclude *incl : elementInclude) {
                QString text = incl->text();

                if (text.isEmpty())
                    continue;

                if (incl->hasAttributeLocation() && incl->attributeLocation() == global ) {
                    text.prepend(u'<');
                    text.append(u'>');
                } else {
                    text.prepend(u'"');
                    text.append(u'"');
                }

                includeHints.append(text);
            }

            m_formWindow->setIncludeHints(includeHints);
        }

        // Register all button groups the form builder adds as children of the main container for them to be found
        // in the signal slot editor
        auto *mdb = core()->metaDataBase();
        for (auto *child : mainWidget->children()) {
            if (QButtonGroup *bg = qobject_cast<QButtonGroup*>(child))
                mdb->add(bg);
        }
        // Load tools
        for (int index = 0; index < m_formWindow->toolCount(); ++index) {
            QDesignerFormWindowToolInterface *tool = m_formWindow->tool(index);
            Q_ASSERT(tool != nullptr);
            tool->loadFromDom(ui, mainWidget);
        }
    }

    factory->currentFormWindow(previousFormWindow);

    if (const DomSlots *domSlots = ui->elementSlots()) {
        if (MetaDataBase *metaDataBase = qobject_cast<MetaDataBase *>(core()->metaDataBase())) {
            QStringList fakeSlots;
            QStringList fakeSignals;
            if (addFakeMethods(domSlots, fakeSlots, fakeSignals)) {
                MetaDataBaseItem *item = metaDataBase->metaDataBaseItem(mainWidget);
                item->setFakeSlots(fakeSlots);
                item->setFakeSignals(fakeSignals);
            }
        }
    }
    if (mainWidget) {
        // Initialize the mainwindow geometry. Has it been  explicitly specified?
        bool hasExplicitGeometry = false;
        const auto &properties = ui->elementWidget()->elementProperty();
        if (!properties.isEmpty()) {
            for (const DomProperty *p : properties) {
                if (p->attributeName() == "geometry"_L1) {
                    hasExplicitGeometry = true;
                    break;
                }
            }
        }
        if (hasExplicitGeometry) {
            // Geometry was specified explicitly: Verify that smartMinSize is respected
            // (changed fonts, label wrapping policies, etc). This does not happen automatically in docked mode.
            const QSize size = mainWidget->size();
            const QSize minSize = size.expandedTo(qSmartMinSize(mainWidget));
            if (minSize != size)
                mainWidget->resize(minSize);
        } else {
            // No explicit Geometry: perform an adjustSize() to resize the form correctly before embedding it into a container
            // (which might otherwise squeeze the form)
            mainWidget->adjustSize();
        }
        // Some integration wizards create forms with main containers
        // based on derived classes of QWidget and load them into Designer
        // without the plugin existing. This will trigger the auto-promotion
        // mechanism of Designer, which will set container=false for
        // QWidgets. For the main container, force container=true and warn.
        const QDesignerWidgetDataBaseInterface *wdb = core()->widgetDataBase();
        const int wdbIndex = wdb->indexOfObject(mainWidget);
        if (wdbIndex != -1) {
            QDesignerWidgetDataBaseItemInterface *item = wdb->item(wdbIndex);
            // Promoted main container that is not of container type
            if (item->isPromoted() && !item->isContainer()) {
                item->setContainer(true);
                qWarning("** WARNING The form's main container is an unknown custom widget '%s'."
                         " Defaulting to a promoted instance of '%s', assuming container.",
                         item->name().toUtf8().constData(), item->extends().toUtf8().constData());
            }
        }
    }
    return mainWidget;
}

QWidget *QDesignerResource::create(DomWidget *ui_widget, QWidget *parentWidget)
{
    const QString className = ui_widget->attributeClass();
    if (!m_isMainWidget && className == "QWidget"_L1
        && !ui_widget->elementLayout().isEmpty()
        && !ui_widget->hasAttributeNative()) {
        // ### check if elementLayout.size() == 1

        QDesignerContainerExtension *container = qt_extension<QDesignerContainerExtension*>(core()->extensionManager(), parentWidget);

        if (container == nullptr) {
            // generate a QLayoutWidget iff the parent is not an QDesignerContainerExtension.
            ui_widget->setAttributeClass(u"QLayoutWidget"_s);
        }
    }

    // save the actions
    const auto &actionRefs = ui_widget->elementAddAction();
    ui_widget->setElementAddAction(QList<DomActionRef *>());

    QWidget *w = QAbstractFormBuilder::create(ui_widget, parentWidget);

    // restore the actions
    ui_widget->setElementAddAction(actionRefs);

    if (w == nullptr)
       return nullptr;

    // ### generalize using the extension manager
    QDesignerMenu *menu = qobject_cast<QDesignerMenu*>(w);
    QDesignerMenuBar *menuBar = qobject_cast<QDesignerMenuBar*>(w);

    if (menu)
        menu->hide();

    for (DomActionRef *ui_action_ref : actionRefs) {
        const QString name = ui_action_ref->attributeName();
        if (name == "separator"_L1) {
            QAction *sep = new QAction(w);
            sep->setSeparator(true);
            w->addAction(sep);
            addMenuAction(sep);
        } else if (QAction *a = d->m_actions.value(name)) {
            w->addAction(a);
        } else if (QActionGroup *g = d->m_actionGroups.value(name)) {
            w->addActions(g->actions());
        } else if (QMenu *menu = w->findChild<QMenu*>(name)) {
            w->addAction(menu->menuAction());
            addMenuAction(menu->menuAction());
        }
    }

    if (menu)
        menu->adjustSpecialActions();
    else if (menuBar)
        menuBar->adjustSpecialActions();

    ui_widget->setAttributeClass(className); // fix the class name
    applyExtensionDataFromDOM(this, core(), ui_widget, w);

    return w;
}

QLayout *QDesignerResource::create(DomLayout *ui_layout, QLayout *layout, QWidget *parentWidget)
{
    QLayout *l = QAbstractFormBuilder::create(ui_layout, layout, parentWidget);

    if (QGridLayout *gridLayout = qobject_cast<QGridLayout*>(l)) {
        QLayoutSupport::createEmptyCells(gridLayout);
    } else {
        if (QFormLayout *formLayout = qobject_cast<QFormLayout*>(l))
            QLayoutSupport::createEmptyCells(formLayout);
    }
    // While the actual values are applied by the form builder, we still need
    // to mark them as 'changed'.
    LayoutPropertySheet::markChangedStretchProperties(core(), l, ui_layout);
    return l;
}

QLayoutItem *QDesignerResource::create(DomLayoutItem *ui_layoutItem, QLayout *layout, QWidget *parentWidget)
{
    if (ui_layoutItem->kind() == DomLayoutItem::Spacer) {
        const DomSpacer *domSpacer = ui_layoutItem->elementSpacer();
        Spacer *spacer = static_cast<Spacer*>(core()->widgetFactory()->createWidget(u"Spacer"_s, parentWidget));
        if (domSpacer->hasAttributeName())
            changeObjectName(spacer, domSpacer->attributeName());
        core()->metaDataBase()->add(spacer);

        spacer->setInteractiveMode(false);
        applyProperties(spacer, ui_layoutItem->elementSpacer()->elementProperty());
        spacer->setInteractiveMode(true);

        if (m_formWindow) {
            m_formWindow->manageWidget(spacer);
            if (QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(core()->extensionManager(), spacer))
                sheet->setChanged(sheet->indexOf(u"orientation"_s), true);
        }

        return new QWidgetItem(spacer);
    }
    if (ui_layoutItem->kind() == DomLayoutItem::Layout && parentWidget) {
        DomLayout *ui_layout = ui_layoutItem->elementLayout();
        QLayoutWidget *layoutWidget = new QLayoutWidget(m_formWindow, parentWidget);
        core()->metaDataBase()->add(layoutWidget);
        if (m_formWindow)
            m_formWindow->manageWidget(layoutWidget);
        (void) create(ui_layout, nullptr, layoutWidget);
        return new QWidgetItem(layoutWidget);
    }
    return QAbstractFormBuilder::create(ui_layoutItem, layout, parentWidget);
}

void QDesignerResource::changeObjectName(QObject *o, QString objName)
{
    m_formWindow->unify(o, objName, true);
    o->setObjectName(objName);

}

/* If the property is a enum or flag value, retrieve
 * the existing enum/flag via property sheet and use it to convert */

static bool readDomEnumerationValue(const DomProperty *p,
                                    const QDesignerPropertySheetExtension* sheet, int index,
                                    QVariant &v)
{
    switch (p->kind()) {
    case DomProperty::Set: {
        const QVariant sheetValue = sheet->property(index);
        if (sheetValue.canConvert<PropertySheetFlagValue>()) {
            const PropertySheetFlagValue f = qvariant_cast<PropertySheetFlagValue>(sheetValue);
            bool ok = false;
            v = f.metaFlags.parseFlags(p->elementSet(), &ok);
            if (!ok)
                designerWarning(f.metaFlags.messageParseFailed(p->elementSet()));
            return true;
        }
    }
        break;
    case DomProperty::Enum: {
        const QVariant sheetValue = sheet->property(index);
        if (sheetValue.canConvert<PropertySheetEnumValue>()) {
            const PropertySheetEnumValue e = qvariant_cast<PropertySheetEnumValue>(sheetValue);
            bool ok = false;
            v = e.metaEnum.parseEnum(p->elementEnum(), &ok);
            if (!ok)
                designerWarning(e.metaEnum.messageParseFailed(p->elementEnum()));
            return true;
        }
    }
        break;
    default:
        break;
    }
    return false;
}

// ### fixme Qt 7 remove this: Exclude deprecated properties of Qt 5.
static bool isDeprecatedQt5Property(const QObject *o, const DomProperty *p)
{
    const QString &propertyName = p->attributeName();
    switch (p->kind()) {
    case DomProperty::Set:
        if (propertyName == u"features" && o->inherits("QDockWidget")
            && p->elementSet() == u"QDockWidget::AllDockWidgetFeatures") {
            return true;
        }
        break;
    case DomProperty::Enum:
        if (propertyName == u"sizeAdjustPolicy" && o->inherits("QComboBox")
            && p->elementEnum() == u"QComboBox::AdjustToMinimumContentsLength") {
            return true;
        }
        break;
    default:
        break;
    }
    return false;
}

void QDesignerResource::applyProperties(QObject *o, const QList<DomProperty*> &properties)
{
    if (properties.isEmpty())
        return;

    QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(core()->extensionManager(), o);
    if (!sheet)
        return;

    QDesignerDynamicPropertySheetExtension *dynamicSheet = qt_extension<QDesignerDynamicPropertySheetExtension*>(core()->extensionManager(), o);
    const bool dynamicPropertiesAllowed = dynamicSheet && dynamicSheet->dynamicPropertiesAllowed();

    for (DomProperty *p : properties) {
        if (isDeprecatedQt5Property(o, p)) // ### fixme Qt 7 remove this
            continue; // ### fixme Qt 7 remove this: Exclude deprecated value of Qt 5.
        QString propertyName = p->attributeName();
        if (propertyName == "numDigits"_L1 && o->inherits("QLCDNumber")) // Deprecated in Qt 4, removed in Qt 5.
            propertyName = u"digitCount"_s;
        const int index = sheet->indexOf(propertyName);
        QVariant v;
        if (!readDomEnumerationValue(p, sheet, index, v))
            v = toVariant(o->metaObject(), p);

        switch (p->kind()) {
        case DomProperty::String:
            if (index != -1 && sheet->property(index).userType() == qMetaTypeId<PropertySheetKeySequenceValue>()) {
                const DomString *key = p->elementString();
                PropertySheetKeySequenceValue keyVal(QKeySequence(key->text()));
                translationParametersFromDom(key, &keyVal);
                v = QVariant::fromValue(keyVal);
            } else {
                const DomString *str = p->elementString();
                PropertySheetStringValue strVal(v.toString());
                translationParametersFromDom(str, &strVal);
                v = QVariant::fromValue(strVal);
            }
            break;
        case DomProperty::StringList: {
            const DomStringList *list = p->elementStringList();
            PropertySheetStringListValue listValue(list->elementString());
            translationParametersFromDom(list, &listValue);
            v = QVariant::fromValue(listValue);
        }
            break;
        default:
            break;
        }

        d->applyPropertyInternally(o, propertyName, v);
        if (index != -1) {
            sheet->setProperty(index, v);
            sheet->setChanged(index, true);
        } else if (dynamicPropertiesAllowed) {
            QVariant defaultValue = QVariant(v.metaType());
            bool isDefault = (v == defaultValue);
            if (v.canConvert<PropertySheetIconValue>()) {
                defaultValue = QVariant(QMetaType(QMetaType::QIcon));
                isDefault = (qvariant_cast<PropertySheetIconValue>(v) == PropertySheetIconValue());
            } else if (v.canConvert<PropertySheetPixmapValue>()) {
                defaultValue = QVariant(QMetaType(QMetaType::QPixmap));
                isDefault = (qvariant_cast<PropertySheetPixmapValue>(v) == PropertySheetPixmapValue());
            } else if (v.canConvert<PropertySheetStringValue>()) {
                defaultValue = QVariant(QMetaType(QMetaType::QString));
                isDefault = (qvariant_cast<PropertySheetStringValue>(v) == PropertySheetStringValue());
            } else if (v.canConvert<PropertySheetStringListValue>()) {
                defaultValue = QVariant(QMetaType(QMetaType::QStringList));
                isDefault = (qvariant_cast<PropertySheetStringListValue>(v) == PropertySheetStringListValue());
            } else if (v.canConvert<PropertySheetKeySequenceValue>()) {
                defaultValue = QVariant(QMetaType(QMetaType::QKeySequence));
                isDefault = (qvariant_cast<PropertySheetKeySequenceValue>(v) == PropertySheetKeySequenceValue());
            }
            if (defaultValue.metaType().id() != QMetaType::User) {
                const int idx = dynamicSheet->addDynamicProperty(p->attributeName(), defaultValue);
                if (idx != -1) {
                    sheet->setProperty(idx, v);
                    sheet->setChanged(idx, !isDefault);
                }
            }
        }

        if (propertyName == "objectName"_L1)
            changeObjectName(o, o->objectName());
    }
}

QWidget *QDesignerResource::createWidget(const QString &widgetName, QWidget *parentWidget, const QString &_name)
{
    QString name = _name;
    if (m_isMainWidget)
        m_isMainWidget = false;

    QWidget *w = core()->widgetFactory()->createWidget(widgetName, parentWidget);
    if (!w)
        return nullptr;

    if (name.isEmpty()) {
        QDesignerWidgetDataBaseInterface *db = core()->widgetDataBase();
        if (QDesignerWidgetDataBaseItemInterface *item = db->item(db->indexOfObject(w)))
            name = qtify(item->name());
    }

    changeObjectName(w, name);

    QDesignerContainerExtension *container = qt_extension<QDesignerContainerExtension*>(core()->extensionManager(), parentWidget);
    if (!qobject_cast<QMenu*>(w) && (!parentWidget || !container)) {
        m_formWindow->manageWidget(w);
        if (parentWidget) {
            QWidgetList list = qvariant_cast<QWidgetList>(parentWidget->property("_q_widgetOrder"));
            list.append(w);
            parentWidget->setProperty("_q_widgetOrder", QVariant::fromValue(list));
            QWidgetList zOrder = qvariant_cast<QWidgetList>(parentWidget->property("_q_zOrder"));
            zOrder.append(w);
            parentWidget->setProperty("_q_zOrder", QVariant::fromValue(zOrder));
        }
    } else {
        core()->metaDataBase()->add(w);
    }

    w->setWindowFlags(w->windowFlags() & ~Qt::Window);
    // Make sure it is non-modal (for example, KDialog calls setModal(true) in the constructor).
    w->setWindowModality(Qt::NonModal);

    return w;
}

QLayout *QDesignerResource::createLayout(const QString &layoutName, QObject *parent, const QString &name)
{
    QWidget *layoutBase = nullptr;
    QLayout *layout = qobject_cast<QLayout*>(parent);

    if (parent->isWidgetType())
        layoutBase = static_cast<QWidget*>(parent);
    else {
        Q_ASSERT( layout != nullptr );
        layoutBase = layout->parentWidget();
    }

    LayoutInfo::Type layoutType = LayoutInfo::layoutType(layoutName);
    if (layoutType == LayoutInfo::NoLayout) {
        designerWarning(QCoreApplication::translate("QDesignerResource", "The layout type '%1' is not supported, defaulting to grid.").arg(layoutName));
        layoutType = LayoutInfo::Grid;
    }
    QLayout *lay = core()->widgetFactory()->createLayout(layoutBase, layout, layoutType);
    if (lay != nullptr)
        changeObjectName(lay, name);

    return lay;
}

// save
DomWidget *QDesignerResource::createDom(QWidget *widget, DomWidget *ui_parentWidget, bool recursive)
{
    QDesignerMetaDataBaseItemInterface *item = core()->metaDataBase()->item(widget);
    if (!item)
        return nullptr;

    if (qobject_cast<Spacer*>(widget) && !m_copyWidget)
        return nullptr;

    const QDesignerWidgetDataBaseInterface *wdb = core()->widgetDataBase();
    QDesignerWidgetDataBaseItemInterface *widgetInfo =  nullptr;
    const int widgetInfoIndex = wdb->indexOfObject(widget, false);
    if (widgetInfoIndex != -1) {
        widgetInfo = wdb->item(widgetInfoIndex);
        // Recursively add all dependent custom widgets
        QDesignerWidgetDataBaseItemInterface *customInfo = widgetInfo;
        while (customInfo && customInfo->isCustom()) {
            m_usedCustomWidgets.insert(customInfo, true);
            const QString extends = customInfo->extends();
            if (extends == customInfo->name())
                break; // There are faulty files around that have name==extends
            const int extendsIndex = wdb->indexOfClassName(customInfo->extends());
            customInfo = extendsIndex != -1 ?  wdb->item(extendsIndex) : nullptr;
        }
    }

    DomWidget *w = nullptr;

    if (QTabWidget *tabWidget = qobject_cast<QTabWidget*>(widget))
        w = saveWidget(tabWidget, ui_parentWidget);
    else if (QStackedWidget *stackedWidget = qobject_cast<QStackedWidget*>(widget))
        w = saveWidget(stackedWidget, ui_parentWidget);
    else if (QToolBox *toolBox = qobject_cast<QToolBox*>(widget))
        w = saveWidget(toolBox, ui_parentWidget);
    else if (QToolBar *toolBar = qobject_cast<QToolBar*>(widget))
        w = saveWidget(toolBar, ui_parentWidget);
    else if (QDesignerDockWidget *dockWidget = qobject_cast<QDesignerDockWidget*>(widget))
        w = saveWidget(dockWidget, ui_parentWidget);
    else if (QDesignerContainerExtension *container = qt_extension<QDesignerContainerExtension*>(core()->extensionManager(), widget))
        w = saveWidget(widget, container, ui_parentWidget);
    else if (QWizardPage *wizardPage = qobject_cast<QWizardPage*>(widget))
        w = saveWidget(wizardPage, ui_parentWidget);
    else
        w = QAbstractFormBuilder::createDom(widget, ui_parentWidget, recursive);

    Q_ASSERT( w != nullptr );

    if (!qobject_cast<QLayoutWidget*>(widget) && w->attributeClass() == "QWidget"_L1)
        w->setAttributeNative(true);

    const QString className = w->attributeClass();
    if (m_internal_to_qt.contains(className))
        w->setAttributeClass(m_internal_to_qt.value(className));

    if (isPromoted( core(), widget)) { // is promoted?
        Q_ASSERT(widgetInfo != nullptr);

        w->setAttributeClass(widgetInfo->name());

        const auto &prop_list = w->elementProperty();
        for (DomProperty *prop : prop_list) {
            if (prop->attributeName() == "geometry"_L1) {
                if (DomRect *rect = prop->elementRect()) {
                    rect->setElementX(widget->x());
                    rect->setElementY(widget->y());
                }
                break;
            }
        }
    } else if (widgetInfo != nullptr && m_usedCustomWidgets.contains(widgetInfo)) {
        if (widgetInfo->name() != w->attributeClass())
            w->setAttributeClass(widgetInfo->name());
    }
    addExtensionDataToDOM(this, core(), w, widget);
    return w;
}

DomLayout *QDesignerResource::createDom(QLayout *layout, DomLayout *ui_parentLayout, DomWidget *ui_parentWidget)
{
    QDesignerMetaDataBaseItemInterface *item = core()->metaDataBase()->item(layout);

    if (item == nullptr) {
        layout = layout->findChild<QLayout*>();
        // refresh the meta database item
        item = core()->metaDataBase()->item(layout);
    }

    if (item == nullptr) {
        // nothing to do.
        return nullptr;
    }

    if (qobject_cast<QSplitter*>(layout->parentWidget()) != 0) {
        // nothing to do.
        return nullptr;
    }

    m_chain.push(layout);

    DomLayout *l = QAbstractFormBuilder::createDom(layout, ui_parentLayout, ui_parentWidget);
    Q_ASSERT(l != nullptr);
    LayoutPropertySheet::stretchAttributesToDom(core(), layout, l);

    m_chain.pop();

    return l;
}

DomLayoutItem *QDesignerResource::createDom(QLayoutItem *item, DomLayout *ui_layout, DomWidget *ui_parentWidget)
{
    DomLayoutItem *ui_item = nullptr;

    if (Spacer *s = qobject_cast<Spacer*>(item->widget())) {
        if (!core()->metaDataBase()->item(s))
            return nullptr;

        DomSpacer *spacer = new DomSpacer();
        const QString objectName = s->objectName();
        if (!objectName.isEmpty())
            spacer->setAttributeName(objectName);
        // ### filter the properties
        spacer->setElementProperty(computeProperties(item->widget()));

        ui_item = new DomLayoutItem();
        ui_item->setElementSpacer(spacer);
        d->m_laidout.insert(item->widget(), true);
    } else if (QLayoutWidget *layoutWidget = qobject_cast<QLayoutWidget*>(item->widget())) {
        // Do not save a QLayoutWidget if it is within a layout (else it is saved as "QWidget"
        Q_ASSERT(layoutWidget->layout());
        DomLayout *l = createDom(layoutWidget->layout(), ui_layout, ui_parentWidget);
        ui_item = new DomLayoutItem();
        ui_item->setElementLayout(l);
        d->m_laidout.insert(item->widget(), true);
    } else if (!item->spacerItem()) { // we use spacer as fake item in the Designer
        ui_item = QAbstractFormBuilder::createDom(item, ui_layout, ui_parentWidget);
    } else {
        return nullptr;
    }
    return ui_item;
}

void QDesignerResource::createCustomWidgets(DomCustomWidgets *dom_custom_widgets)
{
    QSimpleResource::handleDomCustomWidgets(core(), dom_custom_widgets);
}

DomTabStops *QDesignerResource::saveTabStops()
{
    QDesignerMetaDataBaseItemInterface *item = core()->metaDataBase()->item(m_formWindow);
    Q_ASSERT(item);

    QStringList tabStops;
    const QWidgetList &tabOrder = item->tabOrder();
    for (QWidget *widget : tabOrder) {
        if (m_formWindow->mainContainer()->isAncestorOf(widget))
            tabStops.append(widget->objectName());
    }

    if (!tabStops.isEmpty()) {
        DomTabStops *dom = new DomTabStops;
        dom->setElementTabStop(tabStops);
        return dom;
    }

    return nullptr;
}

void QDesignerResource::applyTabStops(QWidget *widget, DomTabStops *tabStops)
{
    if (tabStops == nullptr || widget == nullptr)
        return;

    QWidgetList tabOrder;
    const QStringList &elementTabStop = tabStops->elementTabStop();
    for (const QString &widgetName : elementTabStop) {
        if (QWidget *w = widget->findChild<QWidget*>(widgetName)) {
            tabOrder.append(w);
        }
    }

    QDesignerMetaDataBaseItemInterface *item = core()->metaDataBase()->item(m_formWindow);
    Q_ASSERT(item);
    item->setTabOrder(tabOrder);
}

/* Unmanaged container pages occur when someone adds a page in a custom widget
 * constructor. They don't have a meta DB entry which causes createDom
 * to return 0. */
inline QString msgUnmanagedPage(QDesignerFormEditorInterface *core,
                                QWidget *container, int index, QWidget *page)
{
    return QCoreApplication::translate("QDesignerResource",
"The container extension of the widget '%1' (%2) returned a widget not managed by Designer '%3' (%4) when queried for page #%5.\n"
"Container pages should only be added by specifying them in XML returned by the domXml() method of the custom widget.").
           arg(container->objectName(), WidgetFactory::classNameOf(core, container),
               page->objectName(), WidgetFactory::classNameOf(core, page)).
           arg(index);
}

DomWidget *QDesignerResource::saveWidget(QWidget *widget, QDesignerContainerExtension *container, DomWidget *ui_parentWidget)
{
    DomWidget *ui_widget = QAbstractFormBuilder::createDom(widget, ui_parentWidget, false);
    QList<DomWidget *> ui_widget_list;

    for (int i=0; i<container->count(); ++i) {
        QWidget *page = container->widget(i);
        Q_ASSERT(page);

        if (DomWidget *ui_page = createDom(page, ui_widget)) {
            ui_widget_list.append(ui_page);
        } else {
            designerWarning(msgUnmanagedPage(core(), widget, i, page));
        }
    }

    ui_widget->setElementWidget(ui_widget_list);

    return ui_widget;
}

DomWidget *QDesignerResource::saveWidget(QStackedWidget *widget, DomWidget *ui_parentWidget)
{
    DomWidget *ui_widget = QAbstractFormBuilder::createDom(widget, ui_parentWidget, false);
    QList<DomWidget *> ui_widget_list;
    if (QDesignerContainerExtension *container = qt_extension<QDesignerContainerExtension*>(core()->extensionManager(), widget)) {
        for (int i=0; i<container->count(); ++i) {
            QWidget *page = container->widget(i);
            Q_ASSERT(page);
            if (DomWidget *ui_page = createDom(page, ui_widget)) {
                ui_widget_list.append(ui_page);
            } else {
                designerWarning(msgUnmanagedPage(core(), widget, i, page));
            }
        }
    }

    ui_widget->setElementWidget(ui_widget_list);

    return ui_widget;
}

DomWidget *QDesignerResource::saveWidget(QToolBar *toolBar, DomWidget *ui_parentWidget)
{
    DomWidget *ui_widget = QAbstractFormBuilder::createDom(toolBar, ui_parentWidget, false);
    if (const QMainWindow *mainWindow = qobject_cast<QMainWindow*>(toolBar->parentWidget())) {
        const bool toolBarBreak = mainWindow->toolBarBreak(toolBar);
        const Qt::ToolBarArea area = mainWindow->toolBarArea(toolBar);

        auto attributes = ui_widget->elementAttribute();

        DomProperty *attr = new DomProperty();
        attr->setAttributeName(u"toolBarArea"_s);
        attr->setElementEnum(QLatin1StringView(toolBarAreaMetaEnum().valueToKey(area)));
        attributes  << attr;

        attr = new DomProperty();
        attr->setAttributeName(u"toolBarBreak"_s);
        attr->setElementBool(toolBarBreak ? u"true"_s : u"false"_s);
        attributes  << attr;
        ui_widget->setElementAttribute(attributes);
    }

    return ui_widget;
}

DomWidget *QDesignerResource::saveWidget(QDesignerDockWidget *dockWidget, DomWidget *ui_parentWidget)
{
    DomWidget *ui_widget = QAbstractFormBuilder::createDom(dockWidget, ui_parentWidget, true);
    if (QMainWindow *mainWindow = qobject_cast<QMainWindow*>(dockWidget->parentWidget())) {
        const Qt::DockWidgetArea area = mainWindow->dockWidgetArea(dockWidget);
        DomProperty *attr = new DomProperty();
        attr->setAttributeName(u"dockWidgetArea"_s);
        attr->setElementNumber(int(area));
        ui_widget->setElementAttribute(ui_widget->elementAttribute() << attr);
    }

    return ui_widget;
}

DomWidget *QDesignerResource::saveWidget(QTabWidget *widget, DomWidget *ui_parentWidget)
{
    DomWidget *ui_widget = QAbstractFormBuilder::createDom(widget, ui_parentWidget, false);
    QList<DomWidget *> ui_widget_list;

    if (QDesignerContainerExtension *container = qt_extension<QDesignerContainerExtension*>(core()->extensionManager(), widget)) {
        const int current = widget->currentIndex();
        for (int i=0; i<container->count(); ++i) {
            QWidget *page = container->widget(i);
            Q_ASSERT(page);

            DomWidget *ui_page = createDom(page, ui_widget);
            if (!ui_page) {
                designerWarning(msgUnmanagedPage(core(), widget, i, page));
                continue;
            }
            QList<DomProperty*> ui_attribute_list;

            // attribute `icon'
            widget->setCurrentIndex(i);
            QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(core()->extensionManager(), widget);
            PropertySheetIconValue icon = qvariant_cast<PropertySheetIconValue>(sheet->property(sheet->indexOf(u"currentTabIcon"_s)));
            DomProperty *p = resourceBuilder()->saveResource(workingDirectory(), QVariant::fromValue(icon));
            if (p) {
                p->setAttributeName(QFormBuilderStrings::iconAttribute);
                ui_attribute_list.append(p);
            }
            // attribute `title'
            p = textBuilder()->saveText(sheet->property(sheet->indexOf(u"currentTabText"_s)));
            if (p) {
                p->setAttributeName(QFormBuilderStrings::titleAttribute);
                ui_attribute_list.append(p);
            }

            // attribute `toolTip'
            QVariant v = sheet->property(sheet->indexOf(u"currentTabToolTip"_s));
            if (!qvariant_cast<PropertySheetStringValue>(v).value().isEmpty()) {
                p = textBuilder()->saveText(v);
                if (p) {
                    p->setAttributeName(QFormBuilderStrings::toolTipAttribute);
                ui_attribute_list.append(p);
                }
            }

            // attribute `whatsThis'
            v = sheet->property(sheet->indexOf(u"currentTabWhatsThis"_s));
            if (!qvariant_cast<PropertySheetStringValue>(v).value().isEmpty()) {
                p = textBuilder()->saveText(v);
                if (p) {
                    p->setAttributeName(QFormBuilderStrings::whatsThisAttribute);
                ui_attribute_list.append(p);
                }
            }

            ui_page->setElementAttribute(ui_attribute_list);

            ui_widget_list.append(ui_page);
        }
        widget->setCurrentIndex(current);
    }

    ui_widget->setElementWidget(ui_widget_list);

    return ui_widget;
}

DomWidget *QDesignerResource::saveWidget(QToolBox *widget, DomWidget *ui_parentWidget)
{
    DomWidget *ui_widget = QAbstractFormBuilder::createDom(widget, ui_parentWidget, false);
    QList<DomWidget *> ui_widget_list;

    if (QDesignerContainerExtension *container = qt_extension<QDesignerContainerExtension*>(core()->extensionManager(), widget)) {
        const int current = widget->currentIndex();
        for (int i=0; i<container->count(); ++i) {
            QWidget *page = container->widget(i);
            Q_ASSERT(page);

            DomWidget *ui_page = createDom(page, ui_widget);
            if (!ui_page) {
                designerWarning(msgUnmanagedPage(core(), widget, i, page));
                continue;
            }

            // attribute `label'
            QList<DomProperty*> ui_attribute_list;

            // attribute `icon'
            widget->setCurrentIndex(i);
            QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(core()->extensionManager(), widget);
            PropertySheetIconValue icon = qvariant_cast<PropertySheetIconValue>(sheet->property(sheet->indexOf(u"currentItemIcon"_s)));
            DomProperty *p = resourceBuilder()->saveResource(workingDirectory(), QVariant::fromValue(icon));
            if (p) {
                p->setAttributeName(QFormBuilderStrings::iconAttribute);
                ui_attribute_list.append(p);
            }
            p = textBuilder()->saveText(sheet->property(sheet->indexOf(u"currentItemText"_s)));
            if (p) {
                p->setAttributeName(QFormBuilderStrings::labelAttribute);
                ui_attribute_list.append(p);
            }

            // attribute `toolTip'
            QVariant v = sheet->property(sheet->indexOf(u"currentItemToolTip"_s));
            if (!qvariant_cast<PropertySheetStringValue>(v).value().isEmpty()) {
                p = textBuilder()->saveText(v);
                if (p) {
                    p->setAttributeName(QFormBuilderStrings::toolTipAttribute);
                    ui_attribute_list.append(p);
                }
            }

            ui_page->setElementAttribute(ui_attribute_list);

            ui_widget_list.append(ui_page);
        }
        widget->setCurrentIndex(current);
    }

    ui_widget->setElementWidget(ui_widget_list);

    return ui_widget;
}

DomWidget *QDesignerResource::saveWidget(QWizardPage *wizardPage, DomWidget *ui_parentWidget)
{
    DomWidget *ui_widget = QAbstractFormBuilder::createDom(wizardPage, ui_parentWidget, true);
    QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(core()->extensionManager(), wizardPage);
    // Save the page id (string) attribute, append to existing attributes
    const QString pageIdPropertyName = QLatin1StringView(QWizardPagePropertySheet::pageIdProperty);
    const int pageIdIndex = sheet->indexOf(pageIdPropertyName);
    if (pageIdIndex != -1 && sheet->isChanged(pageIdIndex)) {
        DomProperty *property = variantToDomProperty(this, wizardPage->metaObject(), pageIdPropertyName, sheet->property(pageIdIndex));
        Q_ASSERT(property);
        property->elementString()->setAttributeNotr(u"true"_s);
        DomPropertyList attributes = ui_widget->elementAttribute();
        attributes.push_back(property);
        ui_widget->setElementAttribute(attributes);
    }
    return ui_widget;
}

// Do not save the 'currentTabName' properties of containers
static inline bool checkContainerProperty(const QWidget *w, const QString &propertyName)
{
    if (qobject_cast<const QToolBox *>(w))
        return QToolBoxWidgetPropertySheet::checkProperty(propertyName);
    if (qobject_cast<const QTabWidget *>(w))
        return QTabWidgetPropertySheet::checkProperty(propertyName);
    if (qobject_cast<const QStackedWidget *>(w))
        return QStackedWidgetPropertySheet::checkProperty(propertyName);
    if (qobject_cast<const QMdiArea *>(w))
        return QMdiAreaPropertySheet::checkProperty(propertyName);
    return true;
}

bool QDesignerResource::checkProperty(QObject *obj, const QString &prop) const
{
    const QDesignerMetaObjectInterface *meta = core()->introspection()->metaObject(obj);

    const int pindex = meta->indexOfProperty(prop);
    if (pindex != -1 && !meta->property(pindex)->attributes().testFlag(QDesignerMetaPropertyInterface::StoredAttribute))
        return false;

    if (prop == "objectName"_L1 || prop == "spacerName"_L1)  // ### don't store the property objectName
        return false;

    QWidget *check_widget = nullptr;
    if (obj->isWidgetType())
        check_widget = static_cast<QWidget*>(obj);

    if (check_widget && prop == "geometry"_L1) {
        if (check_widget == m_formWindow->mainContainer())
            return true; // Save although maincontainer is technically laid-out by embedding container
         if (m_selected && m_selected == check_widget)
             return true;

        return !LayoutInfo::isWidgetLaidout(core(), check_widget);
    }

    if (check_widget && !checkContainerProperty(check_widget, prop))
        return false;

    if (QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(core()->extensionManager(), obj)) {
        QDesignerDynamicPropertySheetExtension *dynamicSheet = qt_extension<QDesignerDynamicPropertySheetExtension*>(core()->extensionManager(), obj);
        const int pindex = sheet->indexOf(prop);
        if (sheet->isAttribute(pindex))
            return false;

        if (!dynamicSheet || !dynamicSheet->isDynamicProperty(pindex))
            return sheet->isChanged(pindex);
        if (!sheet->isVisible(pindex))
            return false;
        return true;
    }

    return false;
}

bool QDesignerResource::addItem(DomLayoutItem *ui_item, QLayoutItem *item, QLayout *layout)
{
    if (item->widget() == nullptr) {
        return false;
    }

    QGridLayout *grid = qobject_cast<QGridLayout*>(layout);
    QBoxLayout *box = qobject_cast<QBoxLayout*>(layout);

    if (grid != nullptr) {
        const int rowSpan = ui_item->hasAttributeRowSpan() ? ui_item->attributeRowSpan() : 1;
        const int colSpan = ui_item->hasAttributeColSpan() ? ui_item->attributeColSpan() : 1;
        grid->addWidget(item->widget(), ui_item->attributeRow(), ui_item->attributeColumn(), rowSpan, colSpan, item->alignment());
        return true;
    }
    if (box != nullptr) {
        box->addItem(item);
        return true;
    }

    return QAbstractFormBuilder::addItem(ui_item, item, layout);
}

bool QDesignerResource::addItem(DomWidget *ui_widget, QWidget *widget, QWidget *parentWidget)
{
    core()->metaDataBase()->add(widget); // ensure the widget is in the meta database

    if (! QAbstractFormBuilder::addItem(ui_widget, widget, parentWidget) || qobject_cast<QMainWindow*> (parentWidget)) {
        if (QDesignerContainerExtension *container = qt_extension<QDesignerContainerExtension*>(core()->extensionManager(), parentWidget))
            container->addWidget(widget);
    }

    if (QTabWidget *tabWidget = qobject_cast<QTabWidget*>(parentWidget)) {
        const int tabIndex = tabWidget->count() - 1;
        const int current = tabWidget->currentIndex();

        tabWidget->setCurrentIndex(tabIndex);

        const auto &attributes = ui_widget->elementAttribute();
        QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(core()->extensionManager(), parentWidget);
        if (auto *picon = QFBE::propertyByName(attributes, QFormBuilderStrings::iconAttribute)) {
            QVariant v = resourceBuilder()->loadResource(workingDirectory(), picon);
            sheet->setProperty(sheet->indexOf(u"currentTabIcon"_s), v);
        }
        if (auto *ptext = QFBE::propertyByName(attributes, QFormBuilderStrings::titleAttribute)) {
            QVariant v = textBuilder()->loadText(ptext);
            sheet->setProperty(sheet->indexOf(u"currentTabText"_s), v);
        }
        if (auto *ptext = QFBE::propertyByName(attributes, QFormBuilderStrings::toolTipAttribute)) {
            QVariant v = textBuilder()->loadText(ptext);
            sheet->setProperty(sheet->indexOf(u"currentTabToolTip"_s), v);
        }
        if (auto *ptext = QFBE::propertyByName(attributes, QFormBuilderStrings::whatsThisAttribute)) {
            QVariant v = textBuilder()->loadText(ptext);
            sheet->setProperty(sheet->indexOf(u"currentTabWhatsThis"_s), v);
        }
        tabWidget->setCurrentIndex(current);
    } else if (QToolBox *toolBox = qobject_cast<QToolBox*>(parentWidget)) {
        const int itemIndex = toolBox->count() - 1;
        const int current = toolBox->currentIndex();

        toolBox->setCurrentIndex(itemIndex);

        const auto &attributes = ui_widget->elementAttribute();
        QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(core()->extensionManager(), parentWidget);
        if (auto *picon = QFBE::propertyByName(attributes, QFormBuilderStrings::iconAttribute)) {
            QVariant v = resourceBuilder()->loadResource(workingDirectory(), picon);
            sheet->setProperty(sheet->indexOf(u"currentItemIcon"_s), v);
        }
        if (auto *ptext = QFBE::propertyByName(attributes, QFormBuilderStrings::labelAttribute)) {
            QVariant v = textBuilder()->loadText(ptext);
            sheet->setProperty(sheet->indexOf(u"currentItemText"_s), v);
        }
        if (auto *ptext = QFBE::propertyByName(attributes, QFormBuilderStrings::toolTipAttribute)) {
            QVariant v = textBuilder()->loadText(ptext);
            sheet->setProperty(sheet->indexOf(u"currentItemToolTip"_s), v);
        }
        toolBox->setCurrentIndex(current);
    }

    return true;
}

bool QDesignerResource::copy(QIODevice *dev, const FormBuilderClipboard &selection)
{
    m_copyWidget = true;

    DomUI *ui = copy(selection);

    d->m_laidout.clear();
    m_copyWidget = false;

    if (!ui)
        return false;

    QXmlStreamWriter writer(dev);
    writer.setAutoFormatting(true);
    writer.setAutoFormattingIndent(1);
    writer.writeStartDocument();
    ui->write(writer);
    writer.writeEndDocument();
    delete ui;
    return true;
}

DomUI *QDesignerResource::copy(const FormBuilderClipboard &selection)
{
    if (selection.empty())
        return nullptr;

    m_copyWidget = true;

    DomWidget *ui_widget = new DomWidget();
    ui_widget->setAttributeName(clipboardObjectName);
    bool hasItems = false;
    // Widgets
    if (!selection.m_widgets.isEmpty()) {
        QList<DomWidget *> ui_widget_list;
        for (auto *w : selection.m_widgets) {
            m_selected = w;
            DomWidget *ui_child = createDom(w, ui_widget);
            m_selected = nullptr;
            if (ui_child)
                ui_widget_list.append(ui_child);
        }
        if (!ui_widget_list.isEmpty()) {
            ui_widget->setElementWidget(ui_widget_list);
            hasItems = true;
        }
    }
    // actions
    if (!selection.m_actions.isEmpty()) {
        QList<DomAction *> domActions;
        for (QAction* action : std::as_const(selection.m_actions)) {
            if (DomAction *domAction = createDom(action))
                domActions += domAction;
        }
        if (!domActions.isEmpty()) {
            ui_widget-> setElementAction(domActions);
            hasItems = true;
        }
    }

    d->m_laidout.clear();
    m_copyWidget = false;

    if (!hasItems) {
        delete ui_widget;
        return nullptr;
    }
    // UI
    DomUI *ui = new DomUI();
    ui->setAttributeVersion(currentUiVersion);
    ui->setElementWidget(ui_widget);
    ui->setElementResources(saveResources(m_resourceBuilder->usedQrcFiles()));
    if (DomCustomWidgets *cws = saveCustomWidgets())
        ui->setElementCustomWidgets(cws);
    return ui;
}

FormBuilderClipboard QDesignerResource::paste(DomUI *ui, QWidget *widgetParent, QObject *actionParent)
{
    QDesignerWidgetItemInstaller wii; // Make sure we use QDesignerWidgetItem.
    const int saved = m_isMainWidget;
    m_isMainWidget = false;

    FormBuilderClipboard rc;

    // Widgets
    const DomWidget *topLevel = ui->elementWidget();
    initialize(ui);
    const auto &domWidgets = topLevel->elementWidget();
    if (!domWidgets.isEmpty()) {
        const QPoint offset = m_formWindow->grid();
        for (DomWidget* domWidget : domWidgets) {
            if (QWidget *w = create(domWidget, widgetParent)) {
                w->move(w->pos() + offset);
                // ### change the init properties of w
                rc.m_widgets.append(w);
            }
        }
    }
    const auto domActions = topLevel->elementAction();
    for (DomAction *domAction : domActions) {
        if (QAction *a = create(domAction, actionParent))
            rc.m_actions .append(a);
    }

    m_isMainWidget = saved;

    if (QDesignerExtraInfoExtension *extra = qt_extension<QDesignerExtraInfoExtension*>(core()->extensionManager(), core()))
        extra->loadUiExtraInfo(ui);

    createResources(ui->elementResources());

    return rc;
}

FormBuilderClipboard QDesignerResource::paste(QIODevice *dev, QWidget *widgetParent, QObject *actionParent)
{
    DomUI ui;
    QXmlStreamReader reader(dev);
    bool uiInitialized = false;

    while (!reader.atEnd()) {
        if (reader.readNext() == QXmlStreamReader::StartElement) {
            if (reader.name().compare("ui"_L1, Qt::CaseInsensitive)) {
                ui.read(reader);
                uiInitialized = true;
            } else {
                //: Parsing clipboard contents
                reader.raiseError(QCoreApplication::translate("QDesignerResource", "Unexpected element <%1>").arg(reader.name().toString()));
            }
        }
    }
    if (reader.hasError()) {
        //: Parsing clipboard contents
        designerWarning(QCoreApplication::translate("QDesignerResource", "Error while pasting clipboard contents at line %1, column %2: %3")
                                    .arg(reader.lineNumber()).arg(reader.columnNumber())
                                    .arg(reader.errorString()));
        uiInitialized = false;
    } else if (!uiInitialized) {
        //: Parsing clipboard contents
        designerWarning(QCoreApplication::translate("QDesignerResource", "Error while pasting clipboard contents: The root element <ui> is missing."));
    }

    if (!uiInitialized)
        return FormBuilderClipboard();

    FormBuilderClipboard clipBoard = paste(&ui, widgetParent, actionParent);

    return clipBoard;
}

void QDesignerResource::layoutInfo(DomLayout *layout, QObject *parent, int *margin, int *spacing)
{
    QAbstractFormBuilder::layoutInfo(layout, parent, margin, spacing);
}

DomCustomWidgets *QDesignerResource::saveCustomWidgets()
{
    if (m_usedCustomWidgets.isEmpty())
        return nullptr;

    // We would like the list to be in order of the widget database indexes
    // to ensure that base classes come first (nice optics)
    QDesignerFormEditorInterface *core = m_formWindow->core();
    QDesignerWidgetDataBaseInterface *db = core->widgetDataBase();
    const bool isInternalWidgetDataBase = qobject_cast<const WidgetDataBase *>(db);
    QMap<int, DomCustomWidget *> orderedMap;

    for (auto it = m_usedCustomWidgets.cbegin(), end = m_usedCustomWidgets.cend(); it != end; ++it) {
        QDesignerWidgetDataBaseItemInterface *item = it.key();
        const QString name = item->name();
        DomCustomWidget *custom_widget = new DomCustomWidget;

        custom_widget->setElementClass(name);
        if (item->isContainer())
            custom_widget->setElementContainer(item->isContainer());

        if (!item->includeFile().isEmpty()) {
            DomHeader *header = new DomHeader;
            const  IncludeSpecification spec = includeSpecification(item->includeFile());
            header->setText(spec.first);
            if (spec.second == IncludeGlobal) {
                header->setAttributeLocation(u"global"_s);
            }
            custom_widget->setElementHeader(header);
            custom_widget->setElementExtends(item->extends());
        }

        if (isInternalWidgetDataBase) {
            WidgetDataBaseItem *internalItem = static_cast<WidgetDataBaseItem *>(item);
            const QStringList fakeSlots = internalItem->fakeSlots();
            const QStringList fakeSignals = internalItem->fakeSignals();
            if (!fakeSlots.isEmpty() || !fakeSignals.isEmpty()) {
                DomSlots *domSlots = new DomSlots();
                domSlots->setElementSlot(fakeSlots);
                domSlots->setElementSignal(fakeSignals);
                custom_widget->setElementSlots(domSlots);
            }
            const QString addPageMethod = internalItem->addPageMethod();
            if (!addPageMethod.isEmpty())
                custom_widget->setElementAddPageMethod(addPageMethod);
        }

        orderedMap.insert(db->indexOfClassName(name), custom_widget);
    }

    DomCustomWidgets *customWidgets = new DomCustomWidgets;
    customWidgets->setElementCustomWidget(orderedMap.values().toVector());
    return customWidgets;
}

bool QDesignerResource::canCompressSpacings(QObject *object) const
{
    if (QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(core()->extensionManager(), object)) {
        if (qobject_cast<QGridLayout *>(object)) {
            const int h = sheet->property(sheet->indexOf(u"horizontalSpacing"_s)).toInt();
            const int v = sheet->property(sheet->indexOf(u"verticalSpacing"_s)).toInt();
            if (h == v)
                return true;
        }
    }
    return false;
}

QList<DomProperty*> QDesignerResource::computeProperties(QObject *object)
{
    QList<DomProperty*> properties;
    if (QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(core()->extensionManager(), object)) {
        QDesignerDynamicPropertySheetExtension *dynamicSheet = qt_extension<QDesignerDynamicPropertySheetExtension*>(core()->extensionManager(), object);
        const int count = sheet->count();
        QList<DomProperty *> spacingProperties;
        const bool compressSpacings = canCompressSpacings(object);
        for (int index = 0; index < count; ++index) {
            if (!sheet->isChanged(index) && (!dynamicSheet || !dynamicSheet->isDynamicProperty(index)))
                continue;

            const QString propertyName = sheet->propertyName(index);
            // Suppress windowModality in legacy forms that have it set on child widgets
            if (propertyName == "windowModality"_L1 && !sheet->isVisible(index))
                continue;

            const QVariant value = sheet->property(index);
            if (DomProperty *p = createProperty(object, propertyName, value)) {
                if (compressSpacings && (propertyName == "horizontalSpacing"_L1
                    || propertyName == "verticalSpacing"_L1)) {
                    spacingProperties.append(p);
                } else {
                    properties.append(p);
                }
            }
        }
        if (compressSpacings) {
            if (spacingProperties.size() == 2) {
                DomProperty *spacingProperty = spacingProperties.at(0);
                spacingProperty->setAttributeName(u"spacing"_s);
                properties.append(spacingProperty);
                delete spacingProperties.at(1);
            } else {
                properties += spacingProperties;
            }
        }
    }
    return properties;
}

DomProperty *QDesignerResource::applyProperStdSetAttribute(QObject *object, const QString &propertyName, DomProperty *property)
{
    if (!property)
        return nullptr;

    QExtensionManager *mgr = core()->extensionManager();
    if (const QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(mgr, object)) {
        const QDesignerDynamicPropertySheetExtension *dynamicSheet = qt_extension<QDesignerDynamicPropertySheetExtension*>(mgr, object);
        const QDesignerPropertySheet *designerSheet = qobject_cast<QDesignerPropertySheet*>(core()->extensionManager()->extension(object, Q_TYPEID(QDesignerPropertySheetExtension)));
        const int index = sheet->indexOf(propertyName);
        if ((dynamicSheet && dynamicSheet->isDynamicProperty(index)) || (designerSheet && designerSheet->isDefaultDynamicProperty(index)))
            property->setAttributeStdset(0);
    }
    return property;
}

// Optimistic check for a standard setter function
static inline bool hasSetter(QDesignerFormEditorInterface *core, QObject *object, const QString &propertyName)
{
    const QDesignerMetaObjectInterface *meta = core->introspection()->metaObject(object);
    const int pindex = meta->indexOfProperty(propertyName);
    if (pindex == -1)
        return true;
    return  meta->property(pindex)->hasSetter();
}

DomProperty *QDesignerResource::createProperty(QObject *object, const QString &propertyName, const QVariant &value)
{
    if (!checkProperty(object, propertyName)) {
        return nullptr;
    }

    if (value.canConvert<PropertySheetFlagValue>()) {
        const PropertySheetFlagValue f = qvariant_cast<PropertySheetFlagValue>(value);
        const auto mode = d->m_fullyQualifiedEnums
                          ? DesignerMetaFlags::FullyQualified : DesignerMetaFlags::Qualified;
        const QString flagString = f.metaFlags.toString(f.value, mode);
        if (flagString.isEmpty())
            return nullptr;

        DomProperty *p = new DomProperty;
        // check if we have a standard cpp set function
        if (!hasSetter(core(), object, propertyName))
            p->setAttributeStdset(0);
        p->setAttributeName(propertyName);
        p->setElementSet(flagString);
        return applyProperStdSetAttribute(object, propertyName, p);
    }
    if (value.canConvert<PropertySheetEnumValue>()) {
        const PropertySheetEnumValue e = qvariant_cast<PropertySheetEnumValue>(value);
        const auto mode = d->m_fullyQualifiedEnums
                          ? DesignerMetaEnum::FullyQualified : DesignerMetaEnum::Qualified;
        bool ok;
        const QString id = e.metaEnum.toString(e.value, mode, &ok);
        if (!ok)
            designerWarning(e.metaEnum.messageToStringFailed(e.value));
        if (id.isEmpty())
            return nullptr;

        DomProperty *p = new DomProperty;
        // check if we have a standard cpp set function
        if (!hasSetter(core(), object, propertyName))
            p->setAttributeStdset(0);
        p->setAttributeName(propertyName);
        p->setElementEnum(id);
        return applyProperStdSetAttribute(object, propertyName, p);
    }
    if (value.canConvert<PropertySheetStringValue>()) {
        const PropertySheetStringValue strVal = qvariant_cast<PropertySheetStringValue>(value);
        DomProperty *p = stringToDomProperty(strVal.value(), strVal);
        if (!hasSetter(core(), object, propertyName))
            p->setAttributeStdset(0);

        p->setAttributeName(propertyName);

        return applyProperStdSetAttribute(object, propertyName, p);
    }
    if (value.canConvert<PropertySheetStringListValue>()) {
        const PropertySheetStringListValue listValue = qvariant_cast<PropertySheetStringListValue>(value);
        DomProperty *p = new DomProperty;
        if (!hasSetter(core(), object, propertyName))
            p->setAttributeStdset(0);

        p->setAttributeName(propertyName);

        DomStringList *domStringList = new DomStringList();
        domStringList->setElementString(listValue.value());
        translationParametersToDom(listValue, domStringList);
        p->setElementStringList(domStringList);
        return applyProperStdSetAttribute(object, propertyName, p);
    }
    if (value.canConvert<PropertySheetKeySequenceValue>()) {
        const PropertySheetKeySequenceValue keyVal = qvariant_cast<PropertySheetKeySequenceValue>(value);
        DomProperty *p = stringToDomProperty(keyVal.value().toString(), keyVal);
        if (!hasSetter(core(), object, propertyName))
            p->setAttributeStdset(0);

        p->setAttributeName(propertyName);

        return applyProperStdSetAttribute(object, propertyName, p);
    }

    return applyProperStdSetAttribute(object, propertyName, QAbstractFormBuilder::createProperty(object, propertyName, value));
}

QStringList QDesignerResource::mergeWithLoadedPaths(const QStringList &paths) const
{
    QStringList newPaths = paths;
#ifdef OLD_RESOURCE_FORMAT
    const QStringList loadedPaths = m_resourceBuilder->loadedQrcFiles();
    std::remove_copy_if(loadedPaths.cbegin(), loadedPaths.cend(),
                        std::back_inserter(newPaths),
                        [&newPaths] (const QString &path) { return newPaths.contains(path); });
#endif
    return newPaths;
}


void QDesignerResource::createResources(DomResources *resources)
{
    QStringList paths;
    if (resources != nullptr) {
        const auto &dom_include = resources->elementInclude();
        for (DomResource *res : dom_include) {
            QString path = QDir::cleanPath(m_formWindow->absoluteDir().absoluteFilePath(res->attributeLocation()));
            while (!QFile::exists(path)) {
                QWidget *dialogParent = m_formWindow->core()->topLevel();
                const QString promptTitle = QCoreApplication::translate("qdesigner_internal::QDesignerResource", "Loading qrc file");
                const QString prompt = QCoreApplication::translate("qdesigner_internal::QDesignerResource", "The specified qrc file <p><b>%1</b></p><p>could not be found. Do you want to update the file location?</p>").arg(path);

                const QMessageBox::StandardButton answer = core()->dialogGui()->message(dialogParent,  QDesignerDialogGuiInterface::ResourceLoadFailureMessage,
                        QMessageBox::Warning, promptTitle,  prompt, QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes);
                if (answer == QMessageBox::Yes) {
                    const QFileInfo fi(path);
                    const QString fileDialogTitle = QCoreApplication::translate("qdesigner_internal::QDesignerResource", "New location for %1").arg(fi.fileName());
                    const QString fileDialogPattern = QCoreApplication::translate("qdesigner_internal::QDesignerResource", "Resource files (*.qrc)");
                    path = core()->dialogGui()->getOpenFileName(dialogParent, fileDialogTitle, fi.absolutePath(), fileDialogPattern);
                    if (path.isEmpty())
                        break;
                    m_formWindow->setProperty("_q_resourcepathchanged", QVariant(true));
                } else {
                    break;
                }
            }
            if (!path.isEmpty()) {
                paths << path;
                m_formWindow->addResourceFile(path);
            }
        }
    }

#ifdef OLD_RESOURCE_FORMAT
    paths = mergeWithLoadedPaths(paths);
#endif

    QtResourceSet *resourceSet = m_formWindow->resourceSet();
    if (resourceSet) {
        QStringList newPaths = resourceSet->activeResourceFilePaths();
        std::remove_copy_if(paths.cbegin(), paths.cend(),
                            std::back_inserter(newPaths),
                            [&newPaths] (const QString &path) { return newPaths.contains(path); });
        resourceSet->activateResourceFilePaths(newPaths);
    } else {
        resourceSet = m_formWindow->core()->resourceModel()->addResourceSet(paths);
        m_formWindow->setResourceSet(resourceSet);
        QObject::connect(m_formWindow->core()->resourceModel(), &QtResourceModel::resourceSetActivated,
                m_formWindow, &FormWindowBase::resourceSetActivated);
    }
}

DomResources *QDesignerResource::saveResources()
{
    QStringList paths;
    switch (m_formWindow->resourceFileSaveMode()) {
    case QDesignerFormWindowInterface::SaveAllResourceFiles:
        paths = m_formWindow->activeResourceFilePaths();
        break;
    case QDesignerFormWindowInterface::SaveOnlyUsedResourceFiles:
        paths = m_resourceBuilder->usedQrcFiles();
        break;
    case QDesignerFormWindowInterface::DontSaveResourceFiles:
        break;
    }
    return saveResources(paths);
}

DomResources *QDesignerResource::saveResources(const QStringList &qrcPaths)
{
    QtResourceSet *resourceSet = m_formWindow->resourceSet();
    QList<DomResource *> dom_include;
    if (resourceSet) {
        const QStringList activePaths = resourceSet->activeResourceFilePaths();
        for (const QString &path : activePaths) {
            if (qrcPaths.contains(path)) {
                DomResource *dom_res = new DomResource;
                QString conv_path = path;
                if (m_resourceBuilder->isSaveRelative())
                    conv_path = m_formWindow->absoluteDir().relativeFilePath(path);
                conv_path.replace(QDir::separator(), u'/');
                dom_res->setAttributeLocation(conv_path);
                dom_include.append(dom_res);
            }
        }
    }

    DomResources *dom_resources = new DomResources;
    dom_resources->setElementInclude(dom_include);

    return dom_resources;
}

DomAction *QDesignerResource::createDom(QAction *action)
{
    if (!core()->metaDataBase()->item(action) || action->menu())
        return nullptr;

    return QAbstractFormBuilder::createDom(action);
}

DomActionGroup *QDesignerResource::createDom(QActionGroup *actionGroup)
{
    if (core()->metaDataBase()->item(actionGroup) != nullptr) {
        return QAbstractFormBuilder::createDom(actionGroup);
    }

    return nullptr;
}

QAction *QDesignerResource::create(DomAction *ui_action, QObject *parent)
{
    if (QAction *action = QAbstractFormBuilder::create(ui_action, parent)) {
        core()->metaDataBase()->add(action);
        return action;
    }

    return nullptr;
}

QActionGroup *QDesignerResource::create(DomActionGroup *ui_action_group, QObject *parent)
{
    if (QActionGroup *actionGroup = QAbstractFormBuilder::create(ui_action_group, parent)) {
        core()->metaDataBase()->add(actionGroup);
        return actionGroup;
    }

    return nullptr;
}

DomActionRef *QDesignerResource::createActionRefDom(QAction *action)
{
    if (!core()->metaDataBase()->item(action)
            || (!action->isSeparator() && !action->menu() && action->objectName().isEmpty()))
        return nullptr;

    return QAbstractFormBuilder::createActionRefDom(action);
}

void QDesignerResource::addMenuAction(QAction *action)
{
    core()->metaDataBase()->add(action);
}

QAction *QDesignerResource::createAction(QObject *parent, const QString &name)
{
    if (QAction *action = QAbstractFormBuilder::createAction(parent, name)) {
        core()->metaDataBase()->add(action);
        return action;
    }

    return nullptr;
}

QActionGroup *QDesignerResource::createActionGroup(QObject *parent, const QString &name)
{
    if (QActionGroup *actionGroup = QAbstractFormBuilder::createActionGroup(parent, name)) {
        core()->metaDataBase()->add(actionGroup);
        return actionGroup;
    }

    return nullptr;
}

/* Apply the attributes to a widget via property sheet where appropriate,
 * that is, the sheet handles attributive fake properties */
void QDesignerResource::applyAttributesToPropertySheet(const DomWidget *ui_widget, QWidget *widget)
{
    const DomPropertyList attributes = ui_widget->elementAttribute();
    if (attributes.isEmpty())
        return;
    QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(m_formWindow->core()->extensionManager(), widget);
    for (auto *prop : attributes) {
        const QString name = prop->attributeName();
        const int index = sheet->indexOf(name);
        if (index == -1) {
            const QString msg = "Unable to apply attributive property '%1' to '%2'. It does not exist."_L1.arg(name, widget->objectName());
            designerWarning(msg);
        } else {
            sheet->setProperty(index, domPropertyToVariant(this, widget->metaObject(), prop));
            sheet->setChanged(index, true);
        }
    }
}

void QDesignerResource::loadExtraInfo(DomWidget *ui_widget, QWidget *widget, QWidget *parentWidget)
{
    QAbstractFormBuilder::loadExtraInfo(ui_widget, widget, parentWidget);
    // Apply the page id attribute of a QWizardPage (which is an  attributive fake property)
    if (qobject_cast<const QWizardPage*>(widget))
        applyAttributesToPropertySheet(ui_widget, widget);
}

}

QT_END_NAMESPACE
