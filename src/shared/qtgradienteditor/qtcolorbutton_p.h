// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists for the convenience
// of Qt Designer. This header file may change from version to version
// without notice, or even be removed.
//
// We mean it.
//

#ifndef QTCOLORBUTTON_H
#define QTCOLORBUTTON_H

#include <QtWidgets/QToolButton>

QT_BEGIN_NAMESPACE

class QtColorButton : public QToolButton
{
    Q_OBJECT
    Q_PROPERTY(bool backgroundCheckered READ isBackgroundCheckered WRITE setBackgroundCheckered)
public:
    QtColorButton(QWidget *parent = 0);
    ~QtColorButton();

    bool isBackgroundCheckered() const;
    void setBackgroundCheckered(bool checkered);

    QColor color() const;

public slots:
    void setColor(QColor color);

signals:
    void colorChanged(QColor color);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
#ifndef QT_NO_DRAGANDDROP
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
#endif

private:
    QScopedPointer<class QtColorButtonPrivate> d_ptr;
    Q_DECLARE_PRIVATE(QtColorButton)
    Q_DISABLE_COPY_MOVE(QtColorButton)
};

QT_END_NAMESPACE

#endif
