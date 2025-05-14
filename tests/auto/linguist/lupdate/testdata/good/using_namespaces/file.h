// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once
#include <QCoreAppplication>
namespace NS1 {
struct Tr
{
    Q_DECLARE_TR_FUNCTIONS(NS)
};

namespace NS2 { // defining NS1::NS2 outside of file.cpp
} // namespace NS2
} // namespace NS1
