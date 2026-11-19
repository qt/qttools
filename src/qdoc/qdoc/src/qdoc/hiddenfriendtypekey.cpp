// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "hiddenfriendtypekey.h"

#include <clang/AST/DeclTemplate.h>
#include <clang/AST/Expr.h>
#include <clang/AST/Type.h>
#if __has_include(<clang/UnifiedSymbolResolution/USRGeneration.h>)
#  include <clang/UnifiedSymbolResolution/USRGeneration.h>
#else
#  include <clang/Index/USRGeneration.h>
#endif
#include <llvm/ADT/SmallString.h>

QT_BEGIN_NAMESPACE

namespace {

// A deliberately limited structural encoding, not a C++ deduction engine.
// Constructor tags have fixed arities; variable atoms are length framed. A
// declaration USR, a binder index and an integer value inhabit disjoint domains.
// Atoms: q=qualifiers, n=nominal USR, b=type binder index, f=builtin kind,
// a=value binder index, v=integer value, c=specialization argument count.
// Constructors: p=pointer, l/r=lvalue/rvalue reference, s=specialization,
// t=type argument, i=typed integral argument. These bytes are lookup-local,
// not a persisted serialization format.
class TypeKeyBuilder
{
public:
    explicit TypeKeyBuilder(const clang::TemplateParameterList &binders) : m_binders(binders) { }

    QByteArray build(clang::QualType type)
    {
        return supportedBinders() && writeType(type) ? m_key : QByteArray{};
    }

    QByteArray buildTemplateParameters()
    {
        if (!supportedBinders())
            return {};
        atom('c', QByteArray::number(m_binders.size()));
        for (const auto *binder : m_binders) {
            if (const auto *value = llvm::dyn_cast<clang::NonTypeTemplateParmDecl>(binder)) {
                m_key += 'i';
                if (!writeType(value->getType()))
                    return {};
            } else {
                m_key += 't';
            }
        }
        return m_key;
    }

private:
    bool supportedBinders() const
    {
        // Nested binders, constraints, packs and template-template parameters
        // need more than this positional convention. Do not guess their meaning.
        if (m_binders.size() == 0 || m_binders.getDepth() != 0 || m_binders.getRequiresClause())
            return false;
        for (const auto *binder : m_binders) {
            if (const auto *param = llvm::dyn_cast<clang::TemplateTypeParmDecl>(binder)) {
                if (param->isParameterPack() || param->hasTypeConstraint())
                    return false;
            } else if (const auto *param = llvm::dyn_cast<clang::NonTypeTemplateParmDecl>(binder)) {
                if (param->isParameterPack() || param->getType()->getContainedAutoType())
                    return false;
            } else {
                return false;
            }
        }
        return true;
    }

    void atom(char tag, const QByteArray &value)
    {
        m_key += tag;
        m_key += QByteArray::number(value.size());
        m_key += ':';
        m_key += value;
    }

    bool nominal(const clang::Decl *decl)
    {
        llvm::SmallString<128> usr;
        if (!decl || clang::index::generateUSRForDecl(decl->getCanonicalDecl(), usr))
            return false;
        atom('n', QByteArray(usr.data(), usr.size()));
        return true;
    }

    bool integral(const llvm::APSInt &value, clang::QualType type)
    {
        m_key += 'i';
        if (!writeType(type))
            return false;
        llvm::SmallString<32> digits;
        value.toString(digits, 10);
        atom('v', QByteArray(digits.data(), digits.size()));
        return true;
    }

    bool argument(const clang::TemplateArgument &arg)
    {
        switch (arg.getKind()) {
        case clang::TemplateArgument::Type:
            m_key += 't';
            return writeType(arg.getAsType());
        case clang::TemplateArgument::Integral:
            return integral(arg.getAsIntegral(), arg.getIntegralType());
        case clang::TemplateArgument::Expression: {
            const auto *expr = arg.getAsExpr()->IgnoreParens();
            if (const auto *ref = llvm::dyn_cast<clang::DeclRefExpr>(expr)) {
                if (const auto *param = llvm::dyn_cast<clang::NonTypeTemplateParmDecl>(ref->getDecl())) {
                    const unsigned index = param->getIndex();
                    // Clang can reuse a canonical dependent expression from a
                    // different declaration at the same binder depth/index.
                    if (param->getDepth() != m_binders.getDepth() || index >= m_binders.size()
                            || !llvm::isa<clang::NonTypeTemplateParmDecl>(m_binders.getParam(index)))
                        return false;
                    atom('a', QByteArray::number(index));
                    return writeType(param->getType());
                }
            }
            // Literal spellings are irrelevant; their type and evaluated value
            // are not. Dependent expressions other than a binder are unsupported.
            clang::Expr::EvalResult result;
            if (!expr->isValueDependent()
                    && expr->EvaluateAsInt(result, m_binders.getParam(0)->getASTContext()))
                return integral(result.Val.getInt(), expr->getType());
            return false;
        }
        default:
            return false;
        }
    }

