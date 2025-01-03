// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef EDITDISTANCE_H
#define EDITDISTANCE_H

#include <QtCore/qset.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

int editDistance(const QString &s, const QString &t);
QString nearestName(const QString &actual, const QSet<QString> &candidates);

QT_END_NAMESPACE

#endif
