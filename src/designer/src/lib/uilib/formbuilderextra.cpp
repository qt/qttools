// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "formbuilderextra_p.h"
#include "abstractformbuilder.h"
#include "properties_p.h"
#include "resourcebuilder_p.h"
#include "textbuilder_p.h"
#include "ui4_p.h"

#include <QtWidgets/qlabel.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qgridlayout.h>
#if QT_CONFIG(fontcombobox)
#  include <QtWidgets/qfontcombobox.h>
#endif

#include <QtCore/qvariant.h>
#include <QtCore/qdebug.h>
#include <QtCore/qtextstream.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qversionnumber.h>

#include <limits.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

#ifdef QFORMINTERNAL_NAMESPACE
namespace QFormInternal {
#endif

void uiLibWarning(const QString &message) {
    qWarning("Designer: %s", qPrintable(message));
}

QFormBuilderExtra::CustomWidgetData::CustomWidgetData() = default;

QFormBuilderExtra::CustomWidgetData::CustomWidgetData(const DomCustomWidget *dcw) :
    addPageMethod(dcw->elementAddPageMethod()),
    baseClass(dcw->elementExtends()),
    isContainer(dcw->hasElementContainer() && dcw->elementContainer() != 0)
{
}

QFormBuilderExtra::QFormBuilderExtra() :
    m_defaultMargin(INT_MIN),
    m_defaultSpacing(INT_MIN),
    m_language(u"c++"_s)
{
}

QFormBuilderExtra::~QFormBuilderExtra()
{
    clearResourceBuilder();
    clearTextBuilder();
}

void QFormBuilderExtra::clear()
{
    m_buddies.clear();
    m_parentWidget = nullptr;
    m_parentWidgetIsSet = false;
    m_customWidgetDataHash.clear();
    m_buttonGroups.clear();
}

static inline QString msgXmlError(const QXmlStreamReader &reader)
{
    return QCoreApplication::translate("QAbstractFormBuilder",
                                       "An error has occurred while reading the UI file at line %1, column %2: %3")
                                       .arg(reader.lineNumber()).arg(reader.columnNumber())
                                       .arg(reader.errorString());
}

// Read and check the  version and the (optional) language attribute
// of an <ui> element and leave reader positioned at <ui>.
static bool inline readUiAttributes(QXmlStreamReader &reader, const QString &language,
                                    QString *errorMessage)
{
    // Read up to first element
    while (!reader.atEnd()) {
        switch (reader.readNext()) {
        case QXmlStreamReader::Invalid:
            *errorMessage = msgXmlError(reader);
            return false;
        case QXmlStreamReader::StartElement:
            if (reader.name().compare("ui"_L1, Qt::CaseInsensitive) == 0) {
                const QString versionAttribute = u"version"_s;
                const QString languageAttribute = u"language"_s;
                const QXmlStreamAttributes attributes = reader.attributes();
                if (attributes.hasAttribute(versionAttribute)) {
                    const QVersionNumber version =
                        QVersionNumber::fromString(attributes.value(versionAttribute));
                    if (version < QVersionNumber(4)) {
                        *errorMessage =
                            QCoreApplication::translate("QAbstractFormBuilder",
                                                        "This file was created using Designer from Qt-%1 and cannot be read.")
                                                        .arg(attributes.value(versionAttribute));
                        return false;
                    } // version error
                }     // has version
                if (attributes.hasAttribute(languageAttribute)) {
                    // Check on optional language (Jambi)
                    const QString formLanguage = attributes.value(languageAttribute).toString();
                    if (!formLanguage.isEmpty() && formLanguage.compare(language, Qt::CaseInsensitive)) {
                        *errorMessage =
                            QCoreApplication::translate("QAbstractFormBuilder",
                                                        "This file cannot be read because it was created using %1.")
                                                        .arg(formLanguage);
                        return false;
                    } // language error
                }    // has language
                return true;
            }  // <ui> matched
            break;
        default:
            break;
        }
    }
    // No <ui> found.
    *errorMessage = QCoreApplication::translate("QAbstractFormBuilder",
                                                "Invalid UI file: The root element <ui> is missing.");
    return false;
}

DomUI *QFormBuilderExtra::readUi(QIODevice *dev)
{
    QXmlStreamReader reader(dev);
    m_errorString.clear();
    if (!readUiAttributes(reader, m_language, &m_errorString)) {
        uiLibWarning(m_errorString);
        return nullptr;
    }
    DomUI *ui = new DomUI;
    ui->read(reader);
    if (reader.hasError()) {
        m_errorString = msgXmlError(reader);
        uiLibWarning(m_errorString);
        delete ui;
        return nullptr;
    }
    return ui;
}

QString QFormBuilderExtra::msgInvalidUiFile()
{
    return QCoreApplication::translate("QAbstractFormBuilder", "Invalid UI file");
}

bool QFormBuilderExtra::applyPropertyInternally(QObject *o, const QString &propertyName, const QVariant &value)
{
    // Store buddies and apply them later on as the widgets might not exist yet.
    QLabel *label = qobject_cast<QLabel*>(o);
    if (label == nullptr || propertyName != "buddy"_L1)
        return false;

    m_buddies.insert(label, value.toString());
    return true;
}

void QFormBuilderExtra::applyInternalProperties() const
{
    for (auto it = m_buddies.cbegin(), cend = m_buddies.cend(); it != cend; ++it )
        applyBuddy(it.value(), BuddyApplyAll, it.key());
}

bool QFormBuilderExtra::applyBuddy(const QString &buddyName, BuddyMode applyMode, QLabel *label)
{
    if (buddyName.isEmpty()) {
        label->setBuddy(nullptr);
        return false;
    }

    // QTBUG-96693: Because the same form can be instantiated on multiple
    // widgets, when for example using a form for custom widgets, there can
    // exist multiple widgets with the same buddy name in the window. Since
    // the buddy closest to the label is the correct one to use, we search
    // for the buddy bottom-up rather than top-down.
    QWidgetList widgets;
    QWidget *parent = label->parentWidget();
    while (parent && widgets.isEmpty()) {
        widgets = parent->findChildren<QWidget*>(buddyName);
        parent = parent->parentWidget();
    }

    if (widgets.isEmpty()) {
        label->setBuddy(nullptr);
        return false;
    }

    for (auto *w : widgets) {
        if (applyMode == BuddyApplyAll || !w->isHidden()) {
            label->setBuddy(w);
            return true;
        }
    }

    label->setBuddy(nullptr);
    return false;
}

const QPointer<QWidget> &QFormBuilderExtra::parentWidget() const
{
    return m_parentWidget;
}

bool QFormBuilderExtra::parentWidgetIsSet() const
{
    return m_parentWidgetIsSet;
}

void QFormBuilderExtra::setParentWidget(const QPointer<QWidget> &w)
{
    // Parent widget requires special handling of the geometry property.
    m_parentWidget = w;
    m_parentWidgetIsSet = true;
}

void QFormBuilderExtra::storeCustomWidgetData(const QString &className, const DomCustomWidget *d)
{
    if (d)
        m_customWidgetDataHash.insert(className, CustomWidgetData(d));
}

QString QFormBuilderExtra::customWidgetBaseClass(const QString &className) const
{
    const auto it = m_customWidgetDataHash.constFind(className);
    if (it != m_customWidgetDataHash.constEnd())
        return it.value().baseClass;
    return QString();
}

QString QFormBuilderExtra::customWidgetAddPageMethod(const QString &className) const
{
    const auto it = m_customWidgetDataHash.constFind(className);
    if (it != m_customWidgetDataHash.constEnd())
        return it.value().addPageMethod;
    return QString();
}

bool QFormBuilderExtra::isCustomWidgetContainer(const QString &className) const
{
    const auto it = m_customWidgetDataHash.constFind(className);
    if (it != m_customWidgetDataHash.constEnd())
        return it.value().isContainer;
    return false;
}

void QFormBuilderExtra::setProcessingLayoutWidget(bool processing)
{
    m_layoutWidget = processing;
}

