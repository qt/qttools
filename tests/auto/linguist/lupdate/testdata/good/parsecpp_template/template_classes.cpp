#include <QTranslator>
#include <string>
#include <type_traits>


// classes with one named template parameter

template <class T>
class cl1 {
    Q_OBJECT
    template <class Z>
    void func1() {tr("cl1::func1");}
    template <class Z = int>
    void func2() {tr("cl1::func2");}
    template <class Z = std::string>
    void func3() {tr("cl1::func3");}
    template <class Z, class = std::enable_if_t<std::is_same_v<Z, std::string>>>
    void func4() {tr("cl1::func4");}
};

template <class T = int>
class cl2 {
    Q_OBJECT
    template <class Z>
    void func1() {tr("cl2::func1");}
    template <class Z = int>
    void func2() {tr("cl2::func2");}
    template <class Z = std::string>
    void func3() {tr("cl2::func3");}
    template <class Z, class = std::enable_if_t<std::is_same_v<Z, std::string>>>
    void func4() {tr("cl2::func4");}
};

template <class T = std::string>
class cl3 {
    Q_OBJECT
    template <class Z>
    void func1() {tr("cl3::func1");}
    template <class Z = int>
    void func2() {tr("cl3::func2");}
    template <class Z = std::string>
    void func3() {tr("cl3::func3");}
    template <class Z, class = std::enable_if_t<std::is_same_v<Z, std::string>>>
    void func4() {tr("cl3::func4");}
};

template <class T, class = std::enable_if_t<std::is_same_v<T, std::string>>>
class cl4 {
    Q_OBJECT
    template <class Z>
    void func1() {tr("cl4::func1");}
    template <class Z = int>
    void func2() {tr("cl4::func2");}
    template <class Z = std::string>
    void func3() {tr("cl4::func3");}
    template <class Z, class = std::enable_if_t<std::is_same_v<Z, std::string>>>
    void func4() {tr("cl4::func4");}
};

// classes with two named template parameters
template <class T, class Y>
class cl5 {
    Q_OBJECT
    template <class Z>
    void func1() {tr("cl5::func1");}
    template <class Z = int>
    void func2() {tr("cl5::func2");}
    template <class Z = std::string>
    void func3() {tr("cl5::func3");}
    template <class Z, class = std::enable_if_t<std::is_same_v<Z, std::string>>>
    void func4() {tr("cl5::func4");}
};

template <class T, class Y = int>
class cl6 {
    Q_OBJECT
    template <class Z>
    void func1() {tr("cl6::func1");}
    template <class Z = int>
    void func2() {tr("cl6::func2");}
    template <class Z = std::string>
    void func3() {tr("cl6::func3");}
    template <class Z, class = std::enable_if_t<std::is_same_v<Z, std::string>>>
    void func4() {tr("cl6::func4");}
};

template <class T, class Y = std::string>
class cl7 {
    Q_OBJECT
    template <class Z>
    void func1() {tr("cl7::func1");}
    template <class Z = int>
    void func2() {tr("cl7::func2");}
    template <class Z = std::string>
    void func3() {tr("cl7::func3");}
    template <class Z, class = std::enable_if_t<std::is_same_v<Z, std::string>>>
    void func4() {tr("cl7::func4");}
};

template <class T, class Y, class = std::enable_if_t<std::is_same_v<T, std::string>>>
class cl8 {
    Q_OBJECT
    template <class Z>
    void func1() {tr("cl8::func1");}
    template <class Z = int>
    void func2() {tr("cl8::func2");}
    template <class Z = std::string>
    void func3() {tr("cl8::func3");}
    template <class Z, class = std::enable_if_t<std::is_same_v<Z, std::string>>>
    void func4() {tr("cl8::func4");}
};

