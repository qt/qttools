// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#import <stdio.h>
#import <objc/runtime.h>

// This class should be excluded from mangling
__attribute__((objc_root_class))
@interface TestClass_Helper
+ (id)new;
- (void)printClassName;
@end

@implementation TestClass_Helper
+ (id)new {
    return class_createInstance(self, 0);
}
- (void)printClassName {
    const char *name = object_getClassName(self);
    printf("Helper: %s\n", name);
}
@end

// This class should be mangled
__attribute__((objc_root_class))
@interface TestClass_Manager
+ (id)new;
- (void)printClassName;
@end

@implementation TestClass_Manager
+ (id)new {
    return class_createInstance(self, 0);
}
- (void)printClassName {
    const char *name = object_getClassName(self);
    printf("Manager: %s\n", name);
}
@end

int main() {
    TestClass_Helper *helper = [TestClass_Helper new];
    [helper printClassName];

    TestClass_Manager *manager = [TestClass_Manager new];
    [manager printClassName];

    return 0;
}
