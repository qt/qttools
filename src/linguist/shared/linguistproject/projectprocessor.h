// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef PROJECTPROCESSOR_H
#define PROJECTPROCESSOR_H

#include "projectdescriptionreader.h"
#include <trlib/trparser.h>

#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>

QT_BEGIN_NAMESPACE

/**
 * Returns a list of source files frm the given resource file
 */
QStringList getSourceFilesFromQrc(const QString &resourceFile);

/**
 * Process a project description and update translation files
 * Returns false if errors occurred
 */
bool processProjectDescription(
    Projects &projectDescription,
    const QStringList &tsFileNames,
    const QStringList &alienFiles,
    const QString &sourceLanguage,
    const QString &targetLanguage,
    UpdateOptions options
);

/**
 * Process direct source files (non-project mode) and update translation files
 * Returns false if errors occurred
 */
bool processSourceFiles(
    const QStringList &sourceFiles,
    const QStringList &tsFileNames,
    const QStringList &alienFiles,
    const QSet<QString> &projectRoots,
    const QStringList &includePath,
    const QMultiHash<QString, QString> &allCSources,
    const QString &sourceLanguage,
    const QString &targetLanguage,
    UpdateOptions options
);

QT_END_NAMESPACE

#endif // PROJECTPROCESSOR_H
