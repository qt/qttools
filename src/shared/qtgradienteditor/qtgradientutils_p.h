// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists for the convenience
// of Qt Designer. This header file may change from version to version
// without notice, or even be removed.
//
// We mean it.
//

#ifndef GRADIENTUTILS_H
#define GRADIENTUTILS_H

#include <QtGui/QGradient>
#include <QtGui/QPainter>

QT_BEGIN_NAMESPACE

class QtGradientManager;

class QtGradientUtils
{
public:
    static QString styleSheetCode(const QGradient &gradient);
    // utils methods, they could be outside of this class
    static QString saveState(const QtGradientManager *manager);
    static void restoreState(QtGradientManager *manager, const QString &state);

    static QPixmap gradientPixmap(const QGradient &gradient, QSize size = QSize(64, 64),
                                  bool checkeredBackground = false);
};

QT_END_NAMESPACE

#endif
