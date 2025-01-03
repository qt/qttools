// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef PROJECTDESCRIPTIONREADER_H
#define PROJECTDESCRIPTIONREADER_H

#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>

#include <optional>
#include <vector>

struct Project;

typedef std::vector<Project> Projects;

struct Project
{
    QString filePath;
    QString compileCommands;
    QString codec;
    QStringList excluded;
    QStringList includePaths;
    QStringList sources;
    Projects subProjects;
    std::optional<QStringList> translations;
};

Projects readProjectDescription(const QString &filePath, QString *errorString);

#endif // PROJECTDESCRIPTIONREADER_H
