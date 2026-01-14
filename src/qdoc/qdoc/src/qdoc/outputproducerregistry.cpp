// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "outputproducerregistry.h"

#include "ioutputproducer.h"

QT_BEGIN_NAMESPACE

/*!
    \class OutputProducerRegistry
    \internal
    \brief Singleton registry for discovering output producers by format.

    \sa IOutputProducer, Generator
*/

/*!
    Returns the singleton registry instance.
*/
OutputProducerRegistry &OutputProducerRegistry::instance()
{
    static OutputProducerRegistry registry;
    return registry;
}

/*!
    Registers \a producer with this registry.

    The producer is indexed by its format() identifier, which is
    case-sensitive (see IOutputProducer::format()). If a producer
    with the same format is already registered, it will be replaced.

    The format identifier must remain constant for the lifetime of
    the producer; changing it after registration may prevent successful
    unregistration.

    \sa unregisterProducer()
*/
void OutputProducerRegistry::registerProducer(IOutputProducer *producer)
{
    if (!producer)
        return;

    m_producers.insert(producer->format(), producer);
}

/*!
    Unregisters \a producer from this registry.

    The producer is only removed if it is currently the registered
    producer for its format. This prevents a replaced producer from
    accidentally unregistering its replacement during destruction.

    \sa registerProducer()
*/
void OutputProducerRegistry::unregisterProducer(IOutputProducer *producer)
{
    if (!producer)
        return;

    auto it = m_producers.find(producer->format());
    if (it != m_producers.end() && it.value() == producer)
        m_producers.erase(it);
}

/*!
    Returns the producer registered for \a format, or nullptr if none.
    The \a format comparison is case-sensitive.
*/
IOutputProducer *OutputProducerRegistry::producerForFormat(const QString &format) const
{
    return m_producers.value(format, nullptr);
}

/*!
    Returns all registered producers in unspecified order.
*/
QList<IOutputProducer *> OutputProducerRegistry::allProducers() const
{
    return m_producers.values();
}

QT_END_NAMESPACE

