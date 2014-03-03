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

#ifndef D3DSERVICE_H
#define D3DSERVICE_H

#include <QtCore/qt_windows.h>
#include <QtCore/QPair>
#include <QtCore/QString>
#include <QtCore/QLoggingCategory>

QT_USE_NAMESPACE

typedef QPair<QString, QString> QStringPair;
namespace D3DService
{
    HRESULT compileShader(const QString &source, const QString &destination);

    bool install();
    bool remove();

    bool registerApp(const QString &device, const QString &app);
    bool unregisterApp(const QString &device, const QString &app);
    QList<QStringPair> registrations();
    bool clearRegistrations();

    bool startService(bool replaceMessageHandler = true);
    bool startDirectly();

    void reportStatus(DWORD state, DWORD exitCode, DWORD waitHint);

    bool getCredentials(LPWSTR username, DWORD *usernameSize,
                        LPWSTR password, DWORD *passwordSize);
    bool executeElevated(LPCWSTR exe, LPCWSTR param, DWORD *exitCode);
    bool addLogonRight(LPCWSTR username);
}

Q_DECLARE_LOGGING_CATEGORY(lcD3DService)

#endif // D3DSERVICE_H
