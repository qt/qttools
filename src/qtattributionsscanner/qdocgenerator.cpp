/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qdocgenerator.h"

#include <QtCore/qdir.h>

#include <iostream>

namespace QDocGenerator {

static void generate(QTextStream &out, const Package &package, const QDir &baseDir,
                     LogLevel logLevel)
{
    out << "/*!\n\n";
    out << "\\contentspage attributions.html\n";
    out << "\\page attribution-" << package.id << ".html attribution\n";
    out << "\\target "<< package.id << "\n\n";
    out << "\\title " << package.name << "\n";
    out << "\\brief " << package.license << "\n\n";

    if (!package.description.isEmpty())
        out << package.description << "\n\n";

    if (!package.qtUsage.isEmpty())
        out << package.qtUsage << "\n\n";

    out << "The sources can be found in "
        << QDir::toNativeSeparators(baseDir.relativeFilePath(package.path)) << ".\n\n";

    if (!package.homepage.isEmpty())
        out << "\\l{" << package.homepage << "}{Project Homepage}\n\n";

    if (!package.copyright.isEmpty())
        out << "\n\\badcode\n" << package.copyright << "\n\\endcode\n\n";

    if (!package.licenseId.isEmpty() && package.licenseId != QLatin1String("NONE"))
        out << "\\l{https://spdx.org/licenses/" << package.licenseId << ".html}"
            << "{" << package.license << "}.\n\n";
    else
        out << package.license << ".\n\n";

    if (!package.licenseFile.isEmpty()) {
        QFile file(package.licenseFile);
        if (!file.open(QIODevice::ReadOnly)) {
            if (logLevel != SilentLog)
                std::cerr << qPrintable(tr("Cannot open file %1.").arg(
                                            QDir::toNativeSeparators(package.licenseFile))) << "\n";
            return;
        }
        out << "\\badcode\n";
        out << QString::fromUtf8(file.readAll()).trimmed();
        out << "\n\\endcode\n";
    }
    out << "*/\n";
}

void generate(QTextStream &out, const QVector<Package> &packages, const QString &baseDirectory,
              LogLevel logLevel)
{
    if (logLevel == VerboseLog)
        std::cerr << qPrintable(tr("Generating qdoc file...")) << std::endl;

    QDir baseDir(baseDirectory);
    for (const Package &package : packages)
        generate(out, package, baseDir, logLevel);
}

} // namespace QDocGenerator
