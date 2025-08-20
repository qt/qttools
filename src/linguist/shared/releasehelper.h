// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef RELEASEHELPER_H
#define RELEASEHELPER_H

#include <QtCore/qstringlist.h>

QT_BEGIN_NAMESPACE

class Translator;
class ConversionData;
struct Project;
using Projects = std::vector<Project>;

struct ParamFlags
{
    bool failOnUnfinished = false;
    bool failOnInvalid = false;
    bool removeIdentical = false;
};

bool loadTsFile(Translator &tor, const QString &tsFileName);

bool releaseTranslator(Translator &tor, const QString &qmFileName, ConversionData &cd,
                       ParamFlags params);

bool releaseTsFile(const QString &tsFileName, ConversionData &cd, ParamFlags params);

// Extract translation file names from project structures
QStringList translationsFromProject(const Project &project, bool topLevel);
QStringList translationsFromProjects(const Projects &projects, bool topLevel = true);

void printOut(const QString &out);
void printErr(const QString &out);

QT_END_NAMESPACE

#endif // RELEASEHELPER_H
