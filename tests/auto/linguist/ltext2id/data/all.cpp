// Copyright (C) 2025 The Qt Company Ltd
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtCore>
QString qt_detectRTLLanguage()
{
    return QCoreApplication::tr(
            "QT_LAYOUT_DIRECTION",
            "Translate this string to the string 'LTR' in left-to-right"
            " languages or to 'RTL' in right-to-left languages (such as Hebrew"
            " and Arabic) to get proper widget layout."); // == QLatin1String("RTL");
}

class Dialog2
{
    Q_OBJECT
    void func();
    void func3();
    int getCount() const { return 2; }
};

void Dialog2::func()
{
    int n = getCount();
    tr("%n files", "plural form", n);
    tr("%n cars", 0, n);
    tr("&Find %n cars", 0, n);
    tr("Search in %n items?", 0, n);
    tr("%1. Search in %n items?", 0, n);
    tr("Age: %1");
    tr("There are %n house(s)", "Plurals and function call", getCount());
    QCoreApplication::translate("translate: There are %n house(s)",
                                "Plurals and function call", nullptr, getCount());
    QCoreApplication::translate("Plurals, QCoreApplication", "%n house(s)",
                                "Plurals and identifier", n);
    QCoreApplication::translate("Plurals, QCoreApplication", "%n car(s)",
                                "Plurals and literal number", 1);
    QCoreApplication::translate("Plurals, QCoreApplication", "%n horse(s)",
                                "Plurals and function call", getCount());
    QTranslator trans;
    trans.translate("QTranslator", "Simple");
    trans.translate("QTranslator", "Simple", 0);
    trans.translate("QTranslator", "Simple with comment", "with comment");
    trans.translate("QTranslator", "Plural without comment", 0, 1);
    trans.translate("QTranslator", "Plural with comment", "comment 1", n);
    trans.translate("QTranslator", "Plural with comment", "comment 2", getCount());
    trans.translate("QTranslator", "Plural with comment and static cast", "comment 3",
                    static_cast<long>(getCount()));
}

typedef struct S_
{
    int a;
} S, *SPtr;
class ForwardDecl;

#define FT_DEFINE_SERVICE(name)                                      \
    typedef struct FT_Service_##name##Rec_ FT_Service_##name##Rec;   \
    typedef struct FT_Service_##name##Rec_ const *FT_Service_##name; \
    struct FT_Service_##name##Rec_

void Dialog2::func3()
{
    tr("func3");
}

namespace Gui {
class BaseClass
{
};
} // namespace Gui

class TestClass : QObject
{
    Q_OBJECT

    inline QString inlineFunc1() { return tr("inline function", "TestClass"); }

    QString inlineFunc2() { return tr("inline function 2", "TestClass"); }

    static inline QString staticInlineFunc() { return tr("static inline function", "TestClass"); }

    class NoQObject : public Gui::BaseClass
    {
    public:
        inline QString hello() { return QString("hello"); }
    };
};

class Testing : QObject
{
    Q_OBJECT

    inline QString f1()
    {
        //: this is an extra comment for the translator
        return tr("extra-commented string");
        return tr("not extra-commented string");
        /*: another extra-comment */
        return tr("another extra-commented string");
        /*: blah! */
        return QCoreApplication::translate("scope", "works in translate, too", "blabb", 0);
    }
};

//: extra comment for NOOP
//: which spans multiple lines
const char *c_1 = QT_TRANSLATE_NOOP(
        "scope",
        "string") /*: complain & ignore */; // 4.4 says the line of this is at the next statement
//: extra comment for NOOP
const char *c_2[2] = QT_TRANSLATE_NOOP3_UTF8("scope", "string", "comment"); // 4.4 doesn't see this

const char *c_3 = QT_TRANSLATE_NOOP("scope",
                                    "string " // this is an interleaved comment
                                    "continuation on next line");

class TestingTake17 : QObject
{
    Q_OBJECT

    void function(void)
    {
        //: random comment
        //~ meta-id this_is_an_id
        //~ loc-layout_id fooish_bar
        //~ po-ignore_me totally foo-barred  nonsense
        tr("something cool");

        tr("less cool");

        //~ meta-id another_id
        tr("even more cool");
    }
};

//: again an extra comment, this time for id-based NOOP
//% "This is supposed\tto be quoted \" newline\n"
//% "backslashed \\ stuff."
const char *c_4 = QT_TRID_NOOP("this_a_id");

//~ some thing
//% "This needs to be here. Really."
QString test = qtTrId("this_another_id", 2);

class YetAnotherTest : QObject
{
    Q_OBJECT

    void function(void)
    {
        //
        //:
        //=
        //~
        // #
        //=============
        //~~~~~~~~~~~~~
        //:::::::::::::
        tr("nothing");
    }
};

class Bogus : QObject
{
    Q_OBJECT
    static const char *const s_stringss[];
    static const char *const s_strings[];
};

