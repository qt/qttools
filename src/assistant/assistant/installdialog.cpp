/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Assistant of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include "tracer.h"

#include "installdialog.h"

#include <QtCore/QTimer>
#include <QtCore/QUrl>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QCryptographicHash>
#include <QtCore/QDebug>

#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkProxy>
#include <QtNetwork/QNetworkAccessManager>

#include <QtWidgets/QMessageBox>
#include <QtWidgets/QFileDialog>

#include <QtHelp/QHelpEngineCore>

QT_BEGIN_NAMESPACE
#ifndef QT_NO_HTTP

#define QCH_FILENAME  92943
#define QCH_NAMESPACE 92944
#define QCH_CHECKSUM  92945

static const char targetFileProperty[] = "targetFile";
static const char docInfoTargetFileId[] = "DocInfo";

enum { debug = 0 };

InstallDialog::InstallDialog(QHelpEngineCore *helpEngine, QWidget *parent,
                             const QString &host, int port)
    : QDialog(parent), m_helpEngine(helpEngine),
      m_networkAccessManager(new QNetworkAccessManager(this)),
      m_networkReply(0),
      m_host(host), m_port(port)
{
    TRACE_OBJ
    m_ui.setupUi(this);
    
    m_ui.installButton->setEnabled(false);
    m_ui.cancelButton->setEnabled(false);
    m_ui.pathLineEdit->setText(QFileInfo(m_helpEngine->collectionFile()).absolutePath());
    m_ui.progressBar->hide();

    m_windowTitle = tr("Install Documentation");


    connect(m_networkAccessManager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(httpRequestFinished(QNetworkReply*)));
    connect(m_ui.installButton, SIGNAL(clicked()), this, SLOT(install()));
    connect(m_ui.cancelButton, SIGNAL(clicked()), this, SLOT(cancelDownload()));
    connect(m_ui.browseButton, SIGNAL(clicked()), this, SLOT(browseDirectories()));

    connect(m_ui.listWidget, SIGNAL(itemChanged(QListWidgetItem*)),
        this, SLOT(updateInstallButton()));

    QTimer::singleShot(0, this, SLOT(init()));
}

InstallDialog::~InstallDialog()
{
    TRACE_OBJ
}

QStringList InstallDialog::installedDocumentations() const
{
    TRACE_OBJ
    return m_installedDocumentations;
}

void InstallDialog::init()
{
    TRACE_OBJ
    m_ui.statusLabel->setText(tr("Downloading documentation info..."));
    m_ui.progressBar->show();
    
    QUrl url(QLatin1String("http://qt.nokia.com/doc/assistantdocs/docs.txt"));
    if (m_port > -1)
        m_networkAccessManager->setProxy(QNetworkProxy(QNetworkProxy::HttpProxy, m_host, m_port));
    if (debug)
        qDebug() << "Sending " << url.toString();
    m_networkReply = m_networkAccessManager->get(QNetworkRequest(url));
    m_networkReply->setProperty(targetFileProperty, QVariant(QLatin1String(docInfoTargetFileId)));
    connect(m_networkReply, SIGNAL(uploadProgress(qint64,qint64)),
            this, SLOT(dataReadProgress(qint64,qint64)));

    m_ui.cancelButton->setEnabled(true);
    m_ui.closeButton->setEnabled(false);    
}

void InstallDialog::updateInstallButton()
{
    TRACE_OBJ
    QListWidgetItem *item = 0;
    for (int i=0; i<m_ui.listWidget->count(); ++i) {
        item = m_ui.listWidget->item(i);
        if (item->checkState() == Qt::Checked
            && item->flags() & Qt::ItemIsEnabled) {
            m_ui.installButton->setEnabled(true);
            return;
        }       
    }
    m_ui.installButton->setEnabled(false);
}

void InstallDialog::updateDocItemList()
{
    TRACE_OBJ
    QStringList registeredDocs = m_helpEngine->registeredDocumentations();
    QListWidgetItem *item = 0;
    for (int i=0; i<m_ui.listWidget->count(); ++i) {
        item = m_ui.listWidget->item(i);
        QString ns = item->data(QCH_NAMESPACE).toString();
        if (!ns.isEmpty() && registeredDocs.contains(ns)) {
            item->setFlags(Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Checked);
        }
        item->setCheckState(Qt::Unchecked);
    }
}

void InstallDialog::cancelDownload()
{
    TRACE_OBJ
    m_ui.statusLabel->setText(tr("Download canceled."));
    m_httpAborted = true;
    m_itemsToInstall.clear();
    m_networkReply->abort();
    m_ui.cancelButton->setEnabled(false);
    m_ui.closeButton->setEnabled(true);
    updateInstallButton();
}

void InstallDialog::install()
{
    TRACE_OBJ
    QListWidgetItem *item = 0;
    for (int i=0; i<m_ui.listWidget->count(); ++i) {
        item = m_ui.listWidget->item(i);
        if (item->checkState() == Qt::Checked)
            m_itemsToInstall.append(item);
    }
    m_ui.installButton->setEnabled(false);
    downloadNextFile();
}

