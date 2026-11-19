// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef HIDDENFRIENDTYPEKEY_H
#define HIDDENFRIENDTYPEKEY_H

#include <QtCore/qbytearray.h>

namespace clang {
class QualType;
class TemplateParameterList;
}

QT_BEGIN_NAMESPACE

// An empty key means unsupported, never an equal type. The returned bytes own
// all their data; no Clang object may outlive its translation unit here.
QByteArray hiddenFriendTypeKey(clang::QualType type, const clang::TemplateParameterList &binders);
QByteArray templateParameterKey(const clang::TemplateParameterList &binders);

QT_END_NAMESPACE

#endif // HIDDENFRIENDTYPEKEY_H
