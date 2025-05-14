// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "file.h"

using namespace NS1::NS2;

namespace NS1 {
namespace NS2 {
class cl
{
    void func();
};
} // namespace NS2

void cl::func()
{
    Tr::tr("translate in context NS");
}
} // namespace NS1
