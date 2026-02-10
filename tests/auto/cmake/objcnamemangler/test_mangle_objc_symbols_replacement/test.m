// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#import <stdio.h>
#import <objc/runtime.h>

__attribute__((objc_root_class))
@interface TestClass_Sample
+ (id)new;
- (void)printClassName;
@end

@implementation TestClass_Sample
+ (id)new {
    return class_createInstance(self, 0);
}
- (void)printClassName {
    const char *name = object_getClassName(self);
    printf("%s\n", name);
}
@end

int main() {
    TestClass_Sample *tester = [TestClass_Sample new];
    [tester printClassName];
    return 0;
}
