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
                const qsizetype themeEnum = theme.at(0).isUpper()
                    ? themeIconNames().indexOf(theme) : -1;
                if (themeEnum != -1) {
                    const auto themeEnumE = static_cast<QIcon::ThemeIcon>(themeEnum);
                    return QVariant::fromValue(QIcon::fromTheme(themeEnumE));
                }
                const bool known = QIcon::hasThemeIcon(theme);
                if (themeDebug)
                    qDebug("Theme %s known %d", qPrintable(theme), known);
                if (known)
                    return QVariant::fromValue(QIcon::fromTheme(theme));
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

const QStringList &QResourceBuilder::themeIconNames()
{
    static const QStringList result = {
        "AddressBookNew"_L1, "ApplicationExit"_L1, "AppointmentNew"_L1,
        "CallStart"_L1, "CallStop"_L1, "ContactNew"_L1,
        "DocumentNew"_L1, "DocumentOpen"_L1, "DocumentOpenRecent"_L1,
        "DocumentPageSetup"_L1, "DocumentPrint"_L1, "DocumentPrintPreview"_L1,
        "DocumentProperties"_L1, "DocumentRevert"_L1, "DocumentSave"_L1,
        "DocumentSaveAs"_L1, "DocumentSend"_L1,
        "EditClear"_L1, "EditCopy"_L1, "EditCut"_L1, "EditDelete"_L1,
        "EditFind"_L1, "EditPaste"_L1,
        "EditRedo"_L1, "EditSelectAll"_L1, "EditUndo"_L1,
        "FolderNew"_L1,
        "FormatIndentLess"_L1, "FormatIndentMore"_L1,
        "FormatJustifyCenter"_L1, "FormatJustifyFill"_L1,
        "FormatJustifyLeft"_L1, "FormatJustifyRight"_L1,
        "FormatTextDirectionLtr"_L1, "FormatTextDirectionRtl"_L1,
        "FormatTextBold"_L1, "FormatTextItalic"_L1,
        "FormatTextUnderline"_L1, "FormatTextStrikethrough"_L1,
        "GoDown"_L1, "GoHome"_L1, "GoNext"_L1, "GoPrevious"_L1, "GoUp"_L1,
        "HelpAbout"_L1, "HelpFaq"_L1,
        "InsertImage"_L1, "InsertLink"_L1, "InsertText"_L1,
        "ListAdd"_L1, "ListRemove"_L1,
        "MailForward"_L1, "MailMarkImportant"_L1,  "MailMarkRead"_L1, "MailMarkUnread"_L1,
        "MailMessageNew"_L1, "MailReplyAll"_L1, "MailReplySender"_L1,
        "MailSend"_L1,
        "MediaEject"_L1, "MediaPlaybackPause"_L1, "MediaPlaybackStart"_L1,
        "MediaPlaybackStop"_L1, "MediaRecord"_L1, "MediaSeekBackward"_L1,
        "MediaSeekForward"_L1, "MediaSkipBackward"_L1,
        "MediaSkipForward"_L1,
        "ObjectRotateLeft"_L1, "ObjectRotateRight"_L1,
        "ProcessStop"_L1,
        "SystemLockScreen"_L1, "SystemLogOut"_L1,
        "SystemSearch"_L1, "SystemReboot"_L1, "SystemShutdown"_L1,
        "ToolsCheckSpelling"_L1,
        "ViewFullscreen"_L1, "ViewRefresh"_L1, "ViewRestore"_L1,
        "WindowClose"_L1, "WindowNew"_L1,
        "ZoomFitBest"_L1, "ZoomIn"_L1, "ZoomOut"_L1,
        "AudioCard"_L1, "AudioInputMicrophone"_L1,
        "Battery"_L1,
        "CameraPhoto"_L1, "CameraVideo"_L1, "CameraWeb"_L1,
        "Computer"_L1, "DriveHarddisk"_L1, "DriveOptical"_L1,
        "InputGaming"_L1, "InputKeyboard"_L1, "InputMouse"_L1,
        "InputTablet"_L1,
        "MediaFlash"_L1, "MediaOptical"_L1,
        "MediaTape"_L1,
        "MultimediaPlayer"_L1,
        "NetworkWired"_L1, "NetworkWireless"_L1,
        "Phone"_L1, "Printer"_L1, "Scanner"_L1, "VideoDisplay"_L1,
        "AppointmentMissed"_L1, "AppointmentSoon"_L1,
        "AudioVolumeHigh"_L1, "AudioVolumeLow"_L1, "AudioVolumeMedium"_L1,
        "AudioVolumeMuted"_L1,
        "BatteryCaution"_L1, "BatteryLow"_L1,
        "DialogError"_L1, "DialogInformation"_L1, "DialogPassword"_L1,
        "DialogQuestion"_L1, "DialogWarning"_L1,
        "FolderDragAccept"_L1, "FolderOpen"_L1, "FolderVisiting"_L1,
        "ImageLoading"_L1, "ImageMissing"_L1,
        "MailAttachment"_L1, "MailUnread"_L1, "MailRead"_L1,
        "MailReplied"_L1,
        "MediaPlaylistRepeat"_L1, "MediaPlaylistShuffle"_L1,
        "NetworkOffline"_L1,
        "PrinterPrinting"_L1,
        "SecurityHigh"_L1, "SecurityLow"_L1,
        "SoftwareUpdateAvailable"_L1, "SoftwareUpdateUrgent"_L1,
        "SyncError"_L1,  "SyncSynchronizing"_L1,
        "UserAvailable"_L1,
        "UserOffline"_L1,
        "WeatherClear"_L1, "WeatherClearNight"_L1, "WeatherFewClouds"_L1,
        "WeatherFewCloudsNight"_L1, "WeatherFog"_L1, "WeatherShowers"_L1,
        "WeatherSnow"_L1, "WeatherStorm"_L1
    };

    return result;
};

int QResourceBuilder::themeIconIndex(QStringView name)
{
    const auto lastQual = name.lastIndexOf("::"_L1);
    const auto result = lastQual != -1
        ? themeIconNames().indexOf(name.sliced(lastQual + 2))
        : themeIconNames().indexOf(name);
    return int(result);
}

QString QResourceBuilder::fullyQualifiedThemeIconName(int i)
{
    return i >= 0 && i < themeIconNames().size()
        ? "QIcon::ThemeIcon::"_L1 + themeIconNames().at(i) : QString{};
}

#ifdef QFORMINTERNAL_NAMESPACE
} // namespace QFormInternal
#endif

QT_END_NAMESPACE
