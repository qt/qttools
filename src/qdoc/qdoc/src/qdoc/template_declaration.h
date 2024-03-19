// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <numeric>
#include <optional>
#include <string>
#include <vector>

#include <QString>

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
    struct PrintingPolicy
    {
        bool include_type = true;
        bool include_name = true;
        bool include_initializer = true;
    };

    std::string type;
    std::string name;
    std::string initializer;

    // KLUDGE: Workaround for
    // https://stackoverflow.com/questions/53408962/try-to-understand-compiler-error-message-default-member-initializer-required-be
    static PrintingPolicy default_printing_policy() { return PrintingPolicy{}; }

    /*
     * Constructs and returns a human-readable representation of this
     * declaration.
     *
     * The constructed string is formatted so that as to rebuild a
     * possible version of the C++ code that is modeled by an instance
     * of this type.
     *
     * Each component participates in the human-presentable version if
     * it they are not the empty string.
     *
     * The "type" and "name" component participate with their literal
     * representation.
     *
     * The "iniitlalizer" components contributes an equal symbol,
     * followed by a space followed by the literal representation of
     * the component.
     *
     * The component contributes in an ordered way, with "type"
     * contributing first, "name" contributing second and
     * "initializer" contributing last.
     *
     * Each contribution is separated by a space if the component that
     * comes before it, if any, has contributed to the human-readable
     * representation.
     *
     * For example, an instance of this type that has "type" component
     * "int", "name" component "foo" and "iniitializer" component
     * "100", would be represented as:
     *
     *  int foo = 100
     *
     *  Where "int" is the "type" component contribution, "foo" is the
     *  "name" component contribution and "= 100" is the "initializer"
     *  component contribution.
     *  Each of those contribution is separated by a space, as each
     *  "preceding" component has contributed to the representation.
     *
     *  If we provide a similar instance with, for example, the "type"
     *  and "name" components as the empty string, then the
     *  representation would be "= 100", which is the "initializer"
     *  component contribution, the only component that is not the
     *  empty string.
     *
     *  The policy argument allows to treat certain components as if
     *  they were the empty string.
     *
     *  For example, given an instance of this type that has "type"
     *  component "double", "name" component "bar" and "iniitializer"
     *  component "10.2", its human-readable representation would be
     *  "double bar = 10.2".
     *
     *  If the representation of that same instance was obtained by
     *  using a policy that excludes the "name" component, then that
     *  representation would be "double = 10.2", which is equivalent
     *  to the representation of an instance that is the same as the
     *  orginal one with the "name" component as the empty string.
     */
    inline std::string to_std_string(PrintingPolicy policy = default_printing_policy()) const
    {
        std::string s{};

        if (!type.empty() && policy.include_type)
            s += (s.empty() ? "" : " ") + type;

        if (!name.empty() && policy.include_name)
            s += (s.empty() ? "" : " ") + name;

        if (!initializer.empty() && policy.include_initializer)
            s += (s.empty() ? "= " : " = ") + initializer;

        return s;
    }
};

struct RelaxedTemplateParameter;

struct TemplateDeclarationStorage
{
    std::vector<RelaxedTemplateParameter> parameters;

    inline std::string to_std_string() const;
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

