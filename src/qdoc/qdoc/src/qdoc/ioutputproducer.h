// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef IOUTPUTPRODUCER_H
#define IOUTPUTPRODUCER_H

#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

/*!
    \class IOutputProducer
    \internal
    \brief Interface for documentation output producers.

    IOutputProducer defines the minimal contract for classes that generate
    documentation output. This interface decouples the generation lifecycle
    from any particular implementation, enabling both stateful and stateless
    producers to participate in QDoc's output pipeline.

    Implementations are selected using their format() identifier.

    \sa Generator, TemplateGenerator
*/
class IOutputProducer
{
public:
    virtual ~IOutputProducer() = default;

    /*!
        Returns the format identifier for this producer (e.g., "HTML", "DocBook", "template").
        Used for format selection and matching. The identifier is case-sensitive and must
        match the value used in configuration files.
    */
    [[nodiscard]] virtual QString format() const = 0;

    /*!
        Prepares the producer for an output run.
        Called once per format after configuration is loaded.
    */
    virtual void prepare() = 0;

    /*!
        Produces documentation output.
        Called after prepare() to create the actual documentation files.
    */
    virtual void produce() = 0;

    /*!
        Finalizes output production.
        Called once per format at the end of the run.
    */
    virtual void finalize() = 0;
};

QT_END_NAMESPACE

#endif // IOUTPUTPRODUCER_H

