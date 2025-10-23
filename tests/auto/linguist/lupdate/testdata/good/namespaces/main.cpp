// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only


























#include <QtCore>

class Class : public QObject
{
    Q_OBJECT

    class SubClass
    {
        void f()
        {
            tr("nested class context");
        }
    };

    void f()
    {
        tr("just class context");
    }
};

namespace Outer {

class Class : public QObject { Q_OBJECT };

namespace Middle1 {

class Class : public QObject { Q_OBJECT };

namespace Inner1 {

class Class : public QObject { Q_OBJECT };

}

namespace I = Inner1;

class Something;
class Different;

}

namespace Middle2 {

class Class : public QObject { Q_OBJECT };

namespace Inner2 {

class Class : public QObject { Q_OBJECT };

namespace IO = Middle2;

}

namespace I = Inner2;

}

namespace MI = Middle1::Inner1;

namespace O = ::Outer;

class Middle1::Different : QObject {
Q_OBJECT
    void f() {
        tr("different namespaced class def");
    }
};

}

namespace O = Outer;
namespace OM = Outer::Middle1;
namespace OMI = Outer::Middle1::I;

int main()
{
    Class::tr("outestmost class");
    Outer::Class::tr("outer class");
    Outer::MI::Class::tr("innermost one");
    OMI::Class::tr("innermost two");
    O::Middle1::I::Class::tr("innermost three");
    O::Middle2::I::Class::tr("innermost three b");
    OM::I::Class::tr("innermost four");
    return 0;
}

class OM::Something : QObject {
Q_OBJECT
    void f() {
        tr("namespaced class def");
    }
    void g() {
        tr("namespaced class def 2");
    }
};

// QTBUG-8360
namespace A {

void foo()
{
    using namespace A;
}

QString goo()
{
    return QObject::tr("Bla");
}

}


namespace AA {
namespace B {

using namespace AA;

namespace C {

class Test : public QObject {
    Q_OBJECT
};

}

}

using namespace B;
using namespace C;

void goo()
{
    AA::Test::tr("howdy?");
}

}


namespace A1 {
namespace B {

class Test : public QObject {
    Q_OBJECT
};

using namespace A1;

void foo()
{
    B::B::B::Test::tr("yeeee-ha!");
}

}
}

namespace Ns1::Ns2::Ns3 {
    class Cl: public QObject {
        Q_OBJECT
        void func1();
        void func2();
        void func3();
        void func4();
    };

    // inside namespace

    void Cl::func1() {
        tr("message1");
    }

    void Ns3::Cl::func2() {
        tr("message2");
    }

    void Ns2::Ns3::Cl::func3() {
        tr("message3");
    }

    void Ns1::Ns2::Ns3::Cl::func4() {
        tr("message4");
    }
}

class cl: public QObject {
    Q_OBJECT
    void func();
};

void ::cl::func() {
    tr("text");
}

// using namespace in parent
namespace A1 {
namespace A2 {
class Cl : public QObject
{
    Q_OBJECT
    void func();
};
} // namespace A2
} // namespace A1

using namespace A1::A2;

namespace A1 {
void Cl::func()
{
    tr("using namespace in parent");
}
} // namespace A1

// variable template

namespace NS1 {
namespace NS2 {

template <class T>
struct Cl1
{
    static constexpr bool value = true;
};

template <class T>
constexpr bool varTemplate = NS2::Cl1<T>::value;

} // namespace NS2
} // namespace NS1

class Cl2 : public QObject
{
    Q_OBJECT
    void func() { tr("context after variable template"); }
};

// Direct match in parent namespace must win over using directive match
namespace EdgeTest1 {
namespace Inner {
    class Tr : public QObject { Q_OBJECT };
}
namespace Outer {
    class Tr : public QObject { Q_OBJECT };

    namespace Deep {
        using namespace EdgeTest1::Inner;

        void test()
        {
            Tr::tr("direct parent wins over using");
        }
    }
}
}

// Reusing same using directive for each segment in qualified name
namespace EdgeTest3 {
namespace Level1 {
    namespace Level2 {
        namespace Level3 {
            class Test : public QObject { Q_OBJECT };
        }
    }
}
namespace Container {
    using namespace EdgeTest3::Level1;

    void test()
    {
        Level2::Level3::Test::tr("reuse using per segment");
    }
}
}

// Multiple using directives with same target
namespace EdgeTest4 {
namespace Target {
    class Tr : public QObject { Q_OBJECT };
}
namespace Middle1 {
    using namespace EdgeTest4::Target;
}
namespace Middle2 {
    using namespace EdgeTest4::Target;
}
namespace Container {
    using namespace EdgeTest4::Middle1;
    using namespace EdgeTest4::Middle2;

    void test()
    {
        Tr::tr("prevent duplicate visits");
    }
}
}

// Direct child in current namespace beats using directive in same namespace
namespace EdgeTest5 {
namespace External {
    class Tr : public QObject { Q_OBJECT };
}
namespace MySpace {
    using namespace EdgeTest5::External;
    class Tr : public QObject { Q_OBJECT };

    void test()
    {
        Tr::tr("direct child beats using");
    }
}
}

