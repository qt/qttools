// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0


























#include <QtCore>

class FooChild
{
    Q_OBJECT
    //: Some comments
    const char *c_noop = QT_TR_NOOP("context FooChild. noop with comments");
};