    /*
     * Constructs and returns a human-readable representation of this
     * parameter.
     *
     * The constructed string is formatted so that as to rebuild a
     * possible version of the C++ code that is modeled by an instance
     * of this type.
     *
     * The format of the representation varies based on the "kind" of
     * the parameter.
     *
     *   - A "TypeTemplateParameter", is constructed as the
     *     concatenation of the literal "typename", followed by the
     *     literal "..." if the parameter is a pack, followed by the
     *      human-readable representaion of "valued_declaration".
     *
     *      If the human-readable representation of
     *      "valued_declaration" is not the empty string, it is
     *      preceded by a space when it contributes to the
     *      representation.
     *
     *      For example, the C++ type template parameter "typename Foo
     *      = int", would be represented by the instance:
     *
     *        RelaxedTemplateParameter{
     *            RelaxedTemplateParameter::Kind::TypeTemplateParameter,
     *            false,
     *            {
     *                "",
     *                "Foo",
     *                "int"
     *            },
     *            {}
     *        };
     *
     *      And its representation would be:
     *
     *        typename Foo = int
     *
     *      Where "typename" is the added literal and "Foo = int" is
     *      the representation for "valued_declaration", with a space
     *      in-between the two contributions.
     *
     *    - A "NonTypeTemplateParameter", is constructed by the
     *      contribution of the "type" compoment of "valued_declaration",
     *      followed by the literal "..." if the parameter is a pack,
     *      followed by the human-presentable version of
     *      "valued_declaration" without its "type" component
     *      contribution.
     *
     *      If the contribution of the "type" component of
     *      "valued_declaration" is not empty, the next contribution is
     *      preceded by a space.
     *
     *      For example, the C++ non-type template parameter "int...
     *      SIZE", would be represented by the instance:
     *
     *
     *        RelaxedTemplateParameter{
     *            RelaxedTemplateParameter::Kind::NonTypeTemplateParameter,
     *            true,
     *            {
     *                "int",
     *                "SIZE",
     *                ""
     *            },
     *            {}
     *        };
     *
     *      And its representation would be:
     *
     *        int... SIZE
     *
     *      Where "int" is the "type" component contribution of
     *      "valued_declaration", "..." is the added literal due to
     *      the parameter being a pack and " SIZE" being the
     *      human-readable representation of "valued_declaration"
     *      without its "type" component contribution, preceded by a
     *      space.
     *
     *    - A "TemplateTemplateParameter", is constructed by the
     *      contribution of the human-presentable representation of
     *      "template_declaration", followed by the representation of
     *      this parameter if it was a "TypeTemplateParameter", with a
     *      space between the two contributions if the
     *      human-presentable representation of "template_declaration"
     *      is not empty.
     *
     *      For example, the C++ template template template parameter
     *      "template<typename> T", would be represented by the
     *      instance:
     *
     *
     *        RelaxedTemplateParameter{
     *            RelaxedTemplateParameter::Kind::TemplateTemplateParameter,
     *            false,
     *            {
     *                "",
     *                "T",
     *                ""
     *            },
     *            {
     *              RelaxedTemplateParameter{
     *                RelaxedTemplateParameter::Kind::TypeTemplateParameter,
     *                false,
     *                {
     *                    "",
     *                    "",
     *                    ""
     *                },
     *                {}
     *              }
     *            }
     *        };
     *
     *      And its representation would be:
     *
     *        template <typename> typename T
     *
     *      Where "template <typename>" human-presentable version of
     *      "template_declaration" and "typename T" is the
     *      human-presentable version of this parameter if it was a
     *      type template parameter.
     *
     *      With a space between the two contributions.
     */
    inline std::string to_std_string() const
    {
        switch (kind) {
        // TODO: This can probably be moved under the template
        // template parameter case and reused through a fallback.
        case Kind::TypeTemplateParameter: {
            std::string valued_declaration_string = valued_declaration.to_std_string();

            return std::string("typename") + (is_parameter_pack ? "..." : "")
                    + (valued_declaration_string.empty() ? "" : " ") + valued_declaration_string;
        }
        case Kind::NonTypeTemplateParameter: {
            std::string type_string = valued_declaration.type + (is_parameter_pack ? "..." : "");

            return type_string + (type_string.empty() ? "" : " ")
                    + valued_declaration.to_std_string(
                            ValuedDeclaration::PrintingPolicy{ false, true, true });
        }
        case Kind::TemplateTemplateParameter: {
            std::string valued_declaration_string = valued_declaration.to_std_string();

            return (template_declaration ? (*template_declaration).to_std_string() + " " : "")
                    + "typename" + (is_parameter_pack ? "..." : "")
                    + (valued_declaration_string.empty() ? "" : " ") + valued_declaration_string;
        }
        default:
            return "";
        }
    }
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
struct RelaxedTemplateDeclaration : TemplateDeclarationStorage
{
    inline QString to_qstring() const { return QString::fromStdString(to_std_string()); }
};

/*
 * Constructs and returns a human-readable representation of this
 * declaration.
 *
 * The constructed string is formatted so as to rebuild a
 * possible version of the C++ code that is modeled by an instance
 * of this type.
 *
 * The representation of a declaration is constructed by the literal
 * "template <", followed by the human-presentable version of each
 * parameter in "parameters", with a comma and a space between each
 * parameter, followed by a closing literal ">".
 *
 * For example, the empty declaration is represented as "template <>".
 *
 * While a template declaration that has a type template parameter
 * "Foo" with initializer "int" and a non-type template parameter pack
 * with type "int" and name "S" would be represented as:
 *
 *   template <typename Foo = int, int... S>
 */
inline std::string TemplateDeclarationStorage::to_std_string() const
{
    if (parameters.empty())
        return "template <>";

    return "template <"
            + std::accumulate(std::next(parameters.cbegin()), parameters.cend(),
                              parameters.front().to_std_string(),
                              [](auto &&acc, const RelaxedTemplateParameter &parameter) {
                                  return acc + ", " + parameter.to_std_string();
                              })
            + ">";
}

/*
 * Returns true if the two template declaration represented by left
 * and right are substitutable.
 *
 * QDoc uses a simplified model for template declarations and,
 * similarly, uses a simplified model of "substitutability".
 *
 * Two declarations are substitutable if:
 *
 *  - They have the same amount of parameters
 *  - For each pair of parameters with the same postion:
 *    - They have the same kind
 *    - They are both parameter packs or both are not parameter packs
 *    - If they are non-type template parameters then they have the same type
 *    - If they are both template template parameters then they both
 *      carry an additional template declaration and the additional
 *      template declarations are substitutable
 *
 *  This means that in the simplified models, we generally ignore default arguments, name and such.
 *
 *  This model does not follow the way C++ performs disambiguation but
 *  should be enough to handle most cases in the documentation.
 */
inline bool are_template_declarations_substitutable(const TemplateDeclarationStorage& left, const TemplateDeclarationStorage& right) {
    static auto are_template_parameters_substitutable = [](const RelaxedTemplateParameter& left, const RelaxedTemplateParameter& right) {
        if (left.kind != right.kind) return false;
        if (left.is_parameter_pack != right.is_parameter_pack) return false;

        if (left.kind == RelaxedTemplateParameter::Kind::NonTypeTemplateParameter &&
            (left.valued_declaration.type != right.valued_declaration.type))
            return false;

        if (left.kind == RelaxedTemplateParameter::Kind::TemplateTemplateParameter) {
            if (!left.template_declaration && right.template_declaration) return false;
            if (left.template_declaration && !right.template_declaration) return false;

            if (left.template_declaration && right.template_declaration)
                return are_template_declarations_substitutable(*left.template_declaration, *right.template_declaration);
        }

        return true;
    };

    const auto& left_parameters = left.parameters;
    const auto& right_parameters = right.parameters;

    if (left_parameters.size() != right_parameters.size()) return false;

    return std::transform_reduce(left_parameters.cbegin(), left_parameters.cend(), right_parameters.cbegin(),
        true,
        std::logical_and<bool>{},
        are_template_parameters_substitutable
    );
}