// classes with one unnamed template parameter
template <class>
class cl9 {
    Q_OBJECT
    template <class Z>
    void func1() {tr("cl9::func1");}
    template <class Z = int>
    void func2() {tr("cl9::func2");}
    template <class Z = std::string>
    void func3() {tr("cl9::func3");}
    template <class Z, class = std::enable_if_t<std::is_same_v<Z, std::string>>>
    void func4() {tr("cl9::func4");}
};

template <class = int>
class cl10 {
    Q_OBJECT
    template <class Z>
    void func1() {tr("cl10::func1");}
    template <class Z = int>
    void func2() {tr("cl10::func2");}
    template <class Z = std::string>
    void func3() {tr("cl10::func3");}
    template <class Z, class = std::enable_if_t<std::is_same_v<Z, std::string>>>
    void func4() {tr("cl10::func4");}
};

template <class = std::string>
class cl11 {
    Q_OBJECT
    template <class Z>
    void func1() {tr("cl11::func1");}
    template <class Z = int>
    void func2() {tr("cl11::func2");}
    template <class Z = std::string>
    void func3() {tr("cl11::func3");}
    template <class Z, class = std::enable_if_t<std::is_same_v<Z, std::string>>>
    void func4() {tr("cl11::func4");}
};

template <class = std::enable_if_t<std::is_same_v<std::string, std::string>>>
class cl12 {
    Q_OBJECT
    template <class Z>
    void func1() {tr("cl12::func1");}
    template <class Z = int>
    void func2() {tr("cl12::func2");}
    template <class Z = std::string>
    void func3() {tr("cl12::func3");}
    template <class Z, class = std::enable_if_t<std::is_same_v<Z, std::string>>>
    void func4() {tr("cl12::func4");}
};

// classes with one named and one unnamed template parameter
template <class T, class>
class cl13 {
    Q_OBJECT
    template <class Z>
    void func1() {tr("cl13::func1");}
    template <class Z = int>
    void func2() {tr("cl13::func2");}
    template <class Z = std::string>
    void func3() {tr("cl13::func3");}
    template <class Z, class = std::enable_if_t<std::is_same_v<Z, std::string>>>
    void func4() {tr("cl13::func4");}
};
template <class T, class = int>
class cl14 {
    Q_OBJECT
    template <class Z>
    void func1() {tr("cl14::func1");}
    template <class Z = int>
    void func2() {tr("cl14::func2");}
    template <class Z = std::string>
    void func3() {tr("cl14::func3");}
    template <class Z, class = std::enable_if_t<std::is_same_v<Z, std::string>>>
    void func4() {tr("cl14::func4");}
};

template <class T, class = std::string>
class cl15 {
    Q_OBJECT
    template <class Z>
    void func1() {tr("cl15::func1");}
    template <class Z = int>
    void func2() {tr("cl15::func2");}
    template <class Z = std::string>
    void func3() {tr("cl15::func3");}
    template <class Z, class = std::enable_if_t<std::is_same_v<Z, std::string>>>
    void func4() {tr("cl15::func4");}
};

template <class T, class = std::enable_if_t<std::is_same_v<std::string, std::string>>>
class cl16 {
    Q_OBJECT
    template <class Z>
    void func1() {tr("cl16::func1");}
    template <class Z = int>
    void func2() {tr("cl16::func2");}
    template <class Z = std::string>
    void func3() {tr("cl16::func3");}
    template <class Z, class = std::enable_if_t<std::is_same_v<Z, std::string>>>
    void func4() {tr("cl16::func4");}
};

// classes in namespaces
namespace ns {
    // classes with one named template parameter
    template <class T>
    class cl1;
    template <class T>
    class cl2;
    template <class T>
    class cl3;
    template <class T, class>
    class cl4;
    // classes with two named template parameters
    template <class T, class Y>
    class cl5;
    template <class T, class Y>
    class cl6;
    template <class T, class Y>
    class cl7;
    template <class T, class Y, class>
    class cl8;
    // classes with one unnamed template parameter
    template <class>
    class cl9;
    template <class>
    class cl10;
    template <class>
    class cl11;
    template <class>
    class cl12;
}