void InstallDialog::downloadNextFile()
{
    TRACE_OBJ
    if (!m_itemsToInstall.count()) {
        m_ui.cancelButton->setEnabled(false);
        m_ui.closeButton->setEnabled(true);
        m_ui.statusLabel->setText(tr("Done."));
        m_ui.progressBar->hide();
        updateDocItemList();
        updateInstallButton();
        return;
    }

    QListWidgetItem *item = m_itemsToInstall.dequeue();
    m_currentCheckSum = item->data(QCH_CHECKSUM).toString();
    QString fileName = item->data(QCH_FILENAME).toString();
    QString saveFileName = m_ui.pathLineEdit->text() + QDir::separator()
        + fileName;

    if (QFile::exists(saveFileName)
        && QMessageBox::information(this, m_windowTitle,
        tr("The file %1 already exists. Do you want to overwrite it?")
        .arg(saveFileName), QMessageBox::Yes | QMessageBox::No,
        QMessageBox::Yes) == QMessageBox::No) {
        installFile(saveFileName);
        downloadNextFile();
        return;        
    }

    m_ui.statusLabel->setText(tr("Downloading %1...").arg(fileName));
    m_ui.progressBar->show();

    const QUrl url(QStringLiteral("http://qt.nokia.com/doc/assistantdocs/") + fileName);
    
    m_httpAborted = false;
    m_networkReply = m_networkAccessManager->get(QNetworkRequest(url));
    m_networkReply->setProperty(targetFileProperty, QVariant(saveFileName));
    if (debug)
        qDebug() << "Sending " << url.toString() << saveFileName;

    m_ui.cancelButton->setEnabled(true);
    m_ui.closeButton->setEnabled(false);    
}

void InstallDialog::httpRequestFinished(QNetworkReply *reply)
{
    TRACE_OBJ

    const QString targetFile = reply->property(targetFileProperty).toString();
    if (targetFile == QLatin1String(docInfoTargetFileId)) {
        m_ui.progressBar->hide();
        if (reply->error() != QNetworkReply::NoError) {
            QMessageBox::information(this, m_windowTitle,
                tr("Download failed: %1.")
                .arg(m_networkReply->errorString()));
            return;
        }
        if (!m_httpAborted) {
            while (reply->canReadLine()) {
                QByteArray ba = reply->readLine();
                const QStringList lst = QString::fromLatin1(ba.constData()).split(QLatin1Char('|'));
                if (lst.count() != 4) {
                    QMessageBox::information(this, m_windowTitle,
                        tr("Documentation info file is corrupt!"));
                } else {
                    QListWidgetItem *item = new QListWidgetItem(m_ui.listWidget);
                    item->setText(lst.at(2).trimmed());
                    item->setData(QCH_FILENAME, lst.first());
                    item->setData(QCH_NAMESPACE, lst.at(1));
                    item->setData(QCH_CHECKSUM, lst.last().trimmed());
                }
            }
            updateDocItemList();
        }
        m_ui.statusLabel->setText(tr("Done."));
        m_ui.cancelButton->setEnabled(false);        
        m_ui.closeButton->setEnabled(true);
        updateInstallButton();
        return;
    }

    // Download of file
    if (reply->error() != QNetworkReply::NoError) {
        QMessageBox::warning(this, m_windowTitle,
            tr("Download failed: %1.")
            .arg(reply->errorString()));
        downloadNextFile();
        return;
    }

    if (m_httpAborted) {
        downloadNextFile();
        return;
    }

    QFile file(targetFile);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Truncate)) {
        QMessageBox::information(this, m_windowTitle,
                                 tr("Unable to save the file %1: %2.")
                                 .arg(targetFile).arg(file.errorString()));
        downloadNextFile();
        return;
    }
    const QByteArray data = reply->readAll();
    file.write(data);
    file.close();

    const QByteArray digest = QCryptographicHash::hash(data, QCryptographicHash::Md5);
    const QString checkSum = QString::fromLatin1(digest.toHex());

    if (checkSum.isEmpty() || m_currentCheckSum != checkSum) {
        file.remove();
        QMessageBox::warning(this, m_windowTitle,
                             tr("Download failed: Downloaded file is corrupted."));
        downloadNextFile();
        return;
    }

    m_ui.statusLabel->setText(tr("Installing documentation %1...")
                              .arg(QFileInfo(targetFile).fileName()));
    m_ui.progressBar->setMaximum(0);
    m_ui.statusLabel->setText(tr("Done."));
    installFile(targetFile);
    downloadNextFile();
}

void InstallDialog::installFile(const QString &fileName)
{
    TRACE_OBJ
    if (m_helpEngine->registerDocumentation(fileName)) {
        m_installedDocumentations
            .append(QHelpEngineCore::namespaceName(fileName));
    } else {
        QMessageBox::information(this, m_windowTitle,
            tr("Error while installing documentation:\n%1")
            .arg(m_helpEngine->error()));
    }
}

void InstallDialog::updateDataReadProgress(qint64 bytesRead, qint64 totalBytes)
{
    TRACE_OBJ
    if (m_httpAborted)
        return;

    m_ui.progressBar->setMaximum(totalBytes);
    m_ui.progressBar->setValue(bytesRead);
}

void InstallDialog::browseDirectories()
{
    TRACE_OBJ
    QString dir = QFileDialog::getExistingDirectory(this, m_windowTitle,
        m_ui.pathLineEdit->text());
    if (!dir.isEmpty())
        m_ui.pathLineEdit->setText(dir);
}

#endif
QT_END_NAMESPACE
