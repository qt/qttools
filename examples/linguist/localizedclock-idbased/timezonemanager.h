// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef TIMEZONEMANAGER_H
#define TIMEZONEMANAGER_H

#include <QObject>
#include <QQmlEngine>

class Dialog;

class TimeZoneManager : public QObject
{
    Q_OBJECT
    QML_SINGLETON
    QML_ELEMENT
    Q_PROPERTY(QString timeZone READ timeZone NOTIFY timeZoneChanged)

public:
    explicit TimeZoneManager(QObject *parent = nullptr);
    ~TimeZoneManager();
    Q_INVOKABLE void openDialog();

    Q_INVOKABLE QString timeZone() const { return m_timeZone; }
    Q_INVOKABLE qint64 currentTimeZoneOffsetMs();

signals:
    void timeZoneChanged();

private slots:
    void setTimeZone(const QString &timeZone);

private:
    QString m_timeZone;
    std::unique_ptr<Dialog> m_dialog;
};

#endif // TIMEZONEMANAGER_H
