// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QObject>

// ============================================================================
// CLASS ATTRIBUTES - Basic Cases
// ============================================================================

// Test 1: Basic attribute (original QTBUG-140686 bug report)
class [[maybe_unused]] AttributeTest1 : public QObject
{
    Q_OBJECT
public:
    void foo() { tr("AttributeTest1: basic attribute"); }
};

// Test 2: Multiple attributes
class [[deprecated]] [[nodiscard]] AttributeTest2 : public QObject
{
    Q_OBJECT
public:
    void foo() { tr("AttributeTest2: multiple attributes"); }
};

// Test 3: Attribute with string argument
class [[deprecated("Use NewClass instead")]] AttributeTest3 : public QObject
{
    Q_OBJECT
public:
    void foo() { tr("AttributeTest3: string argument"); }
};

// ============================================================================
// CLASS ATTRIBUTES - Edge Cases with Nested Brackets
// ============================================================================

// Test 4: Attribute with string containing brackets
class [[deprecated("Use Array[5] instead")]] AttributeTest4 : public QObject
{
    Q_OBJECT
public:
    void foo() { tr("AttributeTest4: brackets in string"); }
};

// Test 5: Attribute with nested brackets in string
class [[deprecated("Use Container[items[0]] instead")]] AttributeTest5 : public QObject
{
    Q_OBJECT
public:
    void foo() { tr("AttributeTest5: nested brackets in string"); }
};

// Test 6: Attribute with double closing brackets [[]] in string
class [[deprecated("Old [[syntax]] is bad")]] AttributeTest6 : public QObject
{
    Q_OBJECT
public:
    void foo() { tr("AttributeTest6: double brackets in string"); }
};

// Test 7: String with single closing bracket character
class [[deprecated("]")]] AttributeTest7 : public QObject
{
    Q_OBJECT
public:
    void foo() { tr("AttributeTest7: single bracket character"); }
};

// Test 8: Attribute with escaped quotes and brackets
class [[deprecated("Use \"NewClass[2]\" instead")]] AttributeTest8 : public QObject
{
    Q_OBJECT
public:
    void foo() { tr("AttributeTest8: escaped quotes with brackets"); }
};

// ============================================================================
// CLASS ATTRIBUTES - Raw String Literals
// ============================================================================

// Test 9: Attribute with raw string literal
class [[deprecated(R"(Use "Array[5]" instead)")]] AttributeTest9 : public QObject
{
    Q_OBJECT
public:
    void foo() { tr("AttributeTest9: raw string literal"); }
};

// Test 10: Raw string with brackets and double brackets
class [[deprecated(R"(Use "Array[5]" instead of [[old]]))")]] AttributeTest10 : public QObject
{
    Q_OBJECT
public:
    void foo() { tr("AttributeTest10: raw string with complex content"); }
};

