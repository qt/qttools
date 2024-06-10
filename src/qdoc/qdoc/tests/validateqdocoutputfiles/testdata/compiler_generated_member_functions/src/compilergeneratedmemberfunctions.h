// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef COMPILERGENERATEDMEMBERFUNCTIONS_H
#define COMPILERGENERATEDMEMBERFUNCTIONS_H

namespace QDocTests {

class CompilerGeneratedMemberFunctions {
public:
    int number() const { return m_number; }

private:
    int m_number {42};

};

} // QDocTests

#endif // COMPILERGENERATEDMEMBERFUNCTIONS_H
