/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "d3dservice.h"

#include <QtCore/QDir>
#include <QtCore/QStandardPaths>

QT_USE_NAMESPACE

QStringList D3DService::devices()
{
    const QDir dir(QStandardPaths::writableLocation(QStandardPaths::DataLocation)
                   + QStringLiteral("/qtd3dservice"));
    return dir.entryList(QDir::Dirs|QDir::NoDotAndDotDot);
}

QStringList D3DService::apps(const QString &device)
{
    const QDir dir(QStandardPaths::writableLocation(QStandardPaths::DataLocation)
                   + QStringLiteral("/qtd3dservice/")
                   + (device.isEmpty() ? QStringLiteral("local") : device));
    return dir.entryList(QDir::Dirs|QDir::NoDotAndDotDot);
}

QStringList D3DService::sources( const QString &device, const QString &app)
{
    const QDir dir(QStandardPaths::writableLocation(QStandardPaths::DataLocation)
                   + QStringLiteral("/qtd3dservice/")
                   + (device.isEmpty() ? QStringLiteral("local") : device)
                   + QLatin1Char('/') + app + QStringLiteral("/source"));
    QStringList entries;
    foreach (const QFileInfo &info, dir.entryInfoList(QDir::Files))
        entries.append(info.absoluteFilePath());
    return entries;
}

QStringList D3DService::binaries(const QString &device, const QString &app)
{
    const QDir dir(QStandardPaths::writableLocation(QStandardPaths::DataLocation)
                   + QStringLiteral("/qtd3dservice/")
                   + (device.isEmpty() ? QStringLiteral("local") : device)
                   + QLatin1Char('/') + app + QStringLiteral("/binary"));
    QStringList entries;
    foreach (const QFileInfo &info, dir.entryInfoList(QDir::Files))
        entries.append(info.absoluteFilePath());
    return entries;
}
