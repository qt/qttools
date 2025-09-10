// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
#include "docprivate.h"

#include "text.h"

#include <QtCore/qhash.h>

QT_BEGIN_NAMESPACE

/*!
  Deletes the DocPrivateExtra.
 */
DocPrivate::~DocPrivate()
{
    delete extra;
}

void DocPrivate::addAlso(const Text &also)
{
    m_alsoList.append(also);
}

void DocPrivate::constructExtra()
{
    if (extra == nullptr)
        extra = new DocPrivateExtra;
}

QT_END_NAMESPACE
