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
******************************************************************************/

#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QDialog>

QT_BEGIN_NAMESPACE

class QLabel;
class QPushButton;
class QGridLayout;

class AboutLabel : public QTextBrowser
{
    Q_OBJECT

public:
    AboutLabel(QWidget *parent = nullptr);
    void setText(const QString &text, const QByteArray &resources);
    QSize minimumSizeHint() const override;

private:
    QVariant loadResource(int type, const QUrl &name) override;

    void doSetSource(const QUrl &name, QTextDocument::ResourceType type) override;

    QMap<QString, QByteArray> m_resourceMap;
};

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    AboutDialog(QWidget *parent = nullptr);
    void setText(const QString &text, const QByteArray &resources);
    void setPixmap(const QPixmap &pixmap);
    QString documentTitle() const;

private:
    void updateSize();

    QLabel *m_pixmapLabel;
    AboutLabel *m_aboutLabel;
    QPushButton *m_closeButton;
    QGridLayout *m_layout;
};

QT_END_NAMESPACE

#endif
