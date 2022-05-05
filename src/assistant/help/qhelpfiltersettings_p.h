/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Assistant of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
******************************************************************************/

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
