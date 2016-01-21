/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
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
#ifndef ATWRAPPER_H
#define ATWRAPPER_H

#include <QHash>
#include <QString>
#include <QList>
#include <QImage>

#include <private/qurlinfo_p.h>

class uiLoader : public QObject
{
    Q_OBJECT

    public:
        uiLoader(const QString &pathToProgram);

        enum TestResult { TestRunDone, TestConfigError, TestNoConfig };
        TestResult runAutoTests(QString *errorMessage);

    private:
        bool loadConfig(const QString &, QString *errorMessage);
        void initTests();

        void setupFTP();
        void setupLocal();
        void clearDirectory(const QString&);

        void ftpMkDir( QString );
        void ftpGetFiles(QList<QString>&, const QString&,  const QString&);
        void ftpList(const QString&);
        void ftpClearDirectory(const QString&);
        bool ftpUploadFile(const QString&, const QString&);
        void executeTests();

        void createBaseline();
        void downloadBaseline();

        bool compare();

        void diff(const QString&, const QString&, const QString&);
        int imgDiff(const QString fileA, const QString fileB, const QString output);
        QStringList uiFiles() const;

        QHash<QString, QString> enginesToTest;

        QString framework;
        QString suite;
        QString output;
        QString ftpUser;
        QString ftpPass;
        QString ftpHost;
        QString ftpBaseDir;
        QString threshold;

        QString errorMsg;

        QList<QString> lsDirList;
        QList<QString> lsNeedBaseline;

        QString configPath;

        QString pathToProgram;

    private slots:
        //void ftpAddLsDone( bool );
        void ftpAddLsEntry( const QUrlInfo &urlInfo );
};

#endif
