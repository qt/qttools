// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef PROJECTDESCRIPTIONREADER_H
#define PROJECTDESCRIPTIONREADER_H

#include <QtCore/qjsonarray.h>
#include <QtCore/qregularexpression.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qvector.h>

#include <optional>
#include <vector>

QT_BEGIN_NAMESPACE

struct Project;

typedef std::vector<Project> Projects;

struct Project
{
    QString filePath;
    QString codec;
    QVector<QRegularExpression> excluded;
    QStringList includePaths;
    QStringList sources;
    Projects subProjects;
    std::optional<QStringList> translations;
};

Projects projectDescriptionFromFile(const QString &filePath, QString *errorString);
Projects projectDescriptionFromJson(const QJsonArray &rawProjects, QString *errorString);

QT_END_NAMESPACE

#endif // PROJECTDESCRIPTIONREADER_H