// classes with one named template parameter
template <class T>
class ns::cl1 {
    Q_OBJECT
    template <class Z>
    void func1() {tr("ns::cl1::func1");}
    template <class Z = int>
    void func2() {tr("ns::cl1::func2");}
    template <class Z = std::string>
    void func3() {tr("ns::cl1::func3");}
    template <class Z, class = std::enable_if_t<std::is_same_v<Z, std::string>>>
    void func4() {tr("ns::cl1::func4");}
};

template <class T = int>
class ns::cl2 {
    Q_OBJECT
    template <class Z>
    void func1() {tr("ns::cl2::func1");}
    template <class Z = int>
    void func2() {tr("ns::cl2::func2");}
    template <class Z = std::string>
    void func3() {tr("ns::cl2::func3");}
    template <class Z, class = std::enable_if_t<std::is_same_v<Z, std::string>>>
    void func4() {tr("ns::cl2::func4");}
};

template <class T = std::string>
class ns::cl3 {
    Q_OBJECT
    template <class Z>
    void func1() {tr("ns::cl3::func1");}
    template <class Z = int>
    void func2() {tr("ns::cl3::func2");}
    template <class Z = std::string>
    void func3() {tr("ns::cl3::func3");}
    template <class Z, class = std::enable_if_t<std::is_same_v<Z, std::string>>>
    void func4() {tr("ns::cl3::func4");}
};

template <class T, class = std::enable_if_t<std::is_same_v<T, std::string>>>
class ns::cl4 {
    Q_OBJECT
    template <class Z>
    void func1() {tr("ns::cl4::func1");}
    template <class Z = int>
    void func2() {tr("ns::cl4::func2");}
    template <class Z = std::string>
    void func3() {tr("ns::cl4::func3");}
    template <class Z, class = std::enable_if_t<std::is_same_v<Z, std::string>>>
    void func4() {tr("ns::cl4::func4");}
};

// classes with two named template parameters
template <class T, class Y>
class ns::cl5 {
    Q_OBJECT
    template <class Z>
    void func1() {tr("ns::cl5::func1");}
    template <class Z = int>
    void func2() {tr("ns::cl5::func2");}
    template <class Z = std::string>
    void func3() {tr("ns::cl5::func3");}
    template <class Z, class = std::enable_if_t<std::is_same_v<Z, std::string>>>
    void func4() {tr("ns::cl5::func4");}
};

template <class T, class Y = int>
class ns::cl6 {
    Q_OBJECT
    template <class Z>
    void func1() {tr("ns::cl6::func1");}
    template <class Z = int>
    void func2() {tr("ns::cl6::func2");}
    template <class Z = std::string>
    void func3() {tr("ns::cl6::func3");}
    template <class Z, class = std::enable_if_t<std::is_same_v<Z, std::string>>>
    void func4() {tr("ns::cl6::func4");}
};

template <class T, class Y = std::string>
class ns::cl7 {
    Q_OBJECT
    template <class Z>
    void func1() {tr("ns::cl7::func1");}
    template <class Z = int>
    void func2() {tr("ns::cl7::func2");}
    template <class Z = std::string>
    void func3() {tr("ns::cl7::func3");}
    template <class Z, class = std::enable_if_t<std::is_same_v<Z, std::string>>>
    void func4() {tr("ns::cl7::func4");}
};

template <class T, class Y, class = std::enable_if_t<std::is_same_v<T, std::string>>>
class ns::cl8 {
    Q_OBJECT
    template <class Z>
    void func1() {tr("ns::cl8::func1");}
    template <class Z = int>
    void func2() {tr("ns::cl8::func2");}
    template <class Z = std::string>
    void func3() {tr("ns::cl8::func3");}
    template <class Z, class = std::enable_if_t<std::is_same_v<Z, std::string>>>
    void func4() {tr("ns::cl8::func4");}
};