const char *const Bogus::s_strings[] = { QT_TR_NOOP("this should be in Bogus") };

const char *const Bogus::s_stringss[] = { QT_TR_NOOP("this should be in Bogus") };

void bogosity()
{
    // no spaces here. test collateral damage from ignoring equal sign
    QString toto = QObject::tr("just QObject");
}

namespace Internal {

class Message : public QObject
{
    Q_OBJECT
public:
    Message(QObject *parent = 0);
};

} // namespace Internal

namespace Internal {

static inline QString message1()
{
    return Message::tr("message1"); // Had no namespace
}

static inline QString message2()
{
    return Message::tr("message2"); // Already had namespace
}

} // namespace Internal

class LotsaFun : public QObject
{
    Q_OBJECT
public:
    LotsaFun *operator<<(int i);
};

LotsaFun *LotsaFun::operator<<(int i)
{
    tr("this is inside operator<<");
    return this;
}

namespace NameSchpace {

class YetMoreFun : public QObject
{
    Q_OBJECT
public:
    void funStuff();
};

} // namespace NameSchpace

namespace NameSchpace {

#define somevar 1

void YetMoreFun::funStuff()
{
    tr("funStuff!");
}

} // namespace NameSchpace

void blubb()
{
    QMap<QString, QString> d;
    d[LotsaFun::tr("bracketed")] = "plain";
}

class TestClass2
{
    Q_DECLARE_TR_FUNCTIONS(TestClass);

public:
    static const char TEST_STRING[];
};

const char TestClass2::TEST_STRING[] = QT_TR_NOOP("Test value");

class Class42 : public NameSchpace::YetMoreFun, Gui::BaseClass
{
    Q_OBJECT
    void foo();
    Class42() : NameSchpace::YetMoreFun(), Gui::BaseClass() { tr("does that make sense?"); }
    void hello(int something, QString str);
};

void Class42::foo()
{
    tr("and does that?");
}

void Class42::hello(int something /*= 17 */, QString str = Class42::tr("eyo")) { }

#include "included.cpp"

// test TR_EXCLUDE
#include "notincluded.cpp"

void dupeFail()
{
    // First just the Id.
    qtTrId("dupe_id");

    // Then with source
    //% "This is the source"
    qtTrId("dupe_id");

    // Finally, same source, but without ID.
    QCoreApplication::translate("", "This is the source");
}

namespace Abc {
class NamespacedFinalClass;
}

class FinalClass final : public QObject
{
    Q_OBJECT

    class SubClass final
    {
        void f() { tr("nested class context with final"); }
    };

    void f() { tr("class context with final"); }
};

class Abc::NamespacedFinalClass final : public QObject
{
    Q_OBJECT

    void f() { tr("namespaced class with final"); }
};

void nullptrInPlural()
{
    QObject::tr("%n nullptr(s)", nullptr, 3);
    QCoreApplication::translate("Plurals, nullptr", "%n car(s) text", nullptr, 1);
}

class nullptrClass : public QObject
{
    Q_OBJECT

    void f() { tr("%n car(s)", nullptr, 2); }
};

void nullMacroInPlural()
{
    QObject::tr("%n NULL(s)", NULL, 3);
    QObject::tr("%n Q_NULLPTR(s)", Q_NULLPTR, 3);
}

class ListInitializationClass : public NameSchpace::YetMoreFun, Gui::BaseClass
{
    Q_OBJECT

    ListInitializationClass()
        : NameSchpace::YetMoreFun(), 
        Gui::BaseClass{}, 
        a{ 0 }, 
        b(1), 
        c(tr("Hello World"))
    {
        tr("ListInitializationClass in-class constructor");
    }

    ListInitializationClass(int a);

    ListInitializationClass(int a, int b, int c);

    int a;
    int b;
    QString c;
};

ListInitializationClass::ListInitializationClass(int a) // :
//    b{ { 2, 3 }}[a]
{
    tr("ListInitializationClass out-of-class single member initializer");
}

ListInitializationClass::ListInitializationClass(int a, int b, int c)
    : NameSchpace::YetMoreFun{},
      Gui::BaseClass(),
      a{ 2 + (a / 3) },
      b(b),
      c{ tr("%n item(s)", Q_NULLPTR, c) }
{
    tr("ListInitializationClass out-of-class multi member initializer");
}

class LambdaMemberClass : public Gui::BaseClass
{
    Q_OBJECT

    LambdaMemberClass()
        : Gui::BaseClass(),
          a{ []() { /*std::cout << */
                    QObject::tr("Hello");
          } },
          b([]() { /*std::cout << "World\n";*/ })
    {
        tr("LambdaMemberClass in-class constructor");
    }

    LambdaMemberClass(void *);

    std::function<void()> a;
    std::function<void()> b;
};

LambdaMemberClass::LambdaMemberClass(void *)
    : Gui::BaseClass{},
      a([]() { /*std::cout <<*/
               QObject::tr("Hallo ");
      }),
      b{ []() { /*std::cout << "Welt\n";*/ } }
{
    tr("LambdaMemberClass out-of-class constructor");
}

