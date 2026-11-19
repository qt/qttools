// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <qdoc/hiddenfriendtypekey.h>

// Like the end-to-end tests, these tests don't run under AddressSanitizer.
// They parse code with the provisioned libclang, which is built without
// sanitizer instrumentation. Inline allocator code in LLVM headers poisons
// and unpoisons memory depending on each translation unit's flags, so ASan
// reports use-after-poison inside libclang. Catch2 v2 can't skip tests at
// run time, so leave them out of the build instead.
#ifndef QT_ASAN_ENABLED

#include <catch/catch.hpp>

#include <clang/AST/DeclFriend.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/Frontend/ASTUnit.h>
#include <clang/Tooling/Tooling.h>

QT_BEGIN_NAMESPACE

namespace {

auto parse(const char *code)
{
    auto ast = clang::tooling::buildASTFromCodeWithArgs(code, { "-std=c++20" }, "signature.cpp");
    REQUIRE(ast);
    REQUIRE_FALSE(ast->getDiagnostics().hasErrorOccurred());
    return ast;
}

// Return owned bytes after destroying the AST, just as header parsing does.
QByteArray parameterKey(const char *code, unsigned index = 0)
{
    auto ast = parse(code);
    for (const auto *decl : ast->getASTContext().getTranslationUnitDecl()->decls()) {
        if (const auto *function = llvm::dyn_cast<clang::FunctionTemplateDecl>(decl)) {
            return hiddenFriendTypeKey(function->getTemplatedDecl()->getParamDecl(index)->getOriginalType(),
                                       *function->getTemplateParameters());
        }
    }
    FAIL("No function template in the test input");
    return {};
}

QByteArray binderKey(const char *code)
{
    auto ast = parse(code);
    for (const auto *decl : ast->getASTContext().getTranslationUnitDecl()->decls()) {
        if (const auto *function = llvm::dyn_cast<clang::FunctionTemplateDecl>(decl))
            return templateParameterKey(*function->getTemplateParameters());
    }
    FAIL("No function template in the test input");
    return {};
}

} // namespace

TEST_CASE("type keys normalize binders by position across translation units", "[clang][fnlookup]")
{
    const auto key = parameterKey("template <class T> void f(T &);");
    REQUIRE_FALSE(key.isEmpty());
    CHECK(key == parameterKey("template <class U> void f(U &);"));
    CHECK(key != parameterKey("template <class T> void f(T &&);"));
    CHECK(key != parameterKey("template <class T> void f(const T &);"));
    CHECK(key != parameterKey("template <class U, class T> void f(T &);"));
    CHECK(key != parameterKey("struct T {}; template <class U> void f(T &);"));
}

TEST_CASE("bare function template parameters retain their binder identity", "[clang][fnlookup]")
{
    const auto key = parameterKey("template <class T> void f(T);");
    REQUIRE_FALSE(key.isEmpty());
    CHECK(key == parameterKey("template <class U> void f(U);"));
    CHECK(key != parameterKey("template <class U, class T> void f(T);"));
    CHECK(key != parameterKey("struct T {}; template <class U> void f(T);"));
}

TEST_CASE("type keys desugar reference and pointer aliases", "[clang][fnlookup]")
{
    CHECK(parameterKey("template <class T> using Ref = T &; template <class U> void f(Ref<U>);")
          == parameterKey("template <class T> void f(T &);"));
    const auto key = parameterKey(
            "template <class T> using Ptr = const T *; template <class U> void f(Ptr<U> &);");
    REQUIRE_FALSE(key.isEmpty());
    CHECK(key == parameterKey("template <class T> void f(const T *&);"));
    CHECK(key != parameterKey("template <class T> void f(T *&);"));
}

