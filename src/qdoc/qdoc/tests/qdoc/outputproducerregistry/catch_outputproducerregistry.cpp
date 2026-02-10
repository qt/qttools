// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <catch/catch.hpp>

#include <qdoc/outputproducerregistry.h>
#include <qdoc/outputproducer.h>

#include <QString>
#include <QList>

using namespace Qt::Literals::StringLiterals;

/*!
    \class MockOutputProducer
    \internal
    \brief Test double implementing OutputProducer for registry tests.
*/
class MockOutputProducer : public OutputProducer
{
public:
    explicit MockOutputProducer(const QString &format) : m_format(format) {}

    [[nodiscard]] QString format() const override { return m_format; }
    void prepare() override {}
    void produce() override {}
    void finalize() override {}

private:
    QString m_format;
};

/*!
    \class RegistryGuard
    \internal
    \brief RAII guard ensuring registered producers are unregistered on scope exit.

    Because OutputProducerRegistry is a singleton with no clear() method,
    tests must ensure cleanup regardless of assertion failures or early returns.
    All registration should go through add() to maintain the invariant that
    the guard only tracks what it registers.
*/
class RegistryGuard
{
public:
    explicit RegistryGuard(OutputProducerRegistry &r) : m_registry(r) {}

    ~RegistryGuard()
    {
        for (auto *p : m_producers)
            m_registry.unregisterProducer(p);
    }

    void add(OutputProducer *p)
    {
        if (!p)
            return;
        m_producers.append(p);
        m_registry.registerProducer(p);
    }

private:
    OutputProducerRegistry &m_registry;
    QList<OutputProducer *> m_producers;
};

TEST_CASE("OutputProducerRegistry lookup miss returns nullptr", "[OutputProducerRegistry][Registry]") {
    auto &registry = OutputProducerRegistry::instance();

    OutputProducer *result = registry.producerForFormat("nonexistent-format"_L1);

    REQUIRE(result == nullptr);
}

TEST_CASE("OutputProducerRegistry register and lookup", "[OutputProducerRegistry][Registry]") {
    auto &registry = OutputProducerRegistry::instance();
    MockOutputProducer producer("test-format-1"_L1);
    RegistryGuard guard{registry};
    guard.add(&producer);

    SECTION("Lookup by same format returns the producer") {
        OutputProducer *result = registry.producerForFormat("test-format-1"_L1);
        REQUIRE(result == &producer);
    }

    SECTION("Lookup by different format returns nullptr") {
        OutputProducer *result = registry.producerForFormat("other-format"_L1);
        REQUIRE(result == nullptr);
    }
}

TEST_CASE("OutputProducerRegistry lookup is case-sensitive", "[OutputProducerRegistry][Registry]") {
    auto &registry = OutputProducerRegistry::instance();
    MockOutputProducer producer("HTML"_L1);
    RegistryGuard guard{registry};
    guard.add(&producer);

    SECTION("Exact case matches") {
        OutputProducer *result = registry.producerForFormat("HTML"_L1);
        REQUIRE(result == &producer);
    }

    SECTION("Different case does not match") {
        OutputProducer *result = registry.producerForFormat("html"_L1);
        REQUIRE(result == nullptr);
    }
}

TEST_CASE("OutputProducerRegistry producer replacement", "[OutputProducerRegistry][Registry]") {
    auto &registry = OutputProducerRegistry::instance();
    MockOutputProducer original("replacement-test-format"_L1);
    MockOutputProducer replacement("replacement-test-format"_L1);
    RegistryGuard guard{registry};

    // Register both through guard; second overwrites first in registry
    guard.add(&original);
    guard.add(&replacement);

    SECTION("Replacement is returned for the format") {
        OutputProducer *result = registry.producerForFormat("replacement-test-format"_L1);
        REQUIRE(result == &replacement);
    }

    SECTION("Original cannot unregister the replacement (pointer mismatch protection)") {
        registry.unregisterProducer(&original);

        OutputProducer *result = registry.producerForFormat("replacement-test-format"_L1);
        REQUIRE(result == &replacement);
    }
}

TEST_CASE("OutputProducerRegistry unregister removes entry", "[OutputProducerRegistry][Registry]") {
    auto &registry = OutputProducerRegistry::instance();
    MockOutputProducer producer("unregister-test-format"_L1);
    RegistryGuard guard{registry};
    guard.add(&producer);

    registry.unregisterProducer(&producer);

    OutputProducer *result = registry.producerForFormat("unregister-test-format"_L1);
    REQUIRE(result == nullptr);
}

TEST_CASE("OutputProducerRegistry handles null gracefully", "[OutputProducerRegistry][Registry]") {
    auto &registry = OutputProducerRegistry::instance();

    SECTION("Registering null does not crash") {
        registry.registerProducer(nullptr);
    }

    SECTION("Unregistering null does not crash") {
        registry.unregisterProducer(nullptr);
    }
}

TEST_CASE("OutputProducerRegistry allProducers returns all registered", "[OutputProducerRegistry][Registry]") {
    auto &registry = OutputProducerRegistry::instance();
    MockOutputProducer producer1("all-test-format-1"_L1);
    MockOutputProducer producer2("all-test-format-2"_L1);
    MockOutputProducer producer3("all-test-format-3"_L1);
    RegistryGuard guard{registry};
    guard.add(&producer1);
    guard.add(&producer2);
    guard.add(&producer3);

    QList<OutputProducer *> all = registry.allProducers();

    REQUIRE(all.contains(&producer1));
    REQUIRE(all.contains(&producer2));
    REQUIRE(all.contains(&producer3));
}

