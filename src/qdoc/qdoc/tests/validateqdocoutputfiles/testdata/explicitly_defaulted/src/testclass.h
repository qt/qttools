// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef EXPLICITLYDEFAULTEDTEST_H
#define EXPLICITLYDEFAULTEDTEST_H

#define QDOCTESTS_CONSTEXPR constexpr

#define QDOCTESTS_DEFAULTED_COPY(Class) \
    Class(const Class &other) = default; \
    Class &operator=(const Class &other) = default

#define QDOCTESTS_CONSTEXPR_DEFAULTED_COPY(Class) \
    constexpr Class(const Class &other) = default; \
    constexpr Class &operator=(const Class &other) = default

#define QDOCTESTS_ANNOTATE(text) __attribute__((annotate(#text)))
#define QDOCTESTS_INVOKABLE QDOCTESTS_ANNOTATE(invokable)
#define QDOCTESTS_NODISCARD [[nodiscard]]
#define QDOCTESTS_NODISCARD_CONSTEXPR \
    [[nodiscard]] constexpr

namespace QDocTests {

class ExplicitlyDefaultedTest {
public:
    ExplicitlyDefaultedTest() = default;
    ExplicitlyDefaultedTest(const ExplicitlyDefaultedTest &other) = default;
    ExplicitlyDefaultedTest(ExplicitlyDefaultedTest &&other) = default;
    ~ExplicitlyDefaultedTest() = default;
    ExplicitlyDefaultedTest &operator=(const ExplicitlyDefaultedTest &other) = default;
    ExplicitlyDefaultedTest &operator=(ExplicitlyDefaultedTest &&other) = default;

private:
    int m_value{0};
};

class UnnamedParameterTest {
public:
    UnnamedParameterTest() = default;
    UnnamedParameterTest &operator=(const UnnamedParameterTest &) = default;
    UnnamedParameterTest &operator=(UnnamedParameterTest &&) = default;

private:
    int m_value{0};
};

class DeclaredConstexprTest {
public:
    constexpr DeclaredConstexprTest() = default;
    QDOCTESTS_CONSTEXPR DeclaredConstexprTest(const DeclaredConstexprTest &other) = default;
    DeclaredConstexprTest(DeclaredConstexprTest &&other) = default;

private:
    int m_value{0};
};

class MacroDeclaredTest {
public:
    MacroDeclaredTest() = default;
    QDOCTESTS_DEFAULTED_COPY(MacroDeclaredTest);

private:
    int m_value{0};
};

class MacroDeclaredConstexprTest {
public:
    constexpr MacroDeclaredConstexprTest() = default;
    QDOCTESTS_CONSTEXPR_DEFAULTED_COPY(MacroDeclaredConstexprTest);

private:
    int m_value{0};
};

class MacroPrefixTest {
public:
    QDOCTESTS_NODISCARD MacroPrefixTest() = default;
    QDOCTESTS_INVOKABLE MacroPrefixTest(const MacroPrefixTest &other) = default;
    QDOCTESTS_NODISCARD_CONSTEXPR MacroPrefixTest(MacroPrefixTest &&other) = default;

private:
    int m_value{0};
};

class DeletedCopyCtorTest {
public:
    DeletedCopyCtorTest(const DeletedCopyCtorTest &) = delete;
    DeletedCopyCtorTest &operator=(const DeletedCopyCtorTest &) = delete;

    DeletedCopyCtorTest(int value, const char *name, void *data);

    void someMethod();

private:
    int m_value{0};
};

} // QDocTests

#endif // EXPLICITLYDEFAULTEDTEST_H

