// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QDialog>
#include <QtCore/QMap>

#include <QtCore/QMap>

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
