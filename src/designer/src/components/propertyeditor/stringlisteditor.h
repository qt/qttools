// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef STRINGLISTEDITOR_H
#define STRINGLISTEDITOR_H

#include "ui_stringlisteditor.h"
#include <QtCore/qstringlist.h>

QT_BEGIN_NAMESPACE
class QStringListModel;

namespace qdesigner_internal {

class StringListEditor : public QDialog, private Ui::Dialog
{
    Q_OBJECT
public:
    ~StringListEditor();
    void setStringList(const QStringList &stringList);
    QStringList stringList() const;

    static QStringList getStringList(
        QWidget *parent, const QStringList &init = QStringList(), int *result = nullptr);

private slots:
    void on_upButton_clicked();
    void on_downButton_clicked();
    void on_newButton_clicked();
    void on_deleteButton_clicked();
    void on_valueEdit_textEdited(const QString &text);
    void currentIndexChanged(const QModelIndex &current, const QModelIndex &previous);
    void currentValueChanged();

private:
    StringListEditor(QWidget *parent = nullptr);
    void updateUi();
    int currentIndex() const;
    void setCurrentIndex(int index);
    int count() const;
    QString stringAt(int index) const;
    void setStringAt(int index, const QString &value);
    void removeString(int index);
    void insertString(int index, const QString &value);
    void editString(int index);

    QStringListModel *m_model;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // STRINGLISTEDITOR_H
