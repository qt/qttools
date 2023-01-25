// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "previewactiongroup.h"

#include <deviceprofile_p.h>
#include <shared_settings_p.h>

#include <QtWidgets/qstylefactory.h>
#include <QtCore/qvariant.h>

QT_BEGIN_NAMESPACE

enum { MaxDeviceActions = 20 };

namespace qdesigner_internal {

PreviewActionGroup::PreviewActionGroup(QDesignerFormEditorInterface *core, QObject *parent) :
    QActionGroup(parent),
    m_core(core)
{
    /* Create a list of up to MaxDeviceActions invisible actions to be
     * populated with device profiles (actiondata: index) followed by the
     * standard style actions (actiondata: style name). */
    connect(this, &PreviewActionGroup::triggered, this, &PreviewActionGroup::slotTriggered);
    setExclusive(true);

    const QString objNamePostfix = QStringLiteral("_action");
    // Create invisible actions for devices. Set index as action data.
    QString objNamePrefix = QStringLiteral("__qt_designer_device_");
    for (int i = 0; i < MaxDeviceActions; i++) {
        QAction *a = new QAction(this);
        QString objName = objNamePrefix;
        objName += QString::number(i);
        objName += objNamePostfix;
        a->setObjectName(objName);
        a->setVisible(false);
        a->setData(i);
        addAction(a);
    }
    // Create separator at index MaxDeviceActions
    QAction *sep = new QAction(this);
    sep->setObjectName(QStringLiteral("__qt_designer_deviceseparator"));
    sep->setSeparator(true);
    sep->setVisible(false);
    addAction(sep);
    // Populate devices
    updateDeviceProfiles();

    // Add style actions
    const QStringList styles = QStyleFactory::keys();
    // Make sure ObjectName  is unique in case toolbar solution is used.
    objNamePrefix = QStringLiteral("__qt_designer_style_");
    // Create styles. Set style name string as action data.
    for (const auto &s : styles) {
        QAction *a = new QAction(tr("%1 Style").arg(s), this);
        QString objName = objNamePrefix;
        objName += s;
        objName += objNamePostfix;
        a->setObjectName(objName);
        a->setData(s);
        addAction(a);
    }
}

void PreviewActionGroup::updateDeviceProfiles()
{
    const QDesignerSharedSettings settings(m_core);
    const auto profiles = settings.deviceProfiles();
    const auto al = actions();
    // Separator?
    const bool hasProfiles = !profiles.isEmpty();
    al.at(MaxDeviceActions)->setVisible(hasProfiles);
    int index = 0;
    if (hasProfiles) {
        // Make actions visible
        const int maxIndex = qMin(static_cast<int>(MaxDeviceActions), profiles.size());
        for (; index < maxIndex; index++) {
            const QString name = profiles.at(index).name();
            al.at(index)->setText(name);
            al.at(index)->setVisible(true);
        }
    }
    // Hide rest
    for ( ; index < MaxDeviceActions; index++)
        al.at(index)->setVisible(false);
}

void PreviewActionGroup::slotTriggered(QAction *a)
{
    // Device or style according to data.
    const QVariant data = a->data();
    switch (data.metaType().id()) {
    case QMetaType::QString:
        emit preview(data.toString(), -1);
        break;
    case QMetaType::Int:
        emit preview(QString(), data.toInt());
        break;
    default:
        break;
    }
}

}

QT_END_NAMESPACE
