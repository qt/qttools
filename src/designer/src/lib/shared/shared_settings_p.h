/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
******************************************************************************/

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef SHARED_SETTINGS_H
#define SHARED_SETTINGS_H

#include "shared_global_p.h"
#include "shared_enums_p.h"
#include "deviceprofile_p.h"

#include <QtCore/qglobal.h>
#include <QtCore/qlist.h>

QT_BEGIN_NAMESPACE

class QDesignerFormEditorInterface;
class QDesignerSettingsInterface;

class QSize;

namespace qdesigner_internal {
class Grid;
class PreviewConfiguration;
}

/*!
  Auxiliary methods to store/retrieve settings
  */
namespace qdesigner_internal {

class QDESIGNER_SHARED_EXPORT QDesignerSharedSettings {
public:
    using DeviceProfileList = QList<DeviceProfile>;

    explicit QDesignerSharedSettings(QDesignerFormEditorInterface *core);

    Grid defaultGrid() const;
    void setDefaultGrid(const Grid &grid);

    QStringList formTemplatePaths() const;
    void setFormTemplatePaths(const QStringList &paths);

    void setAdditionalFormTemplatePaths(const QStringList &additionalPaths);
    QStringList additionalFormTemplatePaths() const;

    QString formTemplate() const;
    void setFormTemplate(const QString &t);

    QSize newFormSize() const;
    void setNewFormSize(const QSize &s);

    // Check with isCustomPreviewConfigurationEnabled if custom or default
    // configuration should be used.
    PreviewConfiguration customPreviewConfiguration() const;
    void setCustomPreviewConfiguration(const PreviewConfiguration &configuration);

    bool isCustomPreviewConfigurationEnabled() const;
    void setCustomPreviewConfigurationEnabled(bool enabled);

    QStringList userDeviceSkins() const;
    void setUserDeviceSkins(const QStringList &userDeviceSkins);

    bool zoomEnabled() const;
    void setZoomEnabled(bool v);

    // Zoom in percent
    int zoom() const;
    void setZoom(int z);

    // Object naming convention (ActionEditor)
    ObjectNamingMode objectNamingMode() const;
    void setObjectNamingMode(ObjectNamingMode n);

    // Embedded Design
    DeviceProfile currentDeviceProfile() const;
    void setCurrentDeviceProfileIndex(int i);
    int currentDeviceProfileIndex() const;

    DeviceProfile deviceProfileAt(int idx) const;
    DeviceProfileList deviceProfiles() const;
    void setDeviceProfiles(const DeviceProfileList &dp);

    static const QStringList &defaultFormTemplatePaths();

protected:
    QDesignerSettingsInterface *settings() const { return m_settings; }

private:
    QStringList deviceProfileXml() const;
    QDesignerSettingsInterface *m_settings;
};

} // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // SHARED_SETTINGS_H
