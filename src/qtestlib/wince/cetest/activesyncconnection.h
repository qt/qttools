/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef ACTIVESYNC_REMOTECONNECTION_H
#define ACTIVESYNC_REMOTECONNECTION_H

#include "remoteconnection.h"

#if defined(Q_OS_WIN32)
#define REMOTELIBNAME "remotecommands"
#endif

class ActiveSyncConnection : public AbstractRemoteConnection
{
public:
    ActiveSyncConnection();
    virtual ~ActiveSyncConnection();

    bool connect(QVariantList &list = QVariantList());
    void disconnect();
    bool isConnected() const;

    // These functions are designed for transfer between desktop and device
    // Caution: deviceDest path has to be device specific (eg. no drive letters for CE)
    bool copyFileToDevice(const QString &localSource, const QString &deviceDest, bool failIfExists = false);
    bool copyDirectoryToDevice(const QString &localSource, const QString &deviceDest, bool recursive = true);
    bool copyFileFromDevice(const QString &deviceSource, const QString &localDest, bool failIfExists = false);
    bool copyDirectoryFromDevice(const QString &deviceSource, const QString &localDest, bool recursive = true);

    bool timeStampForLocalFileTime(FILETIME*) const;
    bool fileCreationTime(const QString &fileName, FILETIME*) const;

    // These functions only work on files existing on the device
    bool copyFile(const QString&, const QString&, bool failIfExists = false);
    bool copyDirectory(const QString&, const QString&, bool recursive = true);
    bool deleteFile(const QString&);
    bool deleteDirectory(const QString&, bool recursive = true, bool failIfContentExists = false);
    bool moveFile(const QString&, const QString&, bool FailIfExists = false);
    bool moveDirectory(const QString&, const QString&, bool recursive = true);

    bool createDirectory(const QString&, bool deleteBefore=false);

    bool execute(QString program, QString arguments = QString(), int timeout = -1, int *returnValue = NULL);
    bool resetDevice();
    bool toggleDevicePower(int *returnValue = NULL);
    bool setDeviceAwake(bool activate, int *returnValue = NULL);
private:
    bool connected;
};

#endif
