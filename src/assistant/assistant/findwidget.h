// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
#ifndef FINDWIDGET_H
#define FINDWIDGET_H

#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class QCheckBox;
class QLabel;
class QLineEdit;
class QToolButton;

class FindWidget : public QWidget
{
    Q_OBJECT
public:
    FindWidget(QWidget *parent = nullptr);
    ~FindWidget() override;

    void show();
    void showAndClear();

    QString text() const;
    bool caseSensitive() const;

    void setPalette(bool found);
    void setTextWrappedVisible(bool visible);

signals:
    void findNext();
    void findPrevious();
    void escapePressed();
    void find(const QString &text, bool forward, bool incremental);

protected:
    void hideEvent(QHideEvent* event) override;
    void showEvent(QShowEvent * event) override;

private slots:
    void updateButtons();
    void textChanged(const QString &text);

private:
    bool eventFilter(QObject *object, QEvent *e) override;
    QToolButton* setupToolButton(const QString &text, const QString &icon);

private:
    QPalette appPalette;

    QLineEdit *editFind;
    QCheckBox *checkCase;
    QLabel *labelWrapped;
    QToolButton *toolNext;
    QToolButton *toolClose;
    QToolButton *toolPrevious;
};

QT_END_NAMESPACE

#endif  // FINDWIDGET_H
