// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qtgradientutils_p.h"
#include "qtgradientmanager_p.h"
#include <QtGui/QLinearGradient>
#include <QtGui/QRadialGradient>
#include <QtGui/QConicalGradient>
#include <QtXml/QDomDocument>
#include <QtCore/QDebug>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

static QString gradientTypeToString(QGradient::Type type)
{
    if (type == QGradient::LinearGradient)
        return "LinearGradient"_L1;
    if (type == QGradient::RadialGradient)
        return "RadialGradient"_L1;
    if (type == QGradient::ConicalGradient)
        return "ConicalGradient"_L1;
    return "NoGradient"_L1;
}

static QGradient::Type stringToGradientType(const QString &name)
{
    if (name == "LinearGradient"_L1)
        return QGradient::LinearGradient;
    if (name == "RadialGradient"_L1)
        return QGradient::RadialGradient;
    if (name == "ConicalGradient"_L1)
        return QGradient::ConicalGradient;
    return QGradient::NoGradient;
}

static QString gradientSpreadToString(QGradient::Spread spread)
{
    if (spread == QGradient::PadSpread)
        return "PadSpread"_L1;
    if (spread == QGradient::RepeatSpread)
        return "RepeatSpread"_L1;
    if (spread == QGradient::ReflectSpread)
        return "ReflectSpread"_L1;
    return "PadSpread"_L1;
}

static QGradient::Spread stringToGradientSpread(const QString &name)
{
    if (name == "PadSpread"_L1)
        return QGradient::PadSpread;
    if (name == "RepeatSpread"_L1)
        return QGradient::RepeatSpread;
    if (name == "ReflectSpread"_L1)
        return QGradient::ReflectSpread;
    return QGradient::PadSpread;
}

static QString gradientCoordinateModeToString(QGradient::CoordinateMode mode)
{
    if (mode == QGradient::LogicalMode)
        return "LogicalMode"_L1;
    if (mode == QGradient::StretchToDeviceMode)
        return "StretchToDeviceMode"_L1;
    if (mode == QGradient::ObjectBoundingMode)
        return "ObjectBoundingMode"_L1;
    return "StretchToDeviceMode"_L1;
}

static QGradient::CoordinateMode stringToGradientCoordinateMode(const QString &name)
{
    if (name == "LogicalMode"_L1)
        return QGradient::LogicalMode;
    if (name == "StretchToDeviceMode"_L1)
        return QGradient::StretchToDeviceMode;
    if (name == "ObjectBoundingMode"_L1)
        return QGradient::ObjectBoundingMode;
    return QGradient::StretchToDeviceMode;
}

static QDomElement saveColor(QDomDocument &doc, QColor color)
{
    QDomElement colorElem = doc.createElement("colorData"_L1);

    colorElem.setAttribute("r"_L1, QString::number(color.red()));
    colorElem.setAttribute("g"_L1, QString::number(color.green()));
    colorElem.setAttribute("b"_L1, QString::number(color.blue()));
    colorElem.setAttribute("a"_L1, QString::number(color.alpha()));

    return colorElem;
}

static QDomElement saveGradientStop(QDomDocument &doc, const QGradientStop &stop)
{
    QDomElement stopElem = doc.createElement("stopData"_L1);

    stopElem.setAttribute("position"_L1, QString::number(stop.first));

    const QDomElement colorElem = saveColor(doc, stop.second);
    stopElem.appendChild(colorElem);

    return stopElem;
}

static QDomElement saveGradient(QDomDocument &doc, const QGradient &gradient)
{
    QDomElement gradElem = doc.createElement("gradientData"_L1);

    const QGradient::Type type = gradient.type();
    gradElem.setAttribute("type"_L1, gradientTypeToString(type));
    gradElem.setAttribute("spread"_L1, gradientSpreadToString(gradient.spread()));
    gradElem.setAttribute("coordinateMode"_L1, gradientCoordinateModeToString(gradient.coordinateMode()));

    const QGradientStops stops = gradient.stops();
    for (const QGradientStop &stop : stops)
        gradElem.appendChild(saveGradientStop(doc, stop));

    if (type == QGradient::LinearGradient) {
        const QLinearGradient &g = *static_cast<const QLinearGradient *>(&gradient);
        gradElem.setAttribute("startX"_L1, QString::number(g.start().x()));
        gradElem.setAttribute("startY"_L1, QString::number(g.start().y()));
        gradElem.setAttribute("endX"_L1, QString::number(g.finalStop().x()));
        gradElem.setAttribute("endY"_L1, QString::number(g.finalStop().y()));
    } else if (type == QGradient::RadialGradient) {
        const QRadialGradient &g = *static_cast<const QRadialGradient *>(&gradient);
        gradElem.setAttribute("centerX"_L1, QString::number(g.center().x()));
        gradElem.setAttribute("centerY"_L1, QString::number(g.center().y()));
        gradElem.setAttribute("focalX"_L1, QString::number(g.focalPoint().x()));
        gradElem.setAttribute("focalY"_L1, QString::number(g.focalPoint().y()));
        gradElem.setAttribute("radius"_L1, QString::number(g.radius()));
    } else if (type == QGradient::ConicalGradient) {
        const QConicalGradient &g = *static_cast<const QConicalGradient*>(&gradient);
        gradElem.setAttribute("centerX"_L1, QString::number(g.center().x()));
        gradElem.setAttribute("centerY"_L1, QString::number(g.center().y()));
        gradElem.setAttribute("angle"_L1, QString::number(g.angle()));
    }

    return gradElem;
}

