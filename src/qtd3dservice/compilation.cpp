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
#include <QtCore/QFile>

#include <d3dcommon.h>
#include <wrl.h>
using namespace Microsoft::WRL;

typedef HRESULT (WINAPI *D3DCompileFunc)(
        const void *, SIZE_T, const char *, const D3D_SHADER_MACRO *, ID3DInclude *,
        const char *, const char *, UINT, UINT, ID3DBlob **, ID3DBlob **);

HRESULT D3DService::compileShader(const QString &source, const QString &destination)
{
    static D3DCompileFunc D3DCompile = 0;
    if (!D3DCompile) {
        HMODULE d3dcompiler = LoadLibrary(L"d3dcompiler_47");
        if (!d3dcompiler)
            return GetLastError();

        D3DCompile = reinterpret_cast<D3DCompileFunc>(GetProcAddress(d3dcompiler, "D3DCompile"));
        if (!D3DCompile)
            return GetLastError();
    }

    QFile sourceFile(source);
    qCDebug(lcD3DService) << "Shader source observed at:" << source;
    if (!sourceFile.open(QFile::ReadOnly)) {
        qCWarning(lcD3DService) << "Unable to open shader source:" << sourceFile.errorString();
        return E_FAIL;
    }

    const QByteArray data = sourceFile.readAll();

    QStringList parts = source.split(QLatin1Char('!'));
    if (parts.size() < 4) {
        qCWarning(lcD3DService) << "The shader source file is missing meta data.";
        return E_FAIL;
    }

    ComPtr<ID3DBlob> blob, errorMessage;
    HRESULT hr = D3DCompile(data, data.size(), parts.at(0).toUtf8(), 0, 0, parts.at(1).toUtf8(),
                            parts.at(2).toUtf8(), parts.at(3).toUInt(), 0, &blob, &errorMessage);
    if (FAILED(hr)) {
        if (errorMessage) {
            const QString error = QString::fromUtf8(
                        static_cast<const char *>(errorMessage->GetBufferPointer()),
                        errorMessage->GetBufferSize());
            qCWarning(lcD3DService) << error;
        }
        return hr;
    }

    QFile destinationFile(destination);
    if (!destinationFile.open(QFile::WriteOnly)) {
        qCWarning(lcD3DService) << "Unable to open destination file:" << destinationFile.errorString();
        return E_FAIL;
    }

    destinationFile.write((const char *)blob->GetBufferPointer(), blob->GetBufferSize());
    return hr;
}
