// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only


























// IMPORTANT!!!! If you want to add testdata to this file,
// always add it to the end in order to not change the linenumbers of translations!!!
#include <QtCore>
#include <QtGui>
#include <QtWidgets/QMainWindow>
//
// Test namespace scoping
//

class D : public QObject {
    Q_OBJECT
    public:
    QString foo() {
        return tr("test", "D");
    }

};

namespace A {

    class C : public QObject {
        Q_OBJECT
        public:
        void foo();
    };

    void C::foo() {
        tr("Bla", "A::C");
    }

    void goo() {
        C::tr("Bla", "A::C");       // Is identical to the previous tr(), (same context, sourcetext and comment,
                                    // so it should not add another entry to the list of messages)
    }

    void goo2() {
        C::tr("Bla 2", "A::C");     //Should be in the same namespace as the previous tr()
    }

}


namespace X {

    class D : public QObject {
        Q_OBJECT
        public:

    };

    class E : public QObject {
        Q_OBJECT
        public:
        void foo() { D::tr("foo", "D"); }  // Note that this is X::D from 440 on
    };


    namespace Y {
        class E : public QObject {
            Q_OBJECT

        };

        class C : public QObject {
            Q_OBJECT
            void foo();
        };

        void C::foo() {
            tr("Bla", "X::Y::C");
        }

        void goo() {
            D::tr("Bla", "X::D");   //This should be assigned to the X::D context
        }

        void goo2() {
            E::tr("Bla", "X::Y::E");   //This should be assigned to the X::Y::E context
            Y::E::tr("Bla", "X::Y::E");   //This should be assigned to the X::Y::E context
        }

    }; // namespace Y

    class F : public QObject {
        Q_OBJECT
        inline void inlinefunc() {
            tr("inline function", "X::F");
        }
    };
} // namespace X

namespace ico {
    namespace foo {
        class A : public QObject {
            A();
        };

        A::A() {
            tr("myfoo", "ico::foo::A");
            QObject::tr("task 161186", "QObject");
        }
    }
}

namespace AA {
class C {};
}

/**
 * the context of a message should not be affected by any inherited classes
 *
 * Keep this disabled for now, but at a long-term range it should work.
 */
namespace Gui {
    class MainWindow : public QMainWindow,
                    public AA::C
    {
        Q_OBJECT
public:
        MainWindow()
        {
            tr("More bla", "Gui::MainWindow");
        }

    };
} //namespace Gui


namespace A1 {
    class AB : public QObject {
        Q_OBJECT
        public:

        friend class OtherClass;

        QString inlineFuncAfterFriendDeclaration() const {
            return tr("inlineFuncAfterFriendDeclaration", "A1::AB");
        }
    };
    class B : AB {
        Q_OBJECT
        public:
        QString foo() const { return tr("foo", "A1::B"); }
    };

    // This is valid C++ too....
    class V : virtual AB {
        Q_OBJECT
        public:
        QString bar() const { return tr("bar", "A1::V"); }
    };

    class W : virtual public AB {
        Q_OBJECT
        public:
        QString baz() const { return tr("baz", "A1::W"); }
    };
}

class ForwardDecl;


class B1 : public QObject {
};

class C1 : public QObject {
};

namespace A1 {

class B2 : public QObject {
};

}

void func1()
{
    B1::tr("test TRANSLATOR comment (1)", "B1");

}

using namespace A1;
/*
    TRANSLATOR A1::B2
*/
void func2()
{
    B2::tr("test TRANSLATOR comment (2)", "A1::B2");
    C1::tr("test TRANSLATOR comment (3)", "C1");
}

void func3()
{
    B2::tr("test TRANSLATOR comment (4)", "A1::B2");
}

/*
    TRANSLATOR B2
    This is a comment to the translator.
*/
void func4()
{
    B2::tr("test TRANSLATOR comment (5)", "A1::B2");
}

namespace A1 {
namespace B3 {
class C2 : public QObject {
QString foo();
};
}
}

namespace D1 = A1::B3;
using namespace D1;

// TRANSLATOR A1::B3::C2
QString C2::foo()
{
    return tr("test TRANSLATOR comment (6)", "A1::B3::C2"); // 4.4 screws up
}
namespace Fooish {
  struct toto {
  Q_DECLARE_TR_FUNCTIONS(Bears::And::Spiders)
    QString bar(); };
}

QString Fooish::toto::bar()
{
    return tr("whatever the context", "Bears::And::Spiders");
}


int main(int /*argc*/, char ** /*argv*/) {
    return 0;
}


// QTBUG-140636: Return type vs method qualification disambiguation
// Types from <external_types.h> use angle brackets, so lupdate looks for them in the
// configured paths (and not the relative locations), and if not found, they are not qualified.
// Without the fix, these produce: "Qualifying with unknown namespace/class ::Namespace"
#include <external_types.h>

// Basic namespaced return type
class TestClass1_Widget : public QObject {
    Q_OBJECT
};

Tasking::GroupItem testFunction1()
{
    TestClass1_Widget::tr("test1: basic return type");
}

// Deeply nested return type
namespace MyNS {
class TestClass2 : public QObject {
    Q_OBJECT
public:
    External::Deeply::Nested::Type method();
};
}

External::Deeply::Nested::Type MyNS::TestClass2::method()
{
    tr("test2: nested return type");
    return {};
}

// Trailing return type syntax
class TestClass3 : public QObject {
    Q_OBJECT
public:
    auto method() -> UndefinedNS::RetType;
};

auto TestClass3::method() -> UndefinedNS::RetType
{
    tr("test3: trailing return type");
    return {};
}

// Template return type
#include <vector>
class TestClass4 : public QObject {
    Q_OBJECT
public:
    std::vector<ThirdParty::Item> getItems();
};

std::vector<ThirdParty::Item> TestClass4::getItems()
{
    tr("test4: template return type");
    return {};
}

// Const reference return
class TestClass5 : public QObject {
    Q_OBJECT
public:
    const LibraryNS::Data& getData() const noexcept;
};

const LibraryNS::Data& TestClass5::getData() const noexcept
{
    tr("test5: const reference return");
    static LibraryNS::Data d;
    return d;
}

// Operator overload return type
class TestClass6 : public QObject {
    Q_OBJECT
public:
    OperatorNS::Type operator<<(const OperatorNS::Type& t);
};

OperatorNS::Type TestClass6::operator<<(const OperatorNS::Type& t)
{
    tr("test6: operator overload");
    return t;
}

//#include "main.moc"
