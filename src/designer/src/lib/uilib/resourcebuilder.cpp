// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "resourcebuilder_p.h"
#include "ui4_p.h"
#include <QtCore/qvariant.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qdir.h>
#include <QtCore/qdebug.h>
#include <QtGui/qpixmap.h>
#include <QtGui/qicon.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

#ifdef QFORMINTERNAL_NAMESPACE
namespace QFormInternal {
#endif

enum { themeDebug = 0 };

QResourceBuilder::QResourceBuilder() = default;

QResourceBuilder::~QResourceBuilder() = default;

int QResourceBuilder::iconStateFlags(const DomResourceIcon *dpi)
{
    int rc = 0;
    // Fix form files broken by QTBUG-115465
    if (dpi->hasElementNormalOff() && dpi->elementNormalOff()->text() != "."_L1)
        rc |= NormalOff;
    if (dpi->hasElementNormalOn())
        rc |= NormalOn;
    if (dpi->hasElementDisabledOff())
        rc |= DisabledOff;
    if (dpi->hasElementDisabledOn())
        rc |= DisabledOn;
    if (dpi->hasElementActiveOff())
        rc |= ActiveOff;
    if (dpi->hasElementActiveOn())
        rc |= ActiveOn;
    if (dpi->hasElementSelectedOff())
        rc |= SelectedOff;
    if (dpi->hasElementSelectedOn())
        rc |= SelectedOn;
    return rc;
}

QVariant QResourceBuilder::loadResource(const QDir &workingDirectory, const DomProperty *property) const
{
    switch (property->kind()) {
        case DomProperty::Pixmap: {
            const DomResourcePixmap *dpx = property->elementPixmap();
            QPixmap pixmap(QFileInfo(workingDirectory, dpx->text()).absoluteFilePath());
            return QVariant::fromValue(pixmap);
        }
        case DomProperty::IconSet: {
            const DomResourceIcon *dpi = property->elementIconSet();
            if (!dpi->attributeTheme().isEmpty()) {
                const QString theme = dpi->attributeTheme();
                const bool known = QIcon::hasThemeIcon(theme);
                if (themeDebug)
                    qDebug("Theme %s known %d", qPrintable(theme), known);
                if (known)
                    return QVariant::fromValue(QIcon::fromTheme(dpi->attributeTheme()));
            } // non-empty theme
            if (const int flags = iconStateFlags(dpi)) { // new, post 4.4 format
                QIcon icon;
                if (flags & NormalOff)
                    icon.addFile(QFileInfo(workingDirectory, dpi->elementNormalOff()->text()).absoluteFilePath(), QSize(), QIcon::Normal, QIcon::Off);
                if (flags & NormalOn)
                    icon.addFile(QFileInfo(workingDirectory, dpi->elementNormalOn()->text()).absoluteFilePath(), QSize(), QIcon::Normal, QIcon::On);
                if (flags & DisabledOff)
                    icon.addFile(QFileInfo(workingDirectory, dpi->elementDisabledOff()->text()).absoluteFilePath(), QSize(), QIcon::Disabled, QIcon::Off);
                if (flags & DisabledOn)
                    icon.addFile(QFileInfo(workingDirectory, dpi->elementDisabledOn()->text()).absoluteFilePath(), QSize(), QIcon::Disabled, QIcon::On);
                if (flags & ActiveOff)
                    icon.addFile(QFileInfo(workingDirectory, dpi->elementActiveOff()->text()).absoluteFilePath(), QSize(), QIcon::Active, QIcon::Off);
                if (flags & ActiveOn)
                    icon.addFile(QFileInfo(workingDirectory, dpi->elementActiveOn()->text()).absoluteFilePath(), QSize(), QIcon::Active, QIcon::On);
                if (flags & SelectedOff)
                    icon.addFile(QFileInfo(workingDirectory, dpi->elementSelectedOff()->text()).absoluteFilePath(), QSize(), QIcon::Selected, QIcon::Off);
                if (flags & SelectedOn)
                    icon.addFile(QFileInfo(workingDirectory, dpi->elementSelectedOn()->text()).absoluteFilePath(), QSize(), QIcon::Selected, QIcon::On);
                return QVariant::fromValue(icon);
            } else { // 4.3 legacy
                const QIcon icon(QFileInfo(workingDirectory, dpi->text()).absoluteFilePath());
                return QVariant::fromValue(icon);
            }
        }
            break;
        default:
            break;
    }
    return QVariant();
}

QVariant QResourceBuilder::toNativeValue(const QVariant &value) const
{
    return value;
}

DomProperty *QResourceBuilder::saveResource(const QDir &workingDirectory, const QVariant &value) const
{
    Q_UNUSED(workingDirectory);
    Q_UNUSED(value);
    return nullptr;
}

bool QResourceBuilder::isResourceProperty(const DomProperty *p) const
{
    switch (p->kind()) {
        case DomProperty::Pixmap:
        case DomProperty::IconSet:
            return true;
        default:
            break;
    }
    return false;
}

bool QResourceBuilder::isResourceType(const QVariant &value) const
{
    switch (value.metaType().id()) {
        case QMetaType::QPixmap:
        case QMetaType::QIcon:
            return true;
        default:
            break;
    }
    return false;
}

#ifdef QFORMINTERNAL_NAMESPACE
} // namespace QFormInternal
#endif

QT_END_NAMESPACE
