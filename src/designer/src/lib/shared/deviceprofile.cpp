// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "deviceprofile_p.h"

#include <QtDesigner/abstractformeditor.h>
#include <widgetfactory_p.h>
#include <qdesigner_utils_p.h>

#include <QtWidgets/qapplication.h>
#include <QtGui/qfont.h>
#include <QtWidgets/qstyle.h>
#include <QtWidgets/qstylefactory.h>
#include <QtWidgets/qapplication.h>

#include <QtGui/qscreen.h>

#include <QtCore/qshareddata.h>
#include <QtCore/qtextstream.h>

#include <QtCore/qxmlstream.h>


static const char dpiXPropertyC[] = "_q_customDpiX";
static const char dpiYPropertyC[] = "_q_customDpiY";

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

namespace qdesigner_internal {

// XML serialization
static const char *xmlVersionC="1.0";
static const char *rootElementC="deviceprofile";
static constexpr auto nameElementC = "name"_L1;
static constexpr auto fontFamilyElementC = "fontfamily"_L1;
static constexpr auto fontPointSizeElementC = "fontpointsize"_L1;
static constexpr auto dPIXElementC = "dpix"_L1;
static constexpr auto dPIYElementC = "dpiy"_L1;
static constexpr auto styleElementC = "style"_L1;

/* DeviceProfile:
 * For preview purposes (preview, widget box, new form dialog), the
 * QDesignerFormBuilder applies the settings to the form main container
 * (Point being here that the DPI must be set directly for size calculations
 * to be correct).
 * For editing purposes, FormWindow applies the settings to the form container
 * as not to interfere with the font settings of the form main container.
 * In addition, the widgetfactory maintains the system settings style
 * and applies it when creating widgets. */

// ---------------- DeviceProfileData
class DeviceProfileData : public QSharedData {
public:
    DeviceProfileData() = default;
    void fromSystem();
    void clear();