static QColor loadColor(const QDomElement &elem)
{
    if (elem.tagName() != "colorData"_L1)
        return QColor();

    return QColor(elem.attribute("r"_L1).toInt(),
            elem.attribute("g"_L1).toInt(),
            elem.attribute("b"_L1).toInt(),
            elem.attribute("a"_L1).toInt());
}

static QGradientStop loadGradientStop(const QDomElement &elem)
{
    if (elem.tagName() != "stopData"_L1)
        return QGradientStop();

    const qreal pos = static_cast<qreal>(elem.attribute("position"_L1).toDouble());
    return std::make_pair(pos, loadColor(elem.firstChild().toElement()));
}

static QGradient loadGradient(const QDomElement &elem)
{
    if (elem.tagName() != "gradientData"_L1)
        return QLinearGradient();

    const QGradient::Type type = stringToGradientType(elem.attribute("type"_L1));
    const QGradient::Spread spread = stringToGradientSpread(elem.attribute("spread"_L1));
    const QGradient::CoordinateMode mode = stringToGradientCoordinateMode(elem.attribute("coordinateMode"_L1));

    QGradient gradient = QLinearGradient();

    if (type == QGradient::LinearGradient) {
        QLinearGradient g;
        g.setStart(elem.attribute("startX"_L1).toDouble(), elem.attribute("startY"_L1).toDouble());
        g.setFinalStop(elem.attribute("endX"_L1).toDouble(), elem.attribute("endY"_L1).toDouble());
        gradient = g;
    } else if (type == QGradient::RadialGradient) {
        QRadialGradient g;
        g.setCenter(elem.attribute("centerX"_L1).toDouble(), elem.attribute("centerY"_L1).toDouble());
        g.setFocalPoint(elem.attribute("focalX"_L1).toDouble(), elem.attribute("focalY"_L1).toDouble());
        g.setRadius(elem.attribute("radius"_L1).toDouble());
        gradient = g;
    } else if (type == QGradient::ConicalGradient) {
        QConicalGradient g;
        g.setCenter(elem.attribute("centerX"_L1).toDouble(), elem.attribute("centerY"_L1).toDouble());
        g.setAngle(elem.attribute("angle"_L1).toDouble());
        gradient = g;
    }

    QDomElement stopElem = elem.firstChildElement();
    while (!stopElem.isNull()) {
        QGradientStop stop = loadGradientStop(stopElem);

        gradient.setColorAt(stop.first, stop.second);

        stopElem = stopElem.nextSiblingElement();
    }

    gradient.setSpread(spread);
    gradient.setCoordinateMode(mode);

    return gradient;
}

QString QtGradientUtils::saveState(const QtGradientManager *manager)
{
    QDomDocument doc;

    QDomElement rootElem = doc.createElement("gradients"_L1);

    QMap<QString, QGradient> grads = manager->gradients();
    for (auto itGrad = grads.cbegin(), end = grads.cend(); itGrad != end; ++itGrad) {
        QDomElement idElem = doc.createElement("gradient"_L1);
        idElem.setAttribute("name"_L1, itGrad.key());
        QDomElement gradElem = saveGradient(doc, itGrad.value());
        idElem.appendChild(gradElem);

        rootElem.appendChild(idElem);
    }

    doc.appendChild(rootElem);

    return doc.toString();
}

void QtGradientUtils::restoreState(QtGradientManager *manager, const QString &state)
{
    manager->clear();

    QDomDocument doc;
    doc.setContent(state);

    QDomElement rootElem = doc.documentElement();

    QDomElement gradElem = rootElem.firstChildElement();
    while (!gradElem.isNull()) {
        const QString name = gradElem.attribute("name"_L1);
        const QGradient gradient = loadGradient(gradElem.firstChildElement());

        manager->addGradient(name, gradient);
        gradElem = gradElem.nextSiblingElement();
    }
}

