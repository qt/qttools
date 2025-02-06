// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
#pragma once

#ifdef test_properties
#include <QtCore/qmetaobject.h>
#include <QtCore/qproperty.h>
#include <QtCore/qstring.h>
#endif

#define QDOCTEST_MACRO test
#define QDOCTEST_MACRO2(x) (x) < 0 ? 0 : (x)

namespace TestQDoc {

class Test {
#ifdef test_properties
    Q_OBJECT
    Q_PROPERTY(int id READ id)
#endif
public:

#ifdef test_template
template<typename D, typename T> struct Struct {};
template<typename T>
using Specialized = Struct<int, T>;
#endif

#ifdef test_template
#    define Q_INVOKABLE void foo() {};
#endif

#ifdef test_scopedenum
    enum ClassicEnum { Yee, Haw, Howdy, Partner };

    enum class ScopedEnum : unsigned char {
        This = 0x01,
        That = 0x02,
        All = This | That,
        OmittedValue = 99,
        UselessValue,
        VeryLastValue
    };
#endif
    typedef struct {
        int data;
    } SomeType;
    int someFunction(int, int v = 0);
    void someFunctionDefaultArg(int i, bool b) const;
    void obsoleteMember();
    void anotherObsoleteMember();
    void deprecatedMember();
    void methodWithEnDashInItsDocs();
    void methodWithEmDashInItsDocs();
    void func(bool) {};
    //! [funcPtr]
    void (*funcPtr(bool b, const char *s))(bool) {
        return func;
    }
    //! [op-inc]
    Test &operator++() { return *this; }
    //! [op-dec]
    Test &operator--() { return *this; }

    void anotherFunc() {};
    inline void inlineFunction() {};
    virtual void virtualFun();

    friend bool operator==(const Test &lhs, const Test &rhs) { return false; }

    Test() = delete;
    Test &operator=(Test &&other) = delete;

protected:
    void overload() {}
    void overload(bool b) { if (!b) return; }
#ifdef test_template
    template <typename T1, typename T2> void funcTemplate(T1 a, T2 b) {
        a = b;
    }
#endif
#ifdef test_properties
    virtual int id() { return 0; }
#endif
};

class TestDerived : public Test {
#ifdef test_properties
    Q_OBJECT

    Q_PROPERTY(QString bindableProp READ bindableProp WRITE setBindableProp NOTIFY bindablePropChanged BINDABLE bindableProp)
    Q_PROPERTY(QString someProp READ someProp BINDABLE somBindableProp)
    Q_PROPERTY(int *intProp READ getInt STORED false CONSTANT FINAL)
    Q_PROPERTY(const QString *name READ name)
    QDOC_PROPERTY(bool boolProp READ boolProp WRITE setBoolProp NOTIFY boolPropChanged RESET resetBoolProp REVISION 1)
    QDOC_PROPERTY(bool secondBoolProp READ secondBoolProp NOTIFY boolPropChanged)
#endif

public:

    enum { Val1, Val2 };
    enum { Val3, Val4 };
    using DerivedType = Test::SomeType;
    using NotTypedef = int;
    void virtualFun() override;
    static void staticObsoleteMember();
    NotTypedef someValue();
#ifdef test_properties
    QBindable<QString> bindableProp();
    QBindable<QString> someBindableProp();
    const QString &someProp();
    int *getInt();
    bool boolProp();
    bool secondBoolProp() { return boolProp(); }
    const QString *name() const;

    Q_INVOKABLE void invokeMe() const {}
    int id() override;

Q_SIGNALS:
    void emitSomething(QPrivateSignal);
    void bindablePropChanged();
    Q_REVISION(1) void boolPropChanged();

public Q_SLOTS:
    void setBindableProp(const QString &s);
    void setBoolProp(bool b);
    void resetBoolProp();
#endif
};

#ifdef test_template
template <typename T>
struct BaseVec {};
template <typename T>
using Vec = BaseVec<T>;
#endif

} // namespace TestQDoc

namespace CrossModuleRef {
    void documentMe();
}

class DontLinkToMe {};
