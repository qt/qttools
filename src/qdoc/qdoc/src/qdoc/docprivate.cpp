// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial
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
