// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef GHOSTSINGLETON_H
#define GHOSTSINGLETON_H

#include <QtCore/QObject>
#include <QtQml/qqml.h>

class GhostSingleton : public QObject
{
    Q_OBJECT
    QML_SINGLETON

public:
    explicit GhostSingleton(QObject *parent = nullptr);

    Q_INVOKABLE QString getGhostMessage() const;

private:
    QString m_message;
};

#endif // GHOSTSINGLETON_H
