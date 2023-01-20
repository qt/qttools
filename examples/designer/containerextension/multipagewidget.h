// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef MULTIPAGEWIDGET_H
#define MULTIPAGEWIDGET_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QComboBox;
class QStackedWidget;
QT_END_NAMESPACE

//! [0]
class MultiPageWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex)
    Q_PROPERTY(QString pageTitle READ pageTitle WRITE setPageTitle STORED false)

public:
    explicit MultiPageWidget(QWidget *parent = nullptr);

    QSize sizeHint() const override;

    int count() const;
    int currentIndex() const;
    QWidget *widget(int index);
    QString pageTitle() const;

public slots:
    void addPage(QWidget *page);
    void insertPage(int index, QWidget *page);
    void removePage(int index);
    void setPageTitle(const QString &newTitle);
    void setCurrentIndex(int index);

private slots:
    void pageWindowTitleChanged();

signals:
    void currentIndexChanged(int index);
    void pageTitleChanged(const QString &title);

private:
    QStackedWidget *stackWidget;
    QComboBox *comboBox;
};
//! [0]

#endif
