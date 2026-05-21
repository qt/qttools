// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

// Synthetic C++ concept declaration for the consumer-side constrained
// item. The matching documentation lives in the upstream module via
// \concept UpstreamThing — see upstream_widgets.qdoc. This C++ concept
// exists only so libclang accepts the requires clause on doStuff; the
// rendered concept reference autolinks to the upstream module's
// documentation page.
template <typename T>
concept UpstreamThing = true;

namespace ConsumerSpace {

class ConsumerClass
{
public:
    ConsumerClass();
    void operate();

    template <typename T> requires UpstreamThing<T>
    void doStuff(T value);
};

} // namespace ConsumerSpace