TEST_CASE("injected class names match explicit specializations", "[clang][fnlookup]")
{
    auto ast = parse("template <class T> struct Box { friend void f(Box &); };");
    // Clang also creates implicit builtin declarations; find the named template.
    const clang::ClassTemplateDecl *box = nullptr;
    for (const auto *candidate : ast->getASTContext().getTranslationUnitDecl()->decls()) {
        if ((box = llvm::dyn_cast<clang::ClassTemplateDecl>(candidate)))
            break;
    }
    REQUIRE(box);
    const auto *friendDecl = *box->getTemplatedDecl()->friends().begin();
    const auto *function = llvm::cast<clang::FunctionDecl>(friendDecl->getFriendDecl());
    const auto key = hiddenFriendTypeKey(function->getParamDecl(0)->getOriginalType(),
                                        *box->getTemplateParameters());
    REQUIRE_FALSE(key.isEmpty());
    ast.reset();
    CHECK(key == parameterKey("template <class T> struct Box {}; template <class U> void f(Box<U> &);"));
}

TEST_CASE("integral arguments distinguish values, types and binders", "[clang][fnlookup]")
{
    const auto key = parameterKey("template <auto> struct Tag {}; template <int N> void f(Tag<N> &);");
    REQUIRE_FALSE(key.isEmpty());
    CHECK(key == parameterKey("template <auto> struct Tag {}; template <int M> void f(Tag<M> &);"));
    CHECK(key != parameterKey("template <auto> struct Tag {}; template <int N> void f(Tag<0> &);"));
    const auto fixed = parameterKey("template <auto> struct Tag {}; template <class T> void f(Tag<42> &);");
    REQUIRE_FALSE(fixed.isEmpty());
    CHECK(fixed == parameterKey("template <auto> struct Tag {}; template <class T> void f(Tag<6 * 7> &);"));
    CHECK(fixed != parameterKey("template <auto> struct Tag {}; template <class T> void f(Tag<42u> &);"));
    CHECK(fixed != parameterKey("template <auto> struct Other {}; template <class T> void f(Other<42> &);"));
}

TEST_CASE("unsupported type and binder forms have no matching key", "[clang][fnlookup]")
{
    const auto *code = GENERATE(
            "template <class... T> void f(int);",
            "template <template <class> class C> void f(int);",
            "template <auto N> void f(int);",
            "template <class T> requires (sizeof(T) > 0) void f(int);",
            "template <class T> concept C = true; template <C T> void f(int);",
            "template <int> struct Tag {}; template <int N> void f(Tag<N + 1> &);",
            "template <class T> void f(T (&)[3]);",
            "template <class T> void f(void (*)(T));",
            "template <class T> void f(int T::*);",
            "template <class T> void f(typename T::value_type);",
            "template <class T> void f(decltype(T::value));");
    CHECK(parameterKey(code).isEmpty());
}

TEST_CASE("binder compatibility compares dependent types by meaning", "[clang][fnlookup]")
{
    const auto key = binderKey("template <class T, T N> void f();");
    REQUIRE_FALSE(key.isEmpty());
    CHECK(key == binderKey("template <class U, U M> void f();"));
    CHECK(key == binderKey("template <class U = int, U M = 0> void f();"));
    CHECK(key != binderKey("template <class U, int M> void f();"));
    CHECK(key != binderKey("template <class U, U *M> void f();"));
    CHECK(key != binderKey("template <class U, class V, V M> void f();"));
    const auto alias = binderKey("using Value = int; template <Value N> void f();");
    REQUIRE_FALSE(alias.isEmpty());
    CHECK(alias == binderKey("template <int M> void f();"));
    CHECK(alias != binderKey("template <unsigned M> void f();"));
    CHECK(binderKey("template <class... T> void f();").isEmpty());
    CHECK(binderKey("template <auto N> void f();").isEmpty());
    CHECK(binderKey("template <template <class> class C> void f();").isEmpty());
    CHECK(binderKey("template <class T> requires true void f();").isEmpty());
}

QT_END_NAMESPACE

#endif // QT_ASAN_ENABLED

