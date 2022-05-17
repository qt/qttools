// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qhelpfiltersettings_p.h"
#include "qhelpfilterdata.h"

#include <QtCore/QMap>
#include <QtHelp/QHelpFilterEngine>

QT_BEGIN_NAMESPACE

class QHelpFilterSettingsPrivate : public QSharedData
{
public:
    QHelpFilterSettingsPrivate() = default;
    QHelpFilterSettingsPrivate(const QHelpFilterSettingsPrivate &other) = default;
    ~QHelpFilterSettingsPrivate() = default;

    QMap<QString, QHelpFilterData> m_filterToData;
    QString m_currentFilter;
};

QHelpFilterSettings::QHelpFilterSettings()
    : d(new QHelpFilterSettingsPrivate)
{
}

QHelpFilterSettings::QHelpFilterSettings(const QHelpFilterSettings &) = default;

QHelpFilterSettings::QHelpFilterSettings(QHelpFilterSettings &&) = default;

QHelpFilterSettings::~QHelpFilterSettings() = default;

QHelpFilterSettings &QHelpFilterSettings::operator=(const QHelpFilterSettings &) = default;

QHelpFilterSettings &QHelpFilterSettings::operator=(QHelpFilterSettings &&) = default;

void QHelpFilterSettings::setFilter(const QString &filterName,
                                    const QHelpFilterData &filterData)
{
    d->m_filterToData.insert(filterName, filterData);
}

void QHelpFilterSettings::removeFilter(const QString &filterName)
{
    d->m_filterToData.remove(filterName);
}

QStringList QHelpFilterSettings::filterNames() const
{
    return d->m_filterToData.keys();
}

QHelpFilterData QHelpFilterSettings::filterData(const QString &filterName) const
{
    return d->m_filterToData.value(filterName);
}

QMap<QString, QHelpFilterData> QHelpFilterSettings::filters() const
{
    return d->m_filterToData;
}

void QHelpFilterSettings::setCurrentFilter(const QString &filterName)
{
    d->m_currentFilter = filterName;
}

QString QHelpFilterSettings::currentFilter() const
{
    return d->m_currentFilter;
}

QHelpFilterSettings QHelpFilterSettings::readSettings(const QHelpFilterEngine *filterEngine)
{
    QHelpFilterSettings filterSettings;

    const QStringList allFilters = filterEngine->filters();
    for (const QString &filter : allFilters)
        filterSettings.setFilter(filter, filterEngine->filterData(filter));

    filterSettings.setCurrentFilter(filterEngine->activeFilter());

    return filterSettings;
}

static QMap<QString, QHelpFilterData> subtract(const QMap<QString, QHelpFilterData> &minuend,
                                               const QMap<QString, QHelpFilterData> &subtrahend)
{
    QMap<QString, QHelpFilterData> result = minuend;

    for (auto itSubtrahend = subtrahend.cbegin(); itSubtrahend != subtrahend.cend(); ++itSubtrahend) {
        auto itResult = result.find(itSubtrahend.key());
        if (itResult != result.end() && itSubtrahend.value() == itResult.value())
            result.erase(itResult);
    }

    return result;
}

bool QHelpFilterSettings::applySettings(QHelpFilterEngine *filterEngine,
                                        const QHelpFilterSettings &settings)
{
    bool changed = false;
    const QHelpFilterSettings oldSettings = readSettings(filterEngine);

    const QMap<QString, QHelpFilterData> filtersToRemove = subtract(
                oldSettings.filters(),
                settings.filters());
    const QMap<QString, QHelpFilterData> filtersToAdd = subtract(
                settings.filters(),
                oldSettings.filters());

    const QString &currentFilter = filterEngine->activeFilter();

    for (const QString &filter : filtersToRemove.keys()) {
        filterEngine->removeFilter(filter);
        if (currentFilter == filter && !filtersToAdd.contains(filter))
            filterEngine->setActiveFilter(QString());
        changed = true;
    }

    for (auto it = filtersToAdd.cbegin(); it != filtersToAdd.cend(); ++it) {
        filterEngine->setFilterData(it.key(), it.value());
        changed = true;
    }

    if (changed)
        filterEngine->setActiveFilter(settings.currentFilter());

    return changed;
}

QT_END_NAMESPACE
