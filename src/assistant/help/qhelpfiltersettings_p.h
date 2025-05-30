// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QHELPFILTERSETTINGS_H
#define QHELPFILTERSETTINGS_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists for the convenience
// of the help generator tools. This header file may change from version
// to version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/QSharedDataPointer>

QT_BEGIN_NAMESPACE

template <class K, class T>
class QMap;
class QHelpFilterData;
class QHelpFilterEngine;
class QHelpFilterSettingsPrivate;

class QHelpFilterSettings final
{
public:
    QHelpFilterSettings();
    QHelpFilterSettings(const QHelpFilterSettings &other);
    QHelpFilterSettings(QHelpFilterSettings &&other);
    ~QHelpFilterSettings();

    QHelpFilterSettings &operator=(const QHelpFilterSettings &other);
    QHelpFilterSettings &operator=(QHelpFilterSettings &&other);

    void swap(QHelpFilterSettings &other) noexcept
    { d.swap(other.d); }

    void setFilter(const QString &filterName, const QHelpFilterData &filterData);
    void removeFilter(const QString &filterName);
    QStringList filterNames() const;
    QHelpFilterData filterData(const QString &filterName) const;
    QMap<QString, QHelpFilterData> filters() const;

    void setCurrentFilter(const QString &filterName);
    QString currentFilter() const;

    static QHelpFilterSettings readSettings(const QHelpFilterEngine *filterEngine);
    static bool applySettings(QHelpFilterEngine *filterEngine, const QHelpFilterSettings &settings);

private:
    QSharedDataPointer<QHelpFilterSettingsPrivate> d;
};

QT_END_NAMESPACE

#endif // QHELPFILTERSETTINGS_H
