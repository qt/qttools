/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
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

#ifndef ABSTRACTFINDWIDGET_H
#define ABSTRACTFINDWIDGET_H

#include <QtGui/QIcon>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class QAction;
class QCheckBox;
class QEvent;
class QKeyEvent;
class QLabel;
class QLineEdit;
class QObject;
class QToolButton;

class AbstractFindWidget : public QWidget
{
    Q_OBJECT

public:
    enum FindFlag {
        /// Use a layout that is roughly half as wide and twice as high as the regular one.
        NarrowLayout = 1,
        /// Do not show the "Whole words" checkbox.
        NoWholeWords = 2,
        /// Do not show the "Case sensitive" checkbox.
        NoCaseSensitive = 4
    };
    Q_DECLARE_FLAGS(FindFlags, FindFlag)

    explicit AbstractFindWidget(FindFlags flags = FindFlags(), QWidget *parent = 0);
    ~AbstractFindWidget() override;

    bool eventFilter(QObject *object, QEvent *e) override;

    static QIcon findIconSet();
    QAction *createFindAction(QObject *parent);

public slots:
    void activate();
    virtual void deactivate();
    void findNext();
    void findPrevious();
    void findCurrentText();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void updateButtons();

protected:
    virtual void find(const QString &textToFind, bool skipCurrent, bool backward, bool *found, bool *wrapped) = 0;

    bool caseSensitive() const;
    bool wholeWords() const;

private:
    void findInternal(const QString &textToFind, bool skipCurrent, bool backward);

    QLineEdit *m_editFind;
    QLabel *m_labelWrapped;
    QToolButton *m_toolNext;
    QToolButton *m_toolClose;
    QToolButton *m_toolPrevious;
    QCheckBox *m_checkCase;
    QCheckBox *m_checkWholeWords;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(AbstractFindWidget::FindFlags)

QT_END_NAMESPACE

#endif  // ABSTRACTFINDWIDGET_H
