// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only


























#include <QtCore/qcoreapplication.h>

@interface NSObject
@end

@interface Foo : NSObject
@property (nonatomic, retain) NSObject *child;
- (bool)someMethodWithArg:(NSObject*)child andAnotherArg:(int)age;
@end

@implementation Foo
- (bool)someMethodWithArg:(NSObject*)child andAnotherArg:(int)age
{
    QCoreApplication::translate("Biz", "Baz");
    return self.child == child && age == 42;
}
@end

int main()
{
    QCoreApplication::translate("Foo", "Bar");
    return 0;
}
