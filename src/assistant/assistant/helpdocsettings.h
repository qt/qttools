// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef HELPDOCSETTINGS_H
#define HELPDOCSETTINGS_H

#include <QtCore/QSharedDataPointer>

QT_BEGIN_NAMESPACE

class QVersionNumber;
class QHelpEngineCore;
class HelpDocSettingsPrivate;

class HelpDocSettings final
{
public:
    HelpDocSettings();
    HelpDocSettings(const HelpDocSettings &other);
    HelpDocSettings(HelpDocSettings &&other);
    ~HelpDocSettings();

    HelpDocSettings &operator=(const HelpDocSettings &other);
    HelpDocSettings &operator=(HelpDocSettings &&other);

    void swap(HelpDocSettings &other) noexcept
    { d.swap(other.d); }

    bool addDocumentation(const QString &fileName);
    bool removeDocumentation(const QString &namespaceName);
    QString namespaceName(const QString &fileName) const;
    QStringList components() const;
    QList<QVersionNumber> versions() const;
    QStringList namespaces() const;
    QMap<QString, QString> namespaceToFileName() const;

    static HelpDocSettings readSettings(QHelpEngineCore *helpEngine);
    static bool applySettings(QHelpEngineCore *helpEngine, const HelpDocSettings &settings);

private:
    QSharedDataPointer<HelpDocSettingsPrivate> d;
};

QT_END_NAMESPACE

#endif // HELPDOCSETTINGS_H
