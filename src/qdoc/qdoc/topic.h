// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
#ifndef TOPIC_H
#define TOPIC_H

QT_BEGIN_NAMESPACE

struct Topic
{
public:
    Topic() = default;
    Topic(QString &t, QString a) : m_topic(t), m_args(std::move(a)) { }
    ~Topic() = default;

    [[nodiscard]] bool isEmpty() const { return m_topic.isEmpty(); }
    void clear()
    {
        m_topic.clear();
        m_args.clear();
    }

    QString m_topic {};
    QString m_args {};
};
typedef QList<Topic> TopicList;

QT_END_NAMESPACE

#endif // TOPIC_H
