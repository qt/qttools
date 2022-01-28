/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
#pragma once

#ifdef test_properties
#include <QtCore/qmetaobject.h>
#include <QtCore/qproperty.h>
#include <QtCore/qstring.h>
#endif

#define QDOCTEST_MACRO test
#define QDOCTEST_MACRO2(x) (x) < 0 ? 0 : (x))

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
    void someFunctionDefaultArg(int i, bool b);
    void obsoleteMember();
    void anotherObsoleteMember();
    void deprecatedMember();
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
#endif

public:
    using DerivedType = Test::SomeType;
    using NotTypedef = int;
    void virtualFun() override;
    static void staticObsoleteMember();
#ifdef test_properties
    QBindable<QString> bindableProp();
    QBindable<QString> someBindableProp();
    const QString &someProp();
    int *getInt();
    bool boolProp();
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