    bool specialization(const clang::TemplateDecl *decl, llvm::ArrayRef<clang::TemplateArgument> args)
    {
        m_key += 's';
        if (!nominal(decl))
            return false;
        atom('c', QByteArray::number(args.size()));
        for (const auto &arg : args) {
            if (!argument(arg))
                return false;
        }
        return true;
    }

    bool writeType(clang::QualType type)
    {
        if (type.isNull())
            return false;
        type = type.getCanonicalType(); // Desugar aliases, including reference aliases.
        atom('q', QByteArray::number(type.getLocalQualifiers().getAsOpaqueValue()));
        const auto *raw = type.getTypePtr();
        if (const auto *param = llvm::dyn_cast<clang::TemplateTypeParmType>(raw)) {
            const unsigned index = param->getIndex();
            if (param->getDepth() != m_binders.getDepth() || index >= m_binders.size()
                    || !llvm::isa<clang::TemplateTypeParmDecl>(m_binders.getParam(index)))
                return false;
            atom('b', QByteArray::number(index));
            return true;
        }
        if (const auto *builtin = llvm::dyn_cast<clang::BuiltinType>(raw)) {
            if (builtin->isPlaceholderType() || builtin->isDependentType())
                return false;
            atom('f', QByteArray::number(builtin->getKind()));
            return true;
        }
        if (const auto *pointer = llvm::dyn_cast<clang::PointerType>(raw)) {
            m_key += 'p';
            return writeType(pointer->getPointeeType());
        }
        if (const auto *ref = llvm::dyn_cast<clang::ReferenceType>(raw)) {
            m_key += llvm::isa<clang::LValueReferenceType>(ref) ? 'l' : 'r';
            return writeType(ref->getPointeeType());
        }
        if (const auto *injected = llvm::dyn_cast<clang::InjectedClassNameType>(raw)) {
#if LIBCLANG_VERSION_MAJOR >= 22
            const auto *decl = injected->getDecl()->getDescribedClassTemplate();
            return decl && specialization(decl, decl->getInjectedTemplateArgs(decl->getASTContext()));
#else
            const auto *spec = injected->getInjectedSpecializationType()->getAs<clang::TemplateSpecializationType>();
            return spec && specialization(spec->getTemplateName().getAsTemplateDecl(), spec->template_arguments());
#endif
        }
        if (const auto *spec = llvm::dyn_cast<clang::TemplateSpecializationType>(raw))
            return specialization(spec->getTemplateName().getAsTemplateDecl(), spec->template_arguments());
        if (const auto *record = llvm::dyn_cast<clang::RecordType>(raw)) {
            const auto *decl = record->getDecl();
            if (const auto *spec = llvm::dyn_cast<clang::ClassTemplateSpecializationDecl>(decl))
                return specialization(spec->getSpecializedTemplate(), spec->getTemplateArgs().asArray());
            // A dependent nested record needs its enclosing arguments too.
            return !decl->isDependentContext() && nominal(decl);
        }
        if (const auto *enumeration = llvm::dyn_cast<clang::EnumType>(raw))
            return !enumeration->getDecl()->isDependentContext() && nominal(enumeration->getDecl());
        return false;
    }

    const clang::TemplateParameterList &m_binders;
    QByteArray m_key;
};

} // namespace

QByteArray hiddenFriendTypeKey(clang::QualType type, const clang::TemplateParameterList &binders)
{
    return TypeKeyBuilder(binders).build(type);
}

QByteArray templateParameterKey(const clang::TemplateParameterList &binders)
{
    return TypeKeyBuilder(binders).buildTemplateParameters();
}

QT_END_NAMESPACE
