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

static HKEY openBase()
{
    HKEY regKey;
    LONG result = RegCreateKeyEx(HKEY_CURRENT_USER_LOCAL_SETTINGS, L"qtd3dservice",
                                 0, NULL, 0, KEY_ALL_ACCESS, NULL, &regKey, NULL);
    if (result != ERROR_SUCCESS) {
        qCCritical(lcD3DService) << "Unable to open registry, error:" << result;
        return 0;
    }
    return regKey;
}

bool D3DService::registerApp(const QString &device, const QString &app)
{
    HKEY baseKey = openBase();
    if (!baseKey)
        return false;

    HKEY deviceKey;
    LONG result = RegCreateKeyEx(baseKey, reinterpret_cast<LPCWSTR>(device.utf16()),
                                 0, NULL, 0, KEY_ALL_ACCESS, NULL, &deviceKey, NULL);
    if (result != ERROR_SUCCESS)
        return false;

    HKEY appKey;
    result = RegCreateKeyEx(deviceKey, reinterpret_cast<LPCWSTR>(app.utf16()),
                            0, NULL, 0, KEY_ALL_ACCESS, NULL, &appKey, NULL);
    return result == ERROR_SUCCESS;
}

bool D3DService::unregisterApp(const QString &device, const QString &app)
{
    HKEY baseKey = openBase();
    if (!baseKey)
        return false;

    HKEY deviceKey;
    LONG result = RegCreateKeyEx(baseKey, reinterpret_cast<LPCWSTR>(device.utf16()),
                                 0, NULL, 0, KEY_ALL_ACCESS, NULL, &deviceKey, NULL);
    if (result != ERROR_SUCCESS)
        return false;

    result = RegDeleteKey(deviceKey, reinterpret_cast<LPCWSTR>(app.utf16()));
    return true;
}

QList<QStringPair> D3DService::registrations()
{
    QList<QStringPair> registrations;

    HKEY baseKey = openBase();
    if (!baseKey)
        return registrations;

    int index = 0;
    forever {
        wchar_t device[256];
        wchar_t app[256];
        DWORD size = 256;
        LONG result = RegEnumKeyEx(baseKey, index++, device, &size, NULL, NULL, NULL, NULL);
        if (result != ERROR_SUCCESS)
            break;
        HKEY subKey;
        result = RegOpenKeyEx(baseKey, device, 0, KEY_READ, &subKey);
        if (result != ERROR_SUCCESS)
            continue;
        int subIndex = 0;
        forever {
            size = 256;
            result = RegEnumKeyEx(subKey, subIndex++, app, &size, NULL, NULL, NULL, NULL);
            if (result != ERROR_SUCCESS)
                break;
            registrations.append(qMakePair(QString::fromWCharArray(device),
                                           QString::fromWCharArray(app)));
        }
    }

    return registrations;
}

bool D3DService::clearRegistrations()
{
    LONG result = RegDeleteTree(HKEY_CURRENT_USER_LOCAL_SETTINGS, L"qtd3dservice");
    return result == ERROR_SUCCESS || result == ERROR_FILE_NOT_FOUND;
}