QPixmap QtGradientUtils::gradientPixmap(const QGradient &gradient, QSize size,
                                        bool checkeredBackground)
{
    QImage image(size, QImage::Format_ARGB32);
    QPainter p(&image);
    p.setCompositionMode(QPainter::CompositionMode_Source);

    if (checkeredBackground) {
        int pixSize = 20;
        QPixmap pm(2 * pixSize, 2 * pixSize);

        QPainter pmp(&pm);
        pmp.fillRect(0, 0, pixSize, pixSize, Qt::lightGray);
        pmp.fillRect(pixSize, pixSize, pixSize, pixSize, Qt::lightGray);
        pmp.fillRect(0, pixSize, pixSize, pixSize, Qt::darkGray);
        pmp.fillRect(pixSize, 0, pixSize, pixSize, Qt::darkGray);

        p.setBrushOrigin((size.width() % pixSize + pixSize) / 2, (size.height() % pixSize + pixSize) / 2);
        p.fillRect(0, 0, size.width(), size.height(), pm);
        p.setBrushOrigin(0, 0);
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
    }

    const qreal scaleFactor = 0.999999;
    p.scale(scaleFactor, scaleFactor);
    QGradient grad = gradient;
    grad.setCoordinateMode(QGradient::StretchToDeviceMode);
    p.fillRect(QRect(0, 0, size.width(), size.height()), grad);
    p.drawRect(QRect(0, 0, size.width() - 1, size.height() - 1));

    return QPixmap::fromImage(image);
}

static QString styleSheetFillName(const QGradient &gradient)
{
    QString result;

    switch (gradient.type()) {
        case QGradient::LinearGradient:
            result += "qlineargradient"_L1;
            break;
        case QGradient::RadialGradient:
            result += "qradialgradient"_L1;
            break;
        case QGradient::ConicalGradient:
            result += "qconicalgradient"_L1;
            break;
        default:
            qWarning() << "QtGradientUtils::styleSheetFillName(): gradient type" << gradient.type() << "not supported!";
            break;
    }

    return result;
}

static QStringList styleSheetParameters(const QGradient &gradient)
{
    QStringList result;

    if (gradient.type() != QGradient::ConicalGradient) {
        QString spread;
        switch (gradient.spread()) {
            case QGradient::PadSpread:
                spread = "pad"_L1;
                break;
            case QGradient::ReflectSpread:
                spread = "reflect"_L1;
                break;
            case QGradient::RepeatSpread:
                spread = "repeat"_L1;
                break;
            default:
                qWarning() << "QtGradientUtils::styleSheetParameters(): gradient spread" << gradient.spread() << "not supported!";
                break;
        }
        result << "spread:"_L1 + spread;
    }

    switch (gradient.type()) {
        case QGradient::LinearGradient: {
            const QLinearGradient *linearGradient = static_cast<const QLinearGradient*>(&gradient);
            result << "x1:"_L1 + QString::number(linearGradient->start().x())
                << "y1:"_L1    + QString::number(linearGradient->start().y())
                << "x2:"_L1    + QString::number(linearGradient->finalStop().x())
                << "y2:"_L1    + QString::number(linearGradient->finalStop().y());
            break;
        }
        case QGradient::RadialGradient: {
            const QRadialGradient *radialGradient = static_cast<const QRadialGradient*>(&gradient);
            result << "cx:"_L1  + QString::number(radialGradient->center().x())
                << "cy:"_L1     + QString::number(radialGradient->center().y())
                << "radius:"_L1 + QString::number(radialGradient->radius())
                << "fx:"_L1     + QString::number(radialGradient->focalPoint().x())
                << "fy:"_L1     + QString::number(radialGradient->focalPoint().y());
            break;
        }
        case QGradient::ConicalGradient: {
            const QConicalGradient *conicalGradient = static_cast<const QConicalGradient*>(&gradient);
            result << "cx:"_L1 + QString::number(conicalGradient->center().x())
                << "cy:"_L1    + QString::number(conicalGradient->center().y())
                << "angle:"_L1 + QString::number(conicalGradient->angle());
            break;
        }
        default:
            qWarning() << "QtGradientUtils::styleSheetParameters(): gradient type" << gradient.type() << "not supported!";
            break;
    }

    return result;
}

static QStringList styleSheetStops(const QGradient &gradient)
{
    QStringList result;
    const QGradientStops &stops = gradient.stops();
    for (const QGradientStop &stop : stops) {
        const QColor color = stop.second;

        const QString stopDescription = "stop:"_L1 + QString::number(stop.first) + " rgba("_L1
                + QString::number(color.red()) + ", "_L1
                + QString::number(color.green()) + ", "_L1
                + QString::number(color.blue()) + ", "_L1
                + QString::number(color.alpha()) + QLatin1Char(')');
        result << stopDescription;
    }

    return result;
}

QString QtGradientUtils::styleSheetCode(const QGradient &gradient)
{
    QStringList gradientParameters;
    gradientParameters << styleSheetParameters(gradient) << styleSheetStops(gradient);

    return styleSheetFillName(gradient) + QLatin1Char('(') + gradientParameters.join(", "_L1) + QLatin1Char(')');
}

QT_END_NAMESPACE
