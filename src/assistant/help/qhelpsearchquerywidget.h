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
**
**
**
**
**
**
**
**
**
**
**
******************************************************************************/

#ifndef QHELPSEARCHQUERYWIDGET_H
#define QHELPSEARCHQUERYWIDGET_H

#include <QtHelp/qhelp_global.h>
#include <QtHelp/qhelpsearchengine.h>

#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE


class QFocusEvent;
class QHelpSearchQueryWidgetPrivate;

class QHELP_EXPORT QHelpSearchQueryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QHelpSearchQueryWidget(QWidget *parent = nullptr);
    ~QHelpSearchQueryWidget() override;

    void expandExtendedSearch();
    void collapseExtendedSearch();

#if QT_DEPRECATED_SINCE(5, 9)
    QT_DEPRECATED QList<QHelpSearchQuery> query() const;
    QT_DEPRECATED void setQuery(const QList<QHelpSearchQuery> &queryList);
#endif

    QString searchInput() const;
    void setSearchInput(const QString &searchInput);

    bool isCompactMode() const;
    Q_SLOT void setCompactMode(bool on);

Q_SIGNALS:
    void search();

private:
    void focusInEvent(QFocusEvent *focusEvent) override;
    void changeEvent(QEvent *event) override;

private:
    QHelpSearchQueryWidgetPrivate *d;
};

QT_END_NAMESPACE

#endif  // QHELPSEARCHQUERYWIDGET_H
