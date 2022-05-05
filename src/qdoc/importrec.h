/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
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
    QString m_importUri {}; // subdirectory of module directory

    ImportRec(QString name, QString version, QString importUri)
        : m_moduleName(std::move(name)),
          m_majorMinorVersion(std::move(version)),
          m_importUri(std::move(importUri))
    {
    }
    QString &name() { return m_moduleName; }
    QString &version() { return m_majorMinorVersion; }
    [[nodiscard]] bool isEmpty() const { return m_moduleName.isEmpty(); }
};

QT_END_NAMESPACE

#endif // IMPORTREC_H
