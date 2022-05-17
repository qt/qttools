// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef RECENTFILES_H
#define RECENTFILES_H

#include <QString>
#include <QStringList>
#include <QTimer>

QT_BEGIN_NAMESPACE

class RecentFiles : public QObject
{
    Q_OBJECT

public:
    explicit RecentFiles(const int maxEntries);

    bool isEmpty() { return m_strLists.isEmpty(); }
    void addFiles(const QStringList &names);
    QString lastOpenedFile() const {
        if (m_strLists.isEmpty() || m_strLists.first().isEmpty())
            return QString();
        return m_strLists.at(0).at(0);
    }
    const QList<QStringList>& filesLists() const { return m_strLists; }

    void readConfig();
    void writeConfig() const;

public slots:
    void closeGroup();

private:
    bool m_groupOpen;
    bool m_clone1st;
    int m_maxEntries;
    QList<QStringList> m_strLists;
    QTimer m_timer;
};

QT_END_NAMESPACE

#endif // RECENTFILES_H
