// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

/*
 * Represents a general declaration that has a form that can be
 * described by a type, name and initializer triplet, or any such form
 * that can be described by zero or more of those same parts.
 *
 * For example, it can be used to represent a C++ variable declaration
 * such as:
 *
 *     std::vector<int> foo = { 1, 2, 3 };
 *
 * Where `std::vector<int>` is the type, `foo` is the name and `{ 1, 2,
 * 3 }` is the initializer.
 *
 * Similarly, it can be used to represent a non-type template parameter
 * declaration, such as the `foo` parameter in:
 *
 *     template<int foo = 10>
 *
 * Where `int` is the type, `foo` is the name and `10` is the
 * initializer.
 *
 * An instance can be used to represent less information dense elements
 * by setting one or more of the fields as the empty string.
 *
 * For example, a template type parameter such as `T` in:
 *
 *     template<typename T = int>
 *
 * Can be represented by an instance that has an empty string as the
 * type, `T` as the name and `int` as the initializer.
 *
 * In general, it can be used to represent any such element that has
 * zero or more of the three components, albeit, in QDoc, it is
 * specifically intended to be used to represent various C++
 * declarations.
 *
 * All three fields are lowered stringified version of the original
 * declaration, so that the type should be used at the end of a
 * pipeline where the semantic property of the represented code are not
 * required.
 */
struct ValuedDeclaration
{
    std::string type;
    std::string name;
    std::string initializer;
};

struct RelaxedTemplateParameter;

struct TemplateDeclarationStorage
{
    std::vector<RelaxedTemplateParameter> parameters;
};

/*
 * Represents a C++ template parameter.
 *
 * The model used by this representation is a slighly simplified
 * model.
 *
 * In the model, template parameters are one of:
 *
 *    - A type template parameter.
 *    - A non type template parameter.
 *    - A template template parameter.
 *
 * Furthermore, each parameter can:
 *
 *     - Be a parameter pack.
 *     - Carry an additional template declaration (as a template template
 *       parameter would).
 *     - Have no declared type.
 *     - Have no declared name.
 *     - Have no declared initializer.
 *
 * Due to this simplified model certain incorrect parameters can be
 * represented.
 *
 * For example, it might be possible to represent a parameter pack
 * that has a default initializer, a non-type template parameter that
 * has no type or a template template parameter that carries no
 * template declaration.
 *
 * The model further elides some of the semantic that might be carried
 * by a parameter.
 * For example, the model has no specific concept for template
 * constraints.
 *
 * Template parameters can be represented as instances of the type.
 *
 * For example, a type template parameter `typename T` can be
 * represented as the following instance:
 *
 * RelaxedTemplateParameter{
 *     RelaxedTemplateParameter::Kind::TypeTemplateParameter,
 *     false,
 *     {
 *         "",
 *         "T",
 *         ""
 *     },
 *     {}
 * };
 *
 * And a non-type template parameter pack "int... Args" as:
 *
 * RelaxedTemplateParameter{
 *     RelaxedTemplateParameter::Kind::NonTypeTemplateParameter,
 *     true,
 *     {
 *         "int",
 *         "Args",
 *         ""
 *     },
 *     {}
 * };
 *
 * Due to the relaxed constraint and the representable incorrect
 * parameters, the type is intended to be used for data that is
 * already validated and known to be correct, such as data that is
 * extracted from Clang.
 */
struct RelaxedTemplateParameter
{
    enum class Kind : std::uint8_t {
        TypeTemplateParameter,
        NonTypeTemplateParameter,
        TemplateTemplateParameter
    };

    Kind kind;
    bool is_parameter_pack;
    ValuedDeclaration valued_declaration;
    std::optional<TemplateDeclarationStorage> template_declaration;
};

/*
 * Represents a C++ template declaration as a collection of template
 * parameters.
 *
 * The parameters for the declaration follow the same relaxed rules as
 * `RelaxedTemplateParameter` and inherit the possibility of
 * representing incorrect declarations.
 *
 * Due to the relaxed constraint and the representable incorrect
 * parameters, the type is intended to be used for data that is
 * already validated and known to be correct, such as data that is
 * extracted from Clang.
 */
struct RelaxedTemplateDeclaration : TemplateDeclarationStorage {};
