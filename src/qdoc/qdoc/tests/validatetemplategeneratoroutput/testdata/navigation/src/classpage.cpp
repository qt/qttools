// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "classpage.h"

/*!
    \class NavWidget
    \inmodule TestNavigation
    \brief A widget for navigation testing.

    The NavWidget class demonstrates breadcrumb chains for C++ class pages
    and sidebar TOC rendering with multiple section levels.

    \section1 Basic Usage

    Create a NavWidget and call navigate().

    \section2 Configuration

    Configuration is straightforward.

    \section3 Advanced Options

    For advanced use cases, subclass NavWidget.

    \section1 Thread Safety

    NavWidget is reentrant but not thread-safe.

    \section1 Performance

    Performance depends on navigation target complexity.

    \section2 Benchmarks

    Benchmarks show sub-millisecond navigation times.
*/

/*!
    Constructs a NavWidget.
*/
NavWidget::NavWidget() = default;

/*!
    Navigates to the specified \a target.
*/
void NavWidget::navigate(const QString &target)
{
    Q_UNUSED(target);
}
