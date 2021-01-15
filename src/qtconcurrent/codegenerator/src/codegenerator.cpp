/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
****************************************************************************/
#include "codegenerator.h"
#include <qdebug.h>
namespace CodeGenerator
{

//Convenience constructor so you can say Item("foo")
Item::Item(const char * const text) : generator(Text(QByteArray(text)).generator) {}

int BaseGenerator::currentCount(GeneratorStack * const stack) const
{
    const int stackSize = stack->count();
    for (int i = stackSize - 1; i >= 0; --i) {
        BaseGenerator const * const generator = stack->at(i);
        switch (generator->type) {
            case RepeaterType: {
                RepeaterGenerator const * const repeater = static_cast<RepeaterGenerator const * const>(generator);
                return repeater->currentRepeat;
            } break;
            case GroupType: {
                GroupGenerator const * const group = static_cast<GroupGenerator const * const>(generator);
                return group->currentRepeat;
            } break;
            default:
            break;
        }
    }
    return -1;
}

int BaseGenerator::repeatCount(GeneratorStack * const stack) const
{
    const int stackSize = stack->count();
    for (int i = stackSize - 1; i >= 0; --i) {
        BaseGenerator const * const generator = stack->at(i);
        switch (generator->type) {
            case RepeaterType: {
                RepeaterGenerator const * const repeater = static_cast<RepeaterGenerator const * const>(generator);
                return repeater->currentRepeat;
            } break;
/*            case GroupType: {
                GroupGenerator const * const group = static_cast<GroupGenerator const * const>(generator);
                return group->currentRepeat;
            } break;
*/
            default:
            break;
        }
    }
    return -1;
}

QByteArray RepeaterGenerator::generate(GeneratorStack * const stack)
{
    GeneratorStacker stacker(stack, this);
    QByteArray generated;
    for (int i = repeatOffset; i < repeatCount + repeatOffset; ++i) {
        currentRepeat = i;
        generated += childGenerator->generate(stack);
    }
    return generated;
};

QByteArray GroupGenerator::generate(GeneratorStack * const stack)
{
    const int repeatCount = currentCount(stack);
    GeneratorStacker stacker(stack, this);
    QByteArray generated;

    if (repeatCount > 0)
        generated += prefix->generate(stack);

    for (int i = 1; i <= repeatCount; ++i) {
        currentRepeat = i;
        generated += childGenerator->generate(stack);
        if (i != repeatCount)
            generated += separator->generate(stack);
    }

    if (repeatCount > 0)
        generated += postfix->generate(stack);

    return generated;
};

const Compound operator+(const Item &a, const Item &b)
{
    return Compound(a, b);
}

const Compound operator+(const Item &a, const char * const text)
{
    return Compound(a, Text(text));
}

const Compound operator+(const char * const text, const Item &b)
{
    return Compound(Text(text), b);
}

}