// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef SYNCHRONIZED_H
#define SYNCHRONIZED_H

#include <QtCore/qmutex.h>

#include <atomic>
#include <vector>

QT_BEGIN_NAMESPACE

template<typename T> class WriteSynchronizedRef
{
    Q_DISABLE_COPY_MOVE(WriteSynchronizedRef)

public:
    WriteSynchronizedRef(std::vector<T> &vector) Q_DECL_NOEXCEPT
        : m_vector(vector)
    {}

    void emplace_back(T &&value)
    {
        QMutexLocker lock(&m_mutex);
        m_vector.push_back(std::move(value));
    }

    void emplace_back(const T &value)
    {
        QMutexLocker lock(&m_mutex);
        m_vector.emplace_back(value);
    }

    void emplace_bulk(std::vector<T> && values)
    {
        QMutexLocker lock(&m_mutex);
        if (!m_vector.empty()) {
            m_vector.insert(m_vector.cend(), std::make_move_iterator(values.begin()),
                std::make_move_iterator(values.end()));
        } else {
            m_vector = std::move(values);
        }
    }

private:
    mutable QMutex m_mutex;
    std::vector<T> &m_vector;
};

template<typename T> class ReadSynchronizedRef
{
    Q_DISABLE_COPY_MOVE(ReadSynchronizedRef)

public:
    ReadSynchronizedRef(const std::vector<T> &v) noexcept
        : m_vector(v)
    {}

    size_t size() const
    {
        return m_vector.size();
    }

    bool next(T *value) const
    {
        const auto idx = m_next.fetch_add(1, std::memory_order_acquire);
        const bool hasNext = idx < m_vector.size();
        if (hasNext)
            *value = m_vector[idx];
        return hasNext;
    }

private:
    const std::vector<T> &m_vector;
    mutable std::atomic<size_t> m_next = 0;
};

QT_END_NAMESPACE

#endif
