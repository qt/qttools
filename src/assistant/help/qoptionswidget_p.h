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

#ifndef QOPTIONSWIDGET_H
#define QOPTIONSWIDGET_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists for the convenience
// of the help generator tools. This header file may change from version
// to version without notice, or even be removed.
//
// We mean it.
//

#include <QtWidgets/QWidget>
#include <QtCore/QMap>

QT_BEGIN_NAMESPACE

class QListWidget;
class QListWidgetItem;

class QOptionsWidget : public QWidget
{
    Q_OBJECT
public:
    QOptionsWidget(QWidget *parent = nullptr);

    void clear();
    void setOptions(const QStringList &validOptions,
                    const QStringList &selectedOptions);
    QStringList validOptions() const;
    QStringList selectedOptions() const;

    void setNoOptionText(const QString &text);
    void setInvalidOptionText(const QString &text);

signals:
    void optionSelectionChanged(const QStringList &options);

private:
    QString optionText(const QString &optionName, bool valid) const;
    QListWidgetItem *appendItem(const QString &optionName, bool valid, bool selected);
    void appendSeparator();
    void itemChanged(QListWidgetItem *item);

    QListWidget *m_listWidget = nullptr;
    QString m_noOptionText;
    QString m_invalidOptionText;
    QStringList m_validOptions;
    QStringList m_invalidOptions;
    QStringList m_selectedOptions;
    QMap<QString, QListWidgetItem *> m_optionToItem;
    QMap<QListWidgetItem *, QString> m_itemToOption;
};

QT_END_NAMESPACE

#endif  // OPTIONSWIDGET_H