    QString m_fontFamily;
    QString m_style;
    QString m_name;
    int m_fontPointSize = -1;
    int m_dpiX = -1;
    int m_dpiY = -1;
};

void DeviceProfileData::clear()
{
    m_fontPointSize = -1;
    m_dpiX = 0;
    m_dpiY = 0;
    m_name.clear();
    m_style.clear();
}

void DeviceProfileData::fromSystem()
{
    const QFont appFont = QApplication::font();
    m_fontFamily = appFont.family();
    m_fontPointSize = appFont.pointSize();
    DeviceProfile::systemResolution(&m_dpiX, &m_dpiY);
    m_style.clear();
}

// ---------------- DeviceProfile
DeviceProfile::DeviceProfile() :
   m_d(new DeviceProfileData)
{
}

DeviceProfile::DeviceProfile(const DeviceProfile &o) :
    m_d(o.m_d)

{
}

DeviceProfile& DeviceProfile::operator=(const DeviceProfile &o)
{
    m_d.operator=(o.m_d);
    return *this;
}

DeviceProfile::~DeviceProfile() = default;

void DeviceProfile::clear()
{
    m_d->clear();
}

bool DeviceProfile::isEmpty() const
{
    return m_d->m_name.isEmpty();
}

QString DeviceProfile::fontFamily() const
{
     return m_d->m_fontFamily;
}

void DeviceProfile::setFontFamily(const QString &f)
{
     m_d->m_fontFamily = f;
}

int DeviceProfile::fontPointSize() const
{
     return m_d->m_fontPointSize;
}

void DeviceProfile::setFontPointSize(int p)
{
     m_d->m_fontPointSize = p;
}

QString DeviceProfile::style() const
{
    return m_d->m_style;
}

void DeviceProfile::setStyle(const QString &s)
{
    m_d->m_style = s;
}

int DeviceProfile::dpiX() const
{
     return m_d->m_dpiX;
}

void DeviceProfile::setDpiX(int d)
{
     m_d->m_dpiX = d;
}

int DeviceProfile::dpiY() const
{
     return m_d->m_dpiY;
}

void DeviceProfile::setDpiY(int d)
{
     m_d->m_dpiY = d;
}

void DeviceProfile::fromSystem()
{
    m_d->fromSystem();
}

QString DeviceProfile::name() const
{
    return m_d->m_name;
}

void DeviceProfile::setName(const QString &n)
{
    m_d->m_name = n;
}

void DeviceProfile::systemResolution(int *dpiX, int *dpiY)
{
    auto s = qApp->primaryScreen();
    *dpiX = s->logicalDotsPerInchX();
    *dpiY = s->logicalDotsPerInchY();
}

class FriendlyWidget : public QWidget {
    friend class DeviceProfile;
};

void DeviceProfile::widgetResolution(const QWidget *w, int *dpiX, int *dpiY)
{
    const FriendlyWidget *fw = static_cast<const FriendlyWidget*>(w);
    *dpiX = fw->metric(QPaintDevice::PdmDpiX);
    *dpiY = fw->metric(QPaintDevice::PdmDpiY);
}

QString DeviceProfile::toString() const
{
    const DeviceProfileData &d = *m_d;
    QString rc;
    QTextStream(&rc) << "DeviceProfile:name=" << d.m_name << " Font=" << d.m_fontFamily << ' '
        << d.m_fontPointSize << " Style=" << d.m_style << " DPI=" << d.m_dpiX << ',' << d.m_dpiY;
    return rc;
}

// Apply font to widget
static void applyFont(const QString &family, int size, DeviceProfile::ApplyMode am, QWidget *widget)
{
    QFont currentFont = widget->font();
    if (currentFont.pointSize() == size && currentFont.family() == family)
        return;
    switch (am) {
    case DeviceProfile::ApplyFormParent:
        // Invisible form parent: Apply all
        widget->setFont(QFont(family, size));
        break;
    case DeviceProfile::ApplyPreview: {
        // Preview: Apply only subproperties that have not been changed by designer properties
        bool apply = false;
        const uint resolve = currentFont.resolveMask();
        if (!(resolve & QFont::FamilyResolved)) {
            currentFont.setFamily(family);
            apply = true;
        }
        if (!(resolve & QFont::SizeResolved)) {
            currentFont.setPointSize(size);
            apply = true;
        }
        if (apply)
            widget->setFont(currentFont);
    }
        break;
    }
}

void DeviceProfile::applyDPI(int dpiX, int dpiY, QWidget *widget)
{
    int sysDPIX, sysDPIY; // Set dynamic variables in case values are different from system DPI
    systemResolution(&sysDPIX, &sysDPIY);
    if (dpiX != sysDPIX && dpiY != sysDPIY) {
        widget->setProperty(dpiXPropertyC, QVariant(dpiX));
        widget->setProperty(dpiYPropertyC, QVariant(dpiY));
    }
}

void DeviceProfile::apply(const QDesignerFormEditorInterface *core, QWidget *widget, ApplyMode am) const
{
    if (isEmpty())
        return;

    const DeviceProfileData &d = *m_d;

    if (!d.m_fontFamily.isEmpty())
        applyFont(d.m_fontFamily, d.m_fontPointSize, am, widget);

    applyDPI(d.m_dpiX, d.m_dpiY, widget);

    if (!d.m_style.isEmpty()) {
        if (WidgetFactory *wf = qobject_cast<qdesigner_internal::WidgetFactory *>(core->widgetFactory()))
            wf->applyStyleTopLevel(d.m_style, widget);
    }
}

bool comparesEqual(const DeviceProfile &lhs, const DeviceProfile &rhs) noexcept
{
    const DeviceProfileData &d = *lhs.m_d;
    const DeviceProfileData &rhs_d = *rhs.m_d;
    return d.m_fontPointSize == rhs_d.m_fontPointSize &&
           d.m_dpiX == rhs_d.m_dpiX && d.m_dpiY == rhs_d.m_dpiY && d.m_fontFamily == rhs_d.m_fontFamily &&
           d.m_style == rhs_d.m_style && d.m_name == rhs_d.m_name;
}

static inline void writeElement(QXmlStreamWriter &writer, const QString &element, const QString &cdata)
{
    writer.writeStartElement(element);
    writer.writeCharacters(cdata);
    writer.writeEndElement();
}

QString DeviceProfile::toXml() const
{
    const DeviceProfileData &d = *m_d;
    QString rc;
    QXmlStreamWriter writer(&rc);
    writer.writeStartDocument(QLatin1StringView(xmlVersionC));
    writer.writeStartElement(QLatin1StringView(rootElementC));
    writeElement(writer, nameElementC, d.m_name);

    if (!d.m_fontFamily.isEmpty())
        writeElement(writer, fontFamilyElementC, d.m_fontFamily);
    if (d.m_fontPointSize >= 0)
        writeElement(writer, fontPointSizeElementC, QString::number(d.m_fontPointSize));
    if (d.m_dpiX > 0)
        writeElement(writer, dPIXElementC, QString::number(d.m_dpiX));
    if (d.m_dpiY > 0)
        writeElement(writer, dPIYElementC, QString::number(d.m_dpiY));
    if (!d.m_style.isEmpty())
        writeElement(writer, styleElementC, d.m_style);

    writer.writeEndElement();
    writer.writeEndDocument();
    return rc;
}

/* Switch stages when encountering a start element (state table) */
enum ParseStage { ParseBeginning, ParseWithinRoot,
                  ParseName, ParseFontFamily, ParseFontPointSize, ParseDPIX,  ParseDPIY,  ParseStyle,
                  ParseError };

static ParseStage nextStage(ParseStage currentStage, QStringView startElement)
{
    switch (currentStage) {
    case ParseBeginning:
        if (startElement == QLatin1StringView(rootElementC))
            return ParseWithinRoot;
        break;
    case ParseWithinRoot:
    case ParseName:
    case ParseFontFamily:
    case ParseFontPointSize:
    case ParseDPIX:
    case ParseDPIY:
    case ParseStyle:
        if (startElement == nameElementC)
            return ParseName;
        if (startElement == fontFamilyElementC)
            return ParseFontFamily;
        if (startElement == fontPointSizeElementC)
            return ParseFontPointSize;
        if (startElement == dPIXElementC)
            return ParseDPIX;
        if (startElement == dPIYElementC)
            return ParseDPIY;
        if (startElement == styleElementC)
            return ParseStyle;
        break;
    case ParseError:
        break;
    }
    return ParseError;
}

static bool readIntegerElement(QXmlStreamReader &reader, int *v)
{
    const QString e = reader.readElementText();
    bool ok;
    *v = e.toInt(&ok);
    //: Reading a number for an embedded device profile
    if (!ok)
        reader.raiseError(QApplication::translate("DeviceProfile", "'%1' is not a number.").arg(e));
    return ok;
}

bool DeviceProfile::fromXml(const QString &xml, QString *errorMessage)
{
    DeviceProfileData &d = *m_d;
    d.fromSystem();

    QXmlStreamReader reader(xml);

    ParseStage ps = ParseBeginning;
    QXmlStreamReader::TokenType tt = QXmlStreamReader::NoToken;
    int iv = 0;
    do {
        tt = reader.readNext();
        if (tt == QXmlStreamReader::StartElement) {
            ps = nextStage(ps, reader.name());
            switch (ps) {
            case ParseBeginning:
            case ParseWithinRoot:
                break;
            case ParseError:
                reader.raiseError(QApplication::translate("DeviceProfile", "An invalid tag <%1> was encountered.").arg(reader.name().toString()));
                tt = QXmlStreamReader::Invalid;
                break;
            case ParseName:
                d.m_name = reader.readElementText();
                break;
            case ParseFontFamily:
                d.m_fontFamily = reader.readElementText();
                break;
            case ParseFontPointSize:
                if (readIntegerElement(reader, &iv)) {
                    d.m_fontPointSize = iv;
                } else {
                    tt = QXmlStreamReader::Invalid;
                }
                break;
            case ParseDPIX:
                if (readIntegerElement(reader, &iv)) {
                    d.m_dpiX = iv;
                } else {
                    tt = QXmlStreamReader::Invalid;
                }
                break;
            case ParseDPIY:
                if (readIntegerElement(reader, &iv)) {
                    d.m_dpiY = iv;
                } else {
                    tt = QXmlStreamReader::Invalid;
                }
                break;
            case ParseStyle:
                d.m_style = reader.readElementText();
                break;
            }
        }
    } while (tt != QXmlStreamReader::Invalid && tt != QXmlStreamReader::EndDocument);

    if (reader.hasError()) {
        *errorMessage = reader.errorString();
        return false;
    }

    return true;
}
}

QT_END_NAMESPACE