// Template parameters in base class initialization
class TemplateClass : QVarLengthArray<char, sizeof(std::size_t)>, std::vector<int>
{
    Q_DECLARE_TR_FUNCTIONS(TemplateClass)
    QString member;

public:
    TemplateClass()
        : QVarLengthArray<char, sizeof(std::size_t)>(),
          std::vector<int>(3),
          member(tr("TemplateClass() in-class member initialization"))
    {
        tr("TemplateClass() in-class body");
    }

    TemplateClass(void *);
    TemplateClass(int);
};

TemplateClass::TemplateClass(void *)
    : QVarLengthArray<char, sizeof(std::size_t)>(),
      std::vector<int>{ 1, 2 },
      member{ tr("TemplateClass(void *) out-of-class member initialization") }
{
    tr("TemplateClass(void *) out-of-class body");
}

void ternary()
{
    const auto aaa = true ?
                          //: comment, aaa, true
            QObject::tr("ternary, true, aaa")
                          : QObject::tr("ternary, failure, aaa");

    const auto bbb = true ?
                          //: comment, bbb, true
            QObject::tr("ternary, bbb, true")
                          :
                          //: comment, bbb, false
            QObject::tr("ternary, bbb, false");

    const auto ccc = true ? QObject::tr("ternary, fff, true"): QObject::tr("ternary, fff, false");
}

class TernaryClass : public QObject
{
    Q_OBJECT

    void f()
    {
        const auto ccc = true ?
                              //: comment, ccc, true
                tr("ternary, ccc, true")
                              : tr("ternary, ccc, false");

        const auto ddd = true ?
                              //: comment, ddd, true
                tr("ternary, ddd, true")
                              :
                              //: comment, ddd, false
                tr("ternary, ddd, false");
    }
};

namespace Private {
template <class T>
struct Class1
{
    T t;
};
template <class T>
struct Class1<T &> : Class1<T>
{
};
template <class T>
struct Class2
{
    enum { Value = sizeof(T) };
};
} // namespace Private
class TranslatedAfterPrivate
{
    Q_OBJECT
    TranslatedAfterPrivate() { tr("Must be in context TranslatedAfterPrivate"); }
};

#include <QObject>
class AClass
{
    QString aa = QObject::tr("message after system include without space");
};
#include "qobject.h"
class AAClass
{
    QString aa = QObject::tr("message after local include without space");
};

QString unicodeEscape()
{
    return QCoreApplication::tr("Context", "soft\u00ADhyphen");
}

namespace Outer::Inner {

class Class
{
    Q_OBJECT
    void function() { tr("MoreFunStuff!"); }
};

} // namespace Outer::Inner

static const char *const test_string_n1[] = { QT_TRANSLATE_N_NOOP("scope", "string %n") };

static const char *const test_string_n2[] = QT_TRANSLATE_N_NOOP3("scope", "string %n", "comment");
class testing
{
    Q_OBJECT
    void test();
};
void testing::test()
{
    static const char *const test_string_n3[] = { QT_TR_N_NOOP("%n test") };
}

class Hogus : QObject
{
    Q_OBJECT
    static const QString myString;
};

const QString Hogus::myString(QT_TR_NOOP("this should be in Hogus"));

class QTBUG99415 : QObject
{
    Q_OBJECT
    const QString text1() const noexcept { return tr("text1"); }
    const QString text2() const noexcept;
};

const QString QTBUG99415::text2() const noexcept
{
    return tr("text2");
}

class QTBUG110630 : QObject
{
    Q_OBJECT
    const QString txt()
    {
        //~ quoted " string with spaces "
        tr("translation with extras-quoted field");
    }
};

// enum class - C++11
enum class Bar : unsigned short;
class QTBUG36589 : QObject
{
    Q_OBJECT
    const QString txt() { tr("string after an enum class");
        tr("tr with a \"string\" inside it");
        tr("another tr with a \'string\' inside it");
        tr("yet another tr with a 'string' inside it");
    }
};

class TestMetaStrings : QObject
{
    Q_OBJECT
    void func()
    {
        //~ meta-id id1
        //@ label1
        tr("msg with id1");

        //@ label3
        //% "propagating label"
        qtTrId("id2");

        qtTrId("id2");

        //@ label5
        //% "source"
        qtTrId("id4");
        

        tr("This is going to be a meta string spanning over"
                "\nmultiple lines.");
    }
};

struct Tr
{
    Q_DECLARE_TR_FUNCTIONS(QtC::Help)
};

void observedBugs() {
     m_includeOldEntriesAction->setToolTip(
                Tr::tr("Include branches and tags that have not been active for %n days.", nullptr,
                   Constants::OBSOLETE_COMMIT_AGE_IN_DAYS));

     Tr::tr("[Only %n MB of output shown]", nullptr, maxSize / 1024 / 1024);
}
