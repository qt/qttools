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
#define QDOCTEST_MACRO test
#define QDOCTEST_MACRO2(x) (x) < 0 ? 0 : (x))

namespace TestQDoc {

class Test {
public:

#ifdef test_template
template<typename D, typename T> struct Struct {};
template<typename T>
using Specialized = Struct<int, T>;
#endif

#ifdef test_scopedenum
    enum class ScopedEnum : unsigned char {
        This = 0x01,
        That = 0x02,
        All = This | That,
        OmittedValue = 99
    };
#endif
    typedef struct {
        int data;
    } SomeType;
    int someFunction(int v = 0);
    void someFunctionDefaultArg(int i, bool b);
    void obsoleteMember();
    void anotherObsoleteMember();
    void deprecatedMember();
    inline void inlineFunction() {};
    virtual void virtualFun();

protected:
    void overload() {}
    void overload(bool b) { if (!b) return; }
#ifdef test_template
    template <typename T1, typename T2> void funcTemplate(T1 a, T2 b) {
        a = b;
    }
#endif
};

class TestDerived : public Test {
public:
    using DerivedType = Test::SomeType;
    using NotTypedef = int;
    void virtualFun() override;
};

} // namespace TestQDoc
