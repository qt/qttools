// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef AUTOGENTEST_H
#define AUTOGENTEST_H

namespace AutoDocTests {

class AutoGenSmfTest {
public:
    AutoGenSmfTest() = default;
    AutoGenSmfTest(const AutoGenSmfTest &other) = default;
    AutoGenSmfTest(AutoGenSmfTest &&other) = default;
    ~AutoGenSmfTest() = default;
    AutoGenSmfTest &operator=(const AutoGenSmfTest &other) = default;
    AutoGenSmfTest &operator=(AutoGenSmfTest &&other) = default;

private:
    int m_value{0};
};

class DeletedSmfTest {
public:
    DeletedSmfTest() = default;
    DeletedSmfTest(const DeletedSmfTest &other) = delete;
    DeletedSmfTest(DeletedSmfTest &&other) = delete;
    DeletedSmfTest &operator=(const DeletedSmfTest &other) = delete;
    DeletedSmfTest &operator=(DeletedSmfTest &&other) = delete;

private:
    int m_value{0};
};

class VirtualDtorTest {
public:
    VirtualDtorTest() = default;
    virtual ~VirtualDtorTest() = default;
};

} // AutoDocTests

#endif // AUTOGENTEST_H