// Test 11: Maximum complexity raw string
class [[deprecated(R"delim(
    Use Container[items[indices[0]]] with "quotes" and [[double brackets]]
    See: Documentation[[section[5]]]
)delim")]] AttributeTest11 : public QObject
{
    Q_OBJECT
public:
    void foo() { tr("AttributeTest11: maximum complexity"); }
};

// ============================================================================
// CLASS ATTRIBUTES - Special Cases
// ============================================================================

// Test 12: Attribute with namespace
class [[gnu::aligned(16)]] AttributeTest12 : public QObject
{
    Q_OBJECT
public:
    void foo() { tr("AttributeTest12: namespace attribute"); }
};

// Test 13: Attribute with function call argument
class [[gnu::aligned(sizeof(int))]] AttributeTest13 : public QObject
{
    Q_OBJECT
public:
    void foo() { tr("AttributeTest13: function argument"); }
};

// Test 14: Multi-line attribute
class [[deprecated("This is deprecated")]] AttributeTest14 : public QObject
{
    Q_OBJECT
public:
    void foo() { tr("AttributeTest14: multiline attribute"); }
};

// Test 15: Multi-line attribute with complex nested structures
class [[deprecated("This class uses [[old]] syntax. "
                   "Use Array[n] instead. "
                   "See \"Documentation[Page5]\" for details.")]] AttributeTest15 : public QObject
{
    Q_OBJECT
public:
    void foo() { tr("AttributeTest15: multiline with nested syntax"); }
};

// Test 16: Three consecutive attributes
class [[nodiscard]] [[deprecated("[obsolete]")]] [[maybe_unused]] AttributeTest16 : public QObject
{
    Q_OBJECT
public:
    void foo() { tr("AttributeTest16: three consecutive attributes"); }
};

// Test 17: Attribute with escape sequences
class [[deprecated("Line1\nLine2\t[Bracket]\r\n\"Quote\"")]] AttributeTest17 : public QObject
{
    Q_OBJECT
public:
    void foo() { tr("AttributeTest17: escape sequences"); }
};

// Test 18: Attribute with backslash escape sequences
class [[deprecated("Line1\nLine2\t[Tab]")]] AttributeTest18 : public QObject
{
    Q_OBJECT
public:
    void foo() { tr("AttributeTest18: backslash escapes"); }
};

// ============================================================================
// CLASS ATTRIBUTES - Templates
// ============================================================================

// Test 19: Template class with attribute
template <typename T>
class [[deprecated]] AttributeTest19 : public QObject
{
    Q_OBJECT
public:
    void foo() { tr("AttributeTest19: template with attribute"); }
};

// Test 20: Template class with multiple type parameters
template <typename T, int N>
class [[deprecated]] AttributeTest20 : public QObject
{
    Q_OBJECT
public:
    void foo() { tr("AttributeTest20: template multiple params"); }
};

// ============================================================================
// NAMESPACE ATTRIBUTES
// ============================================================================

// Test 21: Namespace with attribute
namespace [[deprecated]] AttributeNamespace {
class AttributeTest21 : public QObject
{
    Q_OBJECT
public:
    void foo() { tr("AttributeTest21: namespace attribute"); }
};
} // namespace AttributeNamespace

// Test 22: Namespace with attribute containing attributed class
namespace [[deprecated]] AttributeNamespace2 {
class [[maybe_unused]] AttributeTest22 : public QObject
{
    Q_OBJECT
public:
    void foo() { tr("AttributeTest22: attributed class in attributed namespace"); }
};
} // namespace AttributeNamespace2

// ============================================================================
// FUNCTION ATTRIBUTES - Basic
// ============================================================================

// Test 23: Member function with nodiscard
class AttributeTest23 : public QObject
{
    Q_OBJECT
public:
    [[nodiscard]] QString getValue() { return tr("AttributeTest23: function attribute"); }
};

// Test 24: Multiple attributes on function
class AttributeTest24 : public QObject
{
    Q_OBJECT
public:
    [[nodiscard]] [[deprecated]] QString getDeprecated()
    {
        return tr("AttributeTest24: multiple function attributes");
    }
};

// Test 25: Multiple attributes on function with string argument
class AttributeTest25 : public QObject
{
    Q_OBJECT
public:
    [[nodiscard]] [[deprecated("Use getNewValue")]] QString getOldValue()
    {
        return tr("AttributeTest25: function attributes with args");
    }
};

// ============================================================================
// FUNCTION ATTRIBUTES - Constructors and Special Functions
// ============================================================================

// Test 26: Constructor with attribute
class AttributeTest26 : public QObject
{
    Q_OBJECT
public:
    [[deprecated]] AttributeTest26() { tr("AttributeTest26: deprecated constructor"); }
};

// Test 27: Constructor with attribute and initializer list
class AttributeTest27 : public QObject
{
    Q_OBJECT
public:
    [[deprecated]] AttributeTest27() : QObject()
    {
        tr("AttributeTest27: constructor with initializer");
    }
};

// ============================================================================
// FUNCTION ATTRIBUTES - Operators
// ============================================================================

// Test 28: Operator overload with attribute
class AttributeTest28 : public QObject
{
    Q_OBJECT
public:
    [[nodiscard]] bool operator==(const AttributeTest28 &) const
    {
        tr("AttributeTest28: operator with attribute");
        return true;
    }
};

// Test 29: Conversion operator with attribute
class AttributeTest29 : public QObject
{
    Q_OBJECT
public:
    [[nodiscard]] operator bool() const
    {
        tr("AttributeTest29: conversion operator");
        return true;
    }
};

// ============================================================================
// FUNCTION ATTRIBUTES - Static and Virtual
// ============================================================================

// Test 30: Static member function
class AttributeTest30 : public QObject
{
    Q_OBJECT
public:
    [[nodiscard]] static QString getStatic()
    {
        return tr("AttributeTest30: static function attribute");
    }
};

// Test 31: Virtual function with attribute
class AttributeTest31 : public QObject
{
    Q_OBJECT
public:
    [[nodiscard]] virtual QString getVirtual()
    {
        return tr("AttributeTest31: virtual function attribute");
    }
};

// ============================================================================
// REGRESSION TEST
// ============================================================================

// Test 32: Original QTBUG-140686 bug report
// The exact case from the bug report
class [[maybe_unused]] OriginalBugTest : public QObject
{
    Q_OBJECT
public:
    OriginalBugTest() { setObjectName(tr("OriginalBugTest: the reported bug")); }
};