// Test visitedUsings sharing across loop iterations
// Same using directive at different namespace levels
namespace EdgeTest6 {
namespace Target {
    class Tr : public QObject { Q_OBJECT };
}
namespace Parent {
    using namespace EdgeTest6::Target;

    namespace Child {
        using namespace EdgeTest6::Target;

        void test()
        {
            Tr::tr("same using at different levels");
        }
    }
}
}

// Different paths to same namespace via using directives
namespace EdgeTest7 {
namespace Common {
    class Tr : public QObject { Q_OBJECT };
}
namespace PathA {
    using namespace EdgeTest7::Common;
}
namespace PathB {
    using namespace EdgeTest7::Common;
}
namespace Container {
    using namespace EdgeTest7::PathA;

    namespace Inner {
        using namespace EdgeTest7::PathB;

        void test()
        {
            Tr::tr("different paths to same namespace");
        }
    }
}
}

// Using directive at inner level shouldn't block outer level direct match
namespace EdgeTest8 {
namespace Decoy {
    class Tr : public QObject { Q_OBJECT };
}
namespace Outer {
    class Tr : public QObject { Q_OBJECT };

    namespace Inner {
        using namespace EdgeTest8::Decoy;

        void test()
        {
            Tr::tr("inner using shouldn't block outer direct");
        }
    }
}
}

// Nested using directives with shared visitedUsings
namespace EdgeTest9 {
namespace Level1 {
    class Tr : public QObject { Q_OBJECT };
}
namespace Level2 {
    using namespace EdgeTest9::Level1;
}
namespace Level3 {
    using namespace EdgeTest9::Level2;

    namespace Level4 {
        using namespace EdgeTest9::Level1;

        void test()
        {
            Tr::tr("nested using with shared visited");
        }
    }
}
}

// QTBUG-140550: Class with same name as enclosing namespace
namespace Test {
class Test : public QObject {
    Q_OBJECT
public:
    Test();
};

Test::Test() {
    tr("class same name as namespace");
}
} // namespace Test

// QTBUG-140550: Nested namespaces with same name, class with different name
namespace NsA {
namespace NsA {
namespace NsA {
class MyClass : public QObject {
    Q_OBJECT
public:
    void func1();
    void func2();
};

void MyClass::func1() {
    tr("nested same-name ns, different class 1");
}

void NsA::MyClass::func2() {
    tr("nested same-name ns, different class 2");
}
} // namespace NsA
} // namespace NsA
} // namespace NsA

// QTBUG-140550: Complex nesting - namespace A inside namespace B inside namespace A
namespace Alpha {
namespace Beta {
namespace Alpha {
class Widget : public QObject {
    Q_OBJECT
public:
    void method();
};

void Widget::method() {
    tr("complex nesting different names");
}
} // namespace Alpha
} // namespace Beta
} // namespace Alpha

// Combined bug test: Same-named class with using directive (QTBUG-140548 + QTBUG-140550)
namespace CombinedTest1 {
namespace ExternalLib {
    class Tr : public QObject {
        Q_OBJECT
    public:
        void externalFunc() { tr("external lib tr"); }
    };
}

namespace MyApp {
    using namespace CombinedTest1::ExternalLib;

    // Same name as enclosing namespace AND there's a using directive
    class MyApp : public QObject {
        Q_OBJECT
    public:
        void myFunc()
        {
            // Should resolve to MyApp::MyApp (the class), not ExternalLib::Tr
            tr("combined test: direct class wins");
        }
    };

    namespace Inner {
        void testFunc()
        {
            // Qualified call - should prefer the class with same parent namespace
            MyApp::tr("combined test: qualified same name");
        }
    }
}
}

// Deeply nested same-name namespaces with using directive
namespace CombinedTest2 {
namespace Level {
    namespace Level {
        class Helper : public QObject {
            Q_OBJECT
        public:
            void helperMethod() { tr("helper in Level::Level"); }
        };

        namespace Level {
            using namespace CombinedTest2::Level::Level;

            class Level : public QObject {
                Q_OBJECT
            public:
                void method()
                {
                    // Should resolve to Level::Level::Level::Level (the class)
                    tr("deeply nested same name with using");
                }
            };

            void freeFunc()
            {
                // Should prefer the local class
                Level::tr("qualified deeply nested");

                // Should find Helper via using directive
                Helper::tr("helper via using in nested");
            }
        }
    }
}
}

// Using directive shadowing local namespace
namespace CombinedTest3 {
namespace Container {
    class Data : public QObject {
        Q_OBJECT
    public:
        void process() { tr("data in original container"); }
    };
}

namespace Outer {
    using namespace CombinedTest3::Container;

    namespace Container {
        // Local namespace named Container, but there's also a using directive
        class Item : public QObject {
            Q_OBJECT
        public:
            void method()
            {
                // Should find Item in local Outer::Container
                tr("item in local container");
            }
        };

        void testFunc()
        {
            // Should resolve to local namespace
            Item::tr("qualified local item");

            // Should still find Data via using directive
            Data::tr("data via using directive");
        }
    }
}
}

//#include "main.moc"
