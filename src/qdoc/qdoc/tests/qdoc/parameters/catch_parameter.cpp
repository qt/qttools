// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <catch/catch.hpp>
#include "qdoc/parameter.h"

QT_USE_NAMESPACE

SCENARIO("Parameter signature with inside-out declarator syntax", "[Parameter][signature]")
{
    GIVEN("A parameter with a simple type") {
        Parameter p(QStringLiteral("int"), QStringLiteral("x"));
        THEN("The name is appended after the type") {
            REQUIRE(p.signature() == QStringLiteral("int x"));
        }
    }

    GIVEN("A parameter with a pointer type") {
        Parameter p(QStringLiteral("int *"), QStringLiteral("ptr"));
        THEN("The name follows the pointer without extra space") {
            REQUIRE(p.signature() == QStringLiteral("int *ptr"));
        }
    }

    GIVEN("A parameter with a reference type") {
        Parameter p(QStringLiteral("const int &"), QStringLiteral("ref"));
        THEN("The name follows the reference without extra space") {
            REQUIRE(p.signature() == QStringLiteral("const int &ref"));
        }
    }

    GIVEN("A reference-to-array parameter") {
        Parameter p(QStringLiteral("const char (&)[Size]"), QStringLiteral("data"));
        THEN("The name is inserted inside the declarator") {
            REQUIRE(p.signature() == QStringLiteral("const char (&data)[Size]"));
        }
    }

    GIVEN("A pointer-to-function parameter") {
        Parameter p(QStringLiteral("void (*)(int, int)"), QStringLiteral("callback"));
        THEN("The name is inserted inside the declarator") {
            REQUIRE(p.signature() == QStringLiteral("void (*callback)(int, int)"));
        }
    }

    GIVEN("A pointer-to-member-function parameter") {
        Parameter p(QStringLiteral("void (Cls::*)(int)"), QStringLiteral("member"));
        THEN("The name is inserted inside the declarator") {
            REQUIRE(p.signature() == QStringLiteral("void (Cls::*member)(int)"));
        }
    }

    GIVEN("A pointer-to-array parameter") {
        Parameter p(QStringLiteral("int (*)[N]"), QStringLiteral("arr"));
        THEN("The name is inserted inside the declarator") {
            REQUIRE(p.signature() == QStringLiteral("int (*arr)[N]"));
        }
    }

    GIVEN("A pointer-to-pointer-to-array parameter") {
        Parameter p(QStringLiteral("int (**)[N]"), QStringLiteral("arr"));
        THEN("The name is inserted inside the declarator") {
            REQUIRE(p.signature() == QStringLiteral("int (**arr)[N]"));
        }
    }

    GIVEN("A parameter with a default value") {
        Parameter p(QStringLiteral("int"), QStringLiteral("x"), QStringLiteral("0"));
        THEN("The default value is included when requested") {
            REQUIRE(p.signature(true) == QStringLiteral("int x = 0"));
        }
        THEN("The default value is omitted when not requested") {
            REQUIRE(p.signature(false) == QStringLiteral("int x"));
        }
    }

    GIVEN("A parameter with no name") {
        Parameter p(QStringLiteral("int"));
        THEN("Only the type is returned") {
            REQUIRE(p.signature() == QStringLiteral("int"));
        }
    }
}

SCENARIO("Parameter name insertion point for inside-out declarators",
         "[Parameter][nameInsertionPoint]")
{
    GIVEN("A simple type")
    {
        Parameter p(QStringLiteral("int"));
        THEN("Returns -1 (name appended at end)")
        {
            REQUIRE(p.nameInsertionPoint() == -1);
        }
    }

    GIVEN("A pointer type")
    {
        Parameter p(QStringLiteral("int *"));
        THEN("Returns -1 (name appended at end)")
        {
            REQUIRE(p.nameInsertionPoint() == -1);
        }
    }

    GIVEN("A reference type")
    {
        Parameter p(QStringLiteral("const int &"));
        THEN("Returns -1 (name appended at end)")
        {
            REQUIRE(p.nameInsertionPoint() == -1);
        }
    }

    GIVEN("A reference-to-array type")
    {
        Parameter p(QStringLiteral("const char (&)[Size]"));
        THEN("Returns the position before the closing paren")
        {
            // "const char (&)[Size]"
            //              ^ position 13
            REQUIRE(p.nameInsertionPoint() == 13);
        }
    }

    GIVEN("A pointer-to-function type")
    {
        Parameter p(QStringLiteral("void (*)(int, int)"));
        THEN("Returns the position before the closing paren")
        {
            // "void (*)(int, int)"
            //        ^ position 7
            REQUIRE(p.nameInsertionPoint() == 7);
        }
    }

    GIVEN("A pointer-to-member-function type")
    {
        Parameter p(QStringLiteral("void (Cls::*)(int)"));
        THEN("Returns the position before the closing paren")
        {
            // "void (Cls::*)(int)"
            //             ^ position 12
            REQUIRE(p.nameInsertionPoint() == 12);
        }
    }

    GIVEN("A pointer-to-array type")
    {
        Parameter p(QStringLiteral("int (*)[N]"));
        THEN("Returns the position before the closing paren")
        {
            // "int (*)[N]"
            //       ^ position 6
            REQUIRE(p.nameInsertionPoint() == 6);
        }
    }

    GIVEN("A pointer-to-pointer-to-array type")
    {
        Parameter p(QStringLiteral("int (**)[N]"));
        THEN("Returns the position before the closing paren")
        {
            // "int (**)[N]"
            //        ^ position 7
            REQUIRE(p.nameInsertionPoint() == 7);
        }
    }
}
