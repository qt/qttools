// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QCoreApplication>
#include <QTranslator>

namespace Outer {
namespace Inner {
namespace Deep {
class MyClass
{
public:
    static int getValue() { return 42; }
    static int count;
};
} // namespace Deep
} // namespace Inner
} // namespace Outer

template <typename T>
class TemplateClass
{
public:
    static T value;
    static int getCount() { return 10; }
};

template <typename T, typename U>
class DoubleTemplate
{
public:
    static int getValue() { return 20; }
};

class Dialog
{
public:
    static int getCount() { return 5; }
    int value;
    int getValue() { return 42; }
};

int func()
{
    QTranslator trans;
    int n = 5;
    int array[5] = { 1, 2, 3, 4, 5 };
    Dialog obj;
    int *intPtr = &n;

    // Parenthesized expressions

    trans.translate("Context", "Single parens around identifier", "comment", (n));
    trans.translate("Context", "Double parens", "comment", ((n)));
    trans.translate("Context", "Triple parens", "comment", (((n))));
    trans.translate("Context", "Many nested parens", "comment", ((((((n)))))));
    trans.translate("Context", "Parens around number", "comment", (42));
    trans.translate("Context", "Parens around expression", "comment", (n + 1));
    trans.translate("Context", "Parens around complex expr", "comment", (n * 2 + 3));
    trans.translate("Context", "Parens with operators", "comment", ((n + 1) * (n - 1)));
    trans.translate("Context", "Nested parens with operators", "comment", (n + 1) * (n - 1));
    trans.translate("Context", "Empty parens case", "comment", (int)n);

    // Scope resolution operator ::

    trans.translate("Context", "Simple scope resolution", "comment", Dialog::getCount());
    trans.translate("Context", "Nested namespace scope", "comment",
                    Outer::Inner::Deep::MyClass::getValue());
    trans.translate("Context", "Scope with static member", "comment",
                    Outer::Inner::Deep::MyClass::count);
    trans.translate("Context", "Global scope", "comment", ::Dialog::getCount());
    trans.translate("Context", "Long scope chain", "comment",
                    Outer::Inner::Deep::MyClass::getValue());

    // Combined: Scope resolution + Parentheses

    trans.translate("Context", "Parens around scope resolution", "comment", (Dialog::getCount()));
    trans.translate("Context", "Double parens around scope", "comment",
                    ((Outer::Inner::Deep::MyClass::getValue())));
    trans.translate("Context", "Scope in paren expr", "comment", (Dialog::getCount() + 1));

    // Templates (angle brackets)

    trans.translate("Context", "Simple template", "comment", TemplateClass<int>::getCount());
    trans.translate("Context", "Template with scope", "comment", TemplateClass<Dialog>::getCount());
    trans.translate("Context", "Double template", "comment",
                    DoubleTemplate<int, double>::getValue());
    trans.translate("Context", "Nested template", "comment",
                    TemplateClass<TemplateClass<int>>::getCount());
    trans.translate("Context", "Template with parens", "comment", (TemplateClass<int>::getCount()));

    // Cast expressions

    trans.translate("Context", "static_cast with scope resolution", "comment",
                    static_cast<long>(Dialog::getCount()));
    trans.translate("Context", "static_cast in parens", "comment", (static_cast<int>(n)));
    Dialog *dptr = nullptr;
    trans.translate("Context", "reinterpret_cast", "comment", reinterpret_cast<long>(dptr));
    const int *cptr = &n;
    trans.translate("Context", "const_cast", "comment", *const_cast<int *>(cptr));
    trans.translate("Context", "C-style cast", "comment", (int)Dialog::getCount());
    trans.translate("Context", "Functional cast", "comment", int(Dialog::getCount()));

    // Combinations

    trans.translate("Context", "Triple combo", "comment",
                    static_cast<long>(TemplateClass<int>::getCount()));
    trans.translate("Context", "Nested cast with scope", "comment",
                    static_cast<int>(static_cast<long>(Dialog::getCount())));
    trans.translate("Context", "Template with multi-scope", "comment",
                    TemplateClass<Outer::Inner::Deep::MyClass>::getCount());
    trans.translate("Context", "Parens around cast with template", "comment",
                    (static_cast<long>(TemplateClass<int>::getCount())));
    trans.translate("Context", "Double nested template", "comment",
                    DoubleTemplate<TemplateClass<int>, Dialog>::getValue());
    trans.translate("Context", "Mixed angle and scope", "comment",
                    TemplateClass<Outer::Inner::Deep::MyClass>::getCount());

    // Arrow operator

    Dialog *ptr = nullptr;
    trans.translate("Context", "Arrow operator", "comment", ptr->getCount());
    trans.translate("Context", "Arrow with scope", "comment",
                    Outer::Inner::Deep::MyClass::getValue());
    trans.translate("Context", "Parens around arrow", "comment", (ptr->getCount()));

    // Comparison operators

    trans.translate("Context", "Greater than", "comment", (n > 5));
    trans.translate("Context", "Less than", "comment", (n < 5));
    trans.translate("Context", "Greater or equal", "comment", (n >= 5));
    trans.translate("Context", "Less or equal", "comment", (n <= 5));
    trans.translate("Context", "Equal", "comment", (n == 5));
    trans.translate("Context", "Not equal", "comment", (n != 5));

    // Array subscripts

    trans.translate("Context", "Array subscript", "comment", array[0]);
    trans.translate("Context", "Array with expression 1", "comment", array[0] * 2);
    trans.translate("Context", "Array with expression 2", "comment", array[n - 5]);

    // Dot operator

    trans.translate("Context", "Dot operator", "comment", obj.getValue());
    trans.translate("Context", "Dot member access", "comment", obj.value);

    // Unary operators

    trans.translate("Context", "Dereference", "comment", *intPtr);
    trans.translate("Context", "Address of", "comment", (long)&n);
    trans.translate("Context", "Logical not", "comment", !n);
    trans.translate("Context", "Bitwise not", "comment", ~n);
    trans.translate("Context", "Unary minus", "comment", -n);
    trans.translate("Context", "Unary plus", "comment", +n);

    // More binary operators

    trans.translate("Context", "Division", "comment", (n / 2));
    trans.translate("Context", "Modulo", "comment", (n % 3));
    trans.translate("Context", "Bitwise AND", "comment", (n & 0xFF));
    trans.translate("Context", "Bitwise OR", "comment", (n | 0xFF));
    trans.translate("Context", "Bitwise XOR", "comment", (n ^ 0xFF));
    trans.translate("Context", "Bitwise right shift", "comment", (n >> 2));

    // Logical operators

    trans.translate("Context", "Logical AND", "comment", (n && 5));
    trans.translate("Context", "Logical OR", "comment", (n || 5));

    // Special cases

    trans.translate("Context", "Ternary operator", "comment", (n > 0 ? n : 0));
    trans.translate("Context", "Bitwise left shift", "comment", (n << 1));

    return 0;
}
