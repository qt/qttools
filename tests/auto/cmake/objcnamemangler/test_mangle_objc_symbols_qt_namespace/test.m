// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#import <Foundation/Foundation.h>
#import <objc/runtime.h>
#import <QtCore/QString>
#import <QtCore/QDebug>

// This test uses a generic class name that would typically have a Qt namespace prefix
// In QT_NAMESPACE mode, the function will read the actual namespace from Qt::Core
__attribute__((objc_root_class))
@interface QtNamespace_TestClass
+ (id)new;
- (void)printClassName;
@end

@implementation QtNamespace_TestClass
+ (id)new {
    return class_createInstance(self, 0);
}
- (void)printClassName {
    const char *name = object_getClassName(self);
    QString className = QString::fromUtf8(name);
    qDebug() << "Class name:" << className;
}
@end

int main(int argc, char *argv[]) {
    QtNamespace_TestClass *tester = [QtNamespace_TestClass new];
    [tester printClassName];
    return 0;
}
