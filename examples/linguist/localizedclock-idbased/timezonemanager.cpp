// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "timezonemanager.h"
#include "dialog.h"

#include <memory>

TimeZoneManager::TimeZoneManager(QObject *parent) : QObject(parent)
{
    setTimeZone(QString::fromLatin1(QTimeZone::systemTimeZoneId()));
}

TimeZoneManager::~TimeZoneManager() { }

void TimeZoneManager::openDialog()
{
    if (!m_dialog) {
        m_dialog = std::make_unique<Dialog>();
        m_dialog->setModal(true);
        connect(m_dialog.get(), &Dialog::timeZoneSelected, this, &TimeZoneManager::setTimeZone);
    }
    m_dialog->show();
}

qint64 TimeZoneManager::currentTimeZoneOffsetMs()
{
    const QTimeZone tz(m_timeZone.toLatin1());
    if (!tz.isValid())
        return 0;
    const QDateTime nowUtc = QDateTime::currentDateTimeUtc();

    const int targetOffset = tz.offsetFromUtc(nowUtc);
    const int systemOffset = QTimeZone::systemTimeZone().offsetFromUtc(nowUtc);

    return (targetOffset - systemOffset) * 1000;
}

void TimeZoneManager::setTimeZone(const QString &timeZone)
{
    if (m_timeZone != timeZone) {
        m_timeZone = timeZone;
        emit timeZoneChanged();
    }
}
