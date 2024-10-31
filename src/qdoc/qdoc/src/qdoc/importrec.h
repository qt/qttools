// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef IMPORTREC_H
#define IMPORTREC_H

#include <QtCore/qglobal.h>
#include <QtCore/qstring.h>

#include <utility>

QT_BEGIN_NAMESPACE

struct ImportRec
{
    QString m_moduleName {};
    QString m_majorMinorVersion {};
    QString m_importUri {};
    QString m_importId {};

    ImportRec(QString name, QString version, QString importUri, QStringView importId)
        : m_moduleName(std::move(name)),
          m_majorMinorVersion(std::move(version)),
          m_importUri(std::move(importUri)),
          m_importId(importId)
    {
    }
    QString &name() { return m_moduleName; }
    QString &version() { return m_majorMinorVersion; }
    [[nodiscard]] bool isEmpty() const { return m_moduleName.isEmpty(); }
};

QT_END_NAMESPACE

#endif // IMPORTREC_H