 bool QFormBuilderExtra::processingLayoutWidget() const
{
    return m_layoutWidget;
}
void QFormBuilderExtra::setResourceBuilder(QResourceBuilder *builder)
{
    if (m_resourceBuilder == builder)
        return;
    clearResourceBuilder();
    m_resourceBuilder = builder;
}

QResourceBuilder *QFormBuilderExtra::resourceBuilder() const
{
    return m_resourceBuilder;
}

void QFormBuilderExtra::clearResourceBuilder()
{
    if (m_resourceBuilder) {
        delete m_resourceBuilder;
        m_resourceBuilder = nullptr;
    }
}

void QFormBuilderExtra::setTextBuilder(QTextBuilder *builder)
{
    if (m_textBuilder == builder)
        return;
    clearTextBuilder();
    m_textBuilder = builder;
}

QTextBuilder *QFormBuilderExtra::textBuilder() const
{
    return m_textBuilder;
}

void QFormBuilderExtra::clearTextBuilder()
{
    if (m_textBuilder) {
        delete m_textBuilder;
        m_textBuilder = nullptr;
    }
}

void QFormBuilderExtra::registerButtonGroups(const DomButtonGroups *domGroups)
{
    const auto &domGroupList = domGroups->elementButtonGroup();
    for (DomButtonGroup *domGroup : domGroupList)
        m_buttonGroups.insert(domGroup->attributeName(), ButtonGroupEntry(domGroup, nullptr));
}

// Utilities for parsing per-cell integer properties that have setters and
//  getters of the form 'setX(int idx, int value)' and 'x(int index)'
// (converting them to comma-separated string lists and back).
// Used for layout stretch and grid per-row/column properties.

// Format a list of cell-properties of one dimension as a ','-separated list
template <class Layout>
inline QString perCellPropertyToString(const Layout *l, int count, int (Layout::*getter)(int) const)
{
    if (count == 0)
        return QString();
    QString rc;
    {
        QTextStream str(&rc);
        for (int i = 0; i < count; i++) {
            if (i)
                str << ',';
            str << (l->*getter)(i);
        }
    }
    return rc;
}

// Clear the property, set all cells to 0

template <class Layout>
inline void clearPerCellValue(Layout *l, int count, void (Layout::*setter)(int,int), int value = 0)
{
    for (int i = 0; i < count; i++)
        (l->*setter)(i, value);
}

// Parse and set the property from a comma-separated list

template <class Layout>
inline bool parsePerCellProperty(Layout *l, int count, void (Layout::*setter)(int,int), const QString &s, int defaultValue = 0)
{
    if (s.isEmpty()) {
        clearPerCellValue(l, count, setter, defaultValue);
        return true;
    }
    const auto list = QStringView{s}.split(u',');
    if (list.isEmpty()) {
        clearPerCellValue(l, count, setter, defaultValue);
        return true;
    }
    // Apply all values contained in list
    const int ac = qMin(count, list.size());
    bool ok;
    int i = 0;
    for ( ; i < ac; i++) {
        const int value = list.at(i).toInt(&ok);
        if (!ok || value < 0)
            return false;
        (l->*setter)(i, value);
    }
    // Clear rest
    for ( ; i < count; i++)
        (l->*setter)(i, defaultValue);
    return true;
}

// Read and write stretch
static QString msgInvalidStretch(const QString &objectName, const QString &stretch)
{
    //: Parsing layout stretch values
    return QCoreApplication::translate("FormBuilder", "Invalid stretch value for '%1': '%2'").arg(objectName, stretch);
}

void QFormBuilderExtra::getLayoutMargins(const QList<DomProperty*> &properties,
                                         int *left, int *top, int *right, int *bottom)
{
    if (const auto *p = propertyByName(properties, "leftMargin"))
        *left = p->elementNumber();
    if (const auto *p = propertyByName(properties, "topMargin"))
        *top = p->elementNumber();
    if (const auto *p = propertyByName(properties, "rightMargin"))
        *right = p->elementNumber();
    if (const auto *p = propertyByName(properties, "bottomMargin"))
        *bottom = p->elementNumber();
}

bool QFormBuilderExtra::isQFontComboBox(const QWidget *w)
{
#if QT_CONFIG(fontcombobox)
    return qobject_cast<const QFontComboBox*>(w) != nullptr;
#else
    Q_UNUSED(w);
    return false;
#endif
}

QString QFormBuilderExtra::boxLayoutStretch(const QBoxLayout *box)
{
     return perCellPropertyToString(box, box->count(), &QBoxLayout::stretch);
}

bool QFormBuilderExtra::setBoxLayoutStretch(const QString &s, QBoxLayout *box)
{
    const bool rc = parsePerCellProperty(box, box->count(), &QBoxLayout::setStretch, s);
    if (!rc)
        uiLibWarning(msgInvalidStretch(box->objectName(), s));
    return rc;
}

void QFormBuilderExtra::clearBoxLayoutStretch(QBoxLayout *box)
{
    clearPerCellValue(box, box->count(), &QBoxLayout::setStretch);
}

QString QFormBuilderExtra::gridLayoutRowStretch(const QGridLayout *grid)
{
    return perCellPropertyToString(grid, grid->rowCount(), &QGridLayout::rowStretch);
}

bool QFormBuilderExtra::setGridLayoutRowStretch(const QString &s, QGridLayout *grid)
{
    const bool rc = parsePerCellProperty(grid, grid->rowCount(), &QGridLayout::setRowStretch, s);
    if (!rc)
        uiLibWarning(msgInvalidStretch(grid->objectName(), s));
    return rc;
}

void QFormBuilderExtra::clearGridLayoutRowStretch(QGridLayout *grid)
{
    clearPerCellValue(grid, grid->rowCount(), &QGridLayout::setRowStretch);
}

QString QFormBuilderExtra::gridLayoutColumnStretch(const QGridLayout *grid)
{
    return perCellPropertyToString(grid, grid->columnCount(), &QGridLayout::columnStretch);
}

bool QFormBuilderExtra::setGridLayoutColumnStretch(const QString &s, QGridLayout *grid)
{
    const bool rc = parsePerCellProperty(grid, grid->columnCount(), &QGridLayout::setColumnStretch, s);
    if (!rc)
        uiLibWarning(msgInvalidStretch(grid->objectName(), s));
    return rc;
}

void QFormBuilderExtra::clearGridLayoutColumnStretch(QGridLayout *grid)
{
    clearPerCellValue(grid, grid->columnCount(), &QGridLayout::setColumnStretch);
}

// Read and write grid layout row/column size limits

static QString msgInvalidMinimumSize(const QString &objectName, const QString &ms)
{
    //: Parsing grid layout minimum size values
    return QCoreApplication::translate("FormBuilder", "Invalid minimum size for '%1': '%2'").arg(objectName, ms);
}

QString QFormBuilderExtra::gridLayoutRowMinimumHeight(const QGridLayout *grid)
{
    return perCellPropertyToString(grid, grid->rowCount(), &QGridLayout::rowMinimumHeight);
}

bool QFormBuilderExtra::setGridLayoutRowMinimumHeight(const QString &s, QGridLayout *grid)
{
    const bool rc = parsePerCellProperty(grid, grid->rowCount(), &QGridLayout::setRowMinimumHeight, s);
    if (!rc)
        uiLibWarning(msgInvalidMinimumSize(grid->objectName(), s));
    return rc;
}

void QFormBuilderExtra::clearGridLayoutRowMinimumHeight(QGridLayout *grid)
{
     clearPerCellValue(grid, grid->rowCount(), &QGridLayout::setRowMinimumHeight);
}

QString QFormBuilderExtra::gridLayoutColumnMinimumWidth(const QGridLayout *grid)
{
    return perCellPropertyToString(grid, grid->columnCount(), &QGridLayout::columnMinimumWidth);
}

bool QFormBuilderExtra::setGridLayoutColumnMinimumWidth(const QString &s, QGridLayout *grid)
{
    const bool rc = parsePerCellProperty(grid, grid->columnCount(), &QGridLayout::setColumnMinimumWidth, s);
    if (!rc)
        uiLibWarning(msgInvalidMinimumSize(grid->objectName(), s));
    return rc;
}

void QFormBuilderExtra::clearGridLayoutColumnMinimumWidth(QGridLayout *grid)
{
    clearPerCellValue(grid, grid->columnCount(), &QGridLayout::setColumnMinimumWidth);
}

void QFormBuilderExtra::setPixmapProperty(DomProperty *p, const std::pair<QString, QString> &ip)
{
    DomResourcePixmap *pix = new DomResourcePixmap;
    if (!ip.second.isEmpty())
        pix->setAttributeResource(ip.second);

    pix->setText(ip.first);

    p->setAttributeName("pixmap"_L1);
    p->setElementPixmap(pix);
}

void QFormBuilderExtra::setupColorGroup(QPalette *palette, QPalette::ColorGroup colorGroup,
                                        const DomColorGroup *group)
{
    // old format
    const auto &colors = group->elementColor();
    for (int role = 0; role < colors.size(); ++role) {
        const DomColor *color = colors.at(role);
        const QColor c(color->elementRed(), color->elementGreen(), color->elementBlue());
        palette->setColor(colorGroup, QPalette::ColorRole(role), c);
    }

    // new format
    const QMetaEnum colorRole_enum = metaEnum<QAbstractFormBuilderGadget>("colorRole");

    const auto colorRoles = group->elementColorRole();
    for (const DomColorRole *colorRole : colorRoles) {
        if (colorRole->hasAttributeRole()) {
            const int r = colorRole_enum.keyToValue(colorRole->attributeRole().toLatin1());
            if (r != -1) {
                const QBrush br = setupBrush(colorRole->elementBrush());
                palette->setBrush(colorGroup, static_cast<QPalette::ColorRole>(r), br);
            }
        }
    }
}

DomColorGroup *QFormBuilderExtra::saveColorGroup(const QPalette &palette,
                                                 QPalette::ColorGroup colorGroup)
{

    const QMetaEnum colorRole_enum = metaEnum<QAbstractFormBuilderGadget>("colorRole");

    DomColorGroup *group = new DomColorGroup();
    QList<DomColorRole *> colorRoles;

    for (int r = QPalette::WindowText; r < QPalette::NColorRoles; ++r) {
        const auto role = static_cast<QPalette::ColorRole>(r);
        if (palette.isBrushSet(colorGroup, role)) {
            const QBrush &br = palette.brush(colorGroup, role);
            DomColorRole *colorRole = new DomColorRole();
            colorRole->setElementBrush(saveBrush(br));
            colorRole->setAttributeRole(QLatin1StringView(colorRole_enum.valueToKey(role)));
            colorRoles.append(colorRole);
        }
    }

    group->setElementColorRole(colorRoles);
    return group;
}

DomPalette *QFormBuilderExtra::savePalette(const QPalette &palette)
{
    DomPalette *dom = new DomPalette();
    dom->setElementActive(QFormBuilderExtra::saveColorGroup(palette, QPalette::Active));
    dom->setElementInactive(QFormBuilderExtra::saveColorGroup(palette, QPalette::Inactive));
    dom->setElementDisabled(QFormBuilderExtra::saveColorGroup(palette, QPalette::Disabled));

    return dom;
}

QPalette QFormBuilderExtra::loadPalette(const DomPalette *dom)
{
    QPalette palette;

    if (dom->elementActive())
        QFormBuilderExtra::setupColorGroup(&palette, QPalette::Active, dom->elementActive());

    if (dom->elementInactive())
        QFormBuilderExtra::setupColorGroup(&palette, QPalette::Inactive, dom->elementInactive());

    if (dom->elementDisabled())
        QFormBuilderExtra::setupColorGroup(&palette, QPalette::Disabled, dom->elementDisabled());

    palette.setCurrentColorGroup(QPalette::Active);
    return palette;
}

QBrush QFormBuilderExtra::setupBrush(const DomBrush *brush)
{
    QBrush br;
    if (!brush->hasAttributeBrushStyle())
        return br;

    const Qt::BrushStyle style = enumKeyOfObjectToValue<QAbstractFormBuilderGadget, Qt::BrushStyle>("brushStyle",
                                                                                                    brush->attributeBrushStyle().toLatin1().constData());

    if (style == Qt::LinearGradientPattern ||
            style == Qt::RadialGradientPattern ||
            style == Qt::ConicalGradientPattern) {
        const QMetaEnum gradientType_enum = metaEnum<QAbstractFormBuilderGadget>("gradientType");
        const QMetaEnum gradientSpread_enum = metaEnum<QAbstractFormBuilderGadget>("gradientSpread");
        const QMetaEnum gradientCoordinate_enum = metaEnum<QAbstractFormBuilderGadget>("gradientCoordinate");

        const DomGradient *gradient = brush->elementGradient();
        const QGradient::Type type = enumKeyToValue<QGradient::Type>(gradientType_enum, gradient->attributeType().toLatin1());


        QGradient *gr = nullptr;

        if (type == QGradient::LinearGradient) {
            gr = new QLinearGradient(QPointF(gradient->attributeStartX(), gradient->attributeStartY()),
                            QPointF(gradient->attributeEndX(), gradient->attributeEndY()));
        } else if (type == QGradient::RadialGradient) {
            gr = new QRadialGradient(QPointF(gradient->attributeCentralX(), gradient->attributeCentralY()),
                            gradient->attributeRadius(),
                            QPointF(gradient->attributeFocalX(), gradient->attributeFocalY()));
        } else if (type == QGradient::ConicalGradient) {
            gr = new QConicalGradient(QPointF(gradient->attributeCentralX(), gradient->attributeCentralY()),
                            gradient->attributeAngle());
        }
        if (!gr)
            return br;

        const QGradient::Spread spread = enumKeyToValue<QGradient::Spread>(gradientSpread_enum, gradient->attributeSpread().toLatin1());
        gr->setSpread(spread);

        const QGradient::CoordinateMode coord = enumKeyToValue<QGradient::CoordinateMode>(gradientCoordinate_enum, gradient->attributeCoordinateMode().toLatin1());
        gr->setCoordinateMode(coord);

        const auto &stops = gradient->elementGradientStop();
        for (const DomGradientStop *stop : stops) {
            const DomColor *color = stop->elementColor();
            gr->setColorAt(stop->attributePosition(), QColor::fromRgb(color->elementRed(),
                            color->elementGreen(), color->elementBlue(), color->attributeAlpha()));
        }
        br = QBrush(*gr);
        delete gr;
    } else if (style == Qt::TexturePattern) {
        const DomProperty *texture = brush->elementTexture();
        if (texture && texture->kind() == DomProperty::Pixmap) {
            br.setTexture({});
        }
    } else {
        const DomColor *color = brush->elementColor();
        br.setColor(QColor::fromRgb(color->elementRed(),
                            color->elementGreen(), color->elementBlue(), color->attributeAlpha()));
        br.setStyle((Qt::BrushStyle)style);
    }
    return br;
}

DomBrush *QFormBuilderExtra::saveBrush(const QBrush &br)
{
    const QMetaEnum brushStyle_enum = metaEnum<QAbstractFormBuilderGadget>("brushStyle");

    DomBrush *brush = new DomBrush();
    const Qt::BrushStyle style = br.style();
    brush->setAttributeBrushStyle(QLatin1StringView(brushStyle_enum.valueToKey(style)));
    if (style == Qt::LinearGradientPattern ||
                style == Qt::RadialGradientPattern ||
                style == Qt::ConicalGradientPattern) {
        const QMetaEnum gradientType_enum = metaEnum<QAbstractFormBuilderGadget>("gradientType");
        const QMetaEnum gradientSpread_enum = metaEnum<QAbstractFormBuilderGadget>("gradientSpread");
        const QMetaEnum gradientCoordinate_enum = metaEnum<QAbstractFormBuilderGadget>("gradientCoordinate");

        DomGradient *gradient = new DomGradient();
        const QGradient *gr = br.gradient();
        const QGradient::Type type = gr->type();
        gradient->setAttributeType(QLatin1StringView(gradientType_enum.valueToKey(type)));
        gradient->setAttributeSpread(QLatin1StringView(gradientSpread_enum.valueToKey(gr->spread())));
        gradient->setAttributeCoordinateMode(QLatin1StringView(gradientCoordinate_enum.valueToKey(gr->coordinateMode())));
        QList<DomGradientStop *> stops;
        const QGradientStops st = gr->stops();
        for (const QGradientStop &pair : st) {
            DomGradientStop *stop = new DomGradientStop();
            stop->setAttributePosition(pair.first);
            DomColor *color = new DomColor();
            color->setElementRed(pair.second.red());
            color->setElementGreen(pair.second.green());
            color->setElementBlue(pair.second.blue());
            color->setAttributeAlpha(pair.second.alpha());
            stop->setElementColor(color);
            stops.append(stop);
        }
        gradient->setElementGradientStop(stops);
        if (type == QGradient::LinearGradient) {
            auto lgr = static_cast<const QLinearGradient *>(gr);
            gradient->setAttributeStartX(lgr->start().x());
            gradient->setAttributeStartY(lgr->start().y());
            gradient->setAttributeEndX(lgr->finalStop().x());
            gradient->setAttributeEndY(lgr->finalStop().y());
        } else if (type == QGradient::RadialGradient) {
            auto rgr = static_cast<const QRadialGradient *>(gr);
            gradient->setAttributeCentralX(rgr->center().x());
            gradient->setAttributeCentralY(rgr->center().y());
            gradient->setAttributeFocalX(rgr->focalPoint().x());
            gradient->setAttributeFocalY(rgr->focalPoint().y());
            gradient->setAttributeRadius(rgr->radius());
        } else if (type == QGradient::ConicalGradient) {
            auto cgr = static_cast<const QConicalGradient *>(gr);
            gradient->setAttributeCentralX(cgr->center().x());
            gradient->setAttributeCentralY(cgr->center().y());
            gradient->setAttributeAngle(cgr->angle());
        }

        brush->setElementGradient(gradient);
    } else if (style == Qt::TexturePattern) {
        const QPixmap pixmap = br.texture();
        if (!pixmap.isNull()) {
            DomProperty *p = new DomProperty;
            QFormBuilderExtra::setPixmapProperty(p, {});
            brush->setElementTexture(p);
        }
    } else {
        const QColor &c = br.color();
        DomColor *color = new DomColor();
        color->setElementRed(c.red());
        color->setElementGreen(c.green());
        color->setElementBlue(c.blue());
        color->setAttributeAlpha(c.alpha());
        brush->setElementColor(color);
    }
    return brush;
}

DomProperty *QFormBuilderExtra::propertyByName(const QList<DomProperty*> &properties,
                                               QAnyStringView needle)
{
    auto it = std::find_if(properties.cbegin(), properties.cend(),
                           [needle](const DomProperty *p) {
                               return p->attributeName() == needle; });
    return it != properties.cend() ? *it : nullptr;
}

// ------------ QFormBuilderStrings

QFormBuilderStrings::QFormBuilderStrings() :
    itemRoles {
        {Qt::FontRole, "font"_L1},
        {Qt::TextAlignmentRole, "textAlignment"_L1},
        {Qt::BackgroundRole, "background"_L1},
        {Qt::ForegroundRole, "foreground"_L1},
        {Qt::CheckStateRole, "checkState"_L1}
    },
    itemTextRoles { // This must be first for the loop below
        { {Qt::EditRole, Qt::DisplayPropertyRole}, textAttribute},
        { {Qt::ToolTipRole, Qt::ToolTipPropertyRole}, toolTipAttribute},
        { {Qt::StatusTipRole, Qt::StatusTipPropertyRole}, "statusTip"_L1},
        { {Qt::WhatsThisRole, Qt::WhatsThisPropertyRole}, whatsThisAttribute}
    }
{
    for (const RoleNName &it : std::as_const(itemRoles))
        treeItemRoleHash.insert(it.second, it.first);

    // Note: this skips the first item!
    auto it = itemTextRoles.constBegin();
    const auto end = itemTextRoles.constEnd();
    while (++it != end)
        treeItemTextRoleHash.insert(it->second, it->first);
}

const QFormBuilderStrings &QFormBuilderStrings::instance()
{
    static const QFormBuilderStrings rc;
    return rc;
}

#ifdef QFORMINTERNAL_NAMESPACE
} // namespace QFormInternal
#endif

QT_END_NAMESPACE