// classes with one unnamed template parameter
template <class>
class ns::cl9 {
    Q_OBJECT
    template <class Z>
    void func1() {tr("ns::cl9::func1");}
    template <class Z = int>
    void func2() {tr("ns::cl9::func2");}
    template <class Z = std::string>
    void func3() {tr("ns::cl9::func3");}
    template <class Z, class = std::enable_if_t<std::is_same_v<Z, std::string>>>
    void func4() {tr("ns::cl9::func4");}
};

template <class = int>
class ns::cl10 {
    Q_OBJECT
    template <class Z>
    void func1() {tr("ns::cl10::func1");}
    template <class Z = int>
    void func2() {tr("ns::cl10::func2");}
    template <class Z = std::string>
    void func3() {tr("ns::cl10::func3");}
    template <class Z, class = std::enable_if_t<std::is_same_v<Z, std::string>>>
    void func4() {tr("ns::cl10::func4");}
};

template <class = std::string>
class ns::cl11 {
    Q_OBJECT
    template <class Z>
    void func1() {tr("ns::cl11::func1");}
    template <class Z = int>
    void func2() {tr("ns::cl11::func2");}
    template <class Z = std::string>
    void func3() {tr("ns::cl11::func3");}
    template <class Z, class = std::enable_if_t<std::is_same_v<Z, std::string>>>
    void func4() {tr("ns::cl11::func4");}
};

template <class = std::enable_if_t<std::is_same_v<std::string, std::string>>>
class ns::cl12 {
    Q_OBJECT
    template <class Z>
    void func1() {tr("ns::cl12::func1");}
    template <class Z = int>
    void func2() {tr("ns::cl12::func2");}
    template <class Z = std::string>
    void func3() {tr("ns::cl12::func3");}
    template <class Z, class = std::enable_if_t<std::is_same_v<Z, std::string>>>
    void func4() {tr("ns::cl12::func4");}
};

// static functions
template <class T>
void func1() {QT_TRANSLATE_NOOP("global", "func1");}
template <class T = int>
void func2() {QT_TRANSLATE_NOOP("global", "func2");}
template <class T = std::string>
void func3() {QT_TRANSLATE_NOOP("global", "func3");}
template <class T, class = std::enable_if_t<std::is_same_v<T, std::string>>>
void func4() {QT_TRANSLATE_NOOP("global", "func4");}

// parse translation in templates
template <std::size_t N>
struct string_literal
{
    constexpr string_literal(const char (&)[N]) noexcept { }
};

template <string_literal d>
struct A
{
};

struct B : A<QT_TRANSLATE_NOOP("context", "text")>
{
};

struct ClassWithTemplateFn
{
    Q_OBJECT
    template <typename T>
    void fn();
};

template <>
void ClassWithTemplateFn::fn<bool>()
{
    tr("ClassWithTemplateFn");
}

template <typename ClassType>
struct TemplateClassWithTemplateFn
{
    Q_OBJECT
    template <typename T>
    void fn();
};

template <>
template <>
void TemplateClassWithTemplateFn<int>::fn<bool>()
{
    tr("TemplateClassWithTemplateFn");
}

template <typename ClassType>
struct TemplateClassWithFn
{
    Q_OBJECT
    void fn();
};

template <>
void TemplateClassWithFn<int>::fn()
{
    tr("TemplateClassWithFn");
}

namespace ns {

template <typename ClassType>
struct NsTemplateClassWithFn
{
    Q_OBJECT
    void fn();
};

template <typename ClassType>
struct NsTemplateClassWithTemplateFn
{
    Q_OBJECT
    template <typename T>
    void fn();
};

} // namespace ns

template <>
void ns::NsTemplateClassWithFn<int>::fn()
{
    tr("NsTemplateClassWithFn");
}

#include <QDebug>
struct Streamer
{
    QDebug get();
};

template <>
template <>
void ns::NsTemplateClassWithTemplateFn<int>::fn<bool>()
{
    Streamer str;
    str.get() << tr("NsTemplateClassWithTemplateFn");
}
