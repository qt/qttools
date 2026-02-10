// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef OUTPUTPRODUCERREGISTRY_H
#define OUTPUTPRODUCERREGISTRY_H

#include <QtCore/qhash.h>
#include <QtCore/qlist.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class OutputProducer;

class OutputProducerRegistry
{
public:
    static OutputProducerRegistry &instance();

    void registerProducer(OutputProducer *producer);
    void unregisterProducer(OutputProducer *producer);

    [[nodiscard]] OutputProducer *producerForFormat(const QString &format) const;
    [[nodiscard]] QList<OutputProducer *> allProducers() const;

private:
    OutputProducerRegistry() = default;
    ~OutputProducerRegistry() = default;

    OutputProducerRegistry(const OutputProducerRegistry &) = delete;
    OutputProducerRegistry &operator=(const OutputProducerRegistry &) = delete;

    QHash<QString, OutputProducer *> m_producers;
};

QT_END_NAMESPACE

#endif // OUTPUTPRODUCERREGISTRY_H

