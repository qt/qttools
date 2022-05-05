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

#ifndef QTCOLORLINE_H
#define QTCOLORLINE_H

#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class QtColorLine : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor)
    Q_PROPERTY(int indicatorSpace READ indicatorSpace WRITE setIndicatorSpace)
    Q_PROPERTY(int indicatorSize READ indicatorSize WRITE setIndicatorSize)
    Q_PROPERTY(bool flip READ flip WRITE setFlip)
    Q_PROPERTY(bool backgroundCheckered READ isBackgroundCheckered WRITE setBackgroundCheckered)
    Q_PROPERTY(ColorComponent colorComponent READ colorComponent WRITE setColorComponent)
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)
public:

    enum ColorComponent {
        Red,
        Green,
        Blue,
        Hue,
        Saturation,
        Value,
        Alpha
    };
    Q_ENUM(ColorComponent)

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

    QtColorLine(QWidget *parent = 0);
    ~QtColorLine();

    QColor color() const;

    void setIndicatorSize(int size);
    int indicatorSize() const;

    void setIndicatorSpace(int space);
    int indicatorSpace() const;

    void setFlip(bool flip);
    bool flip() const;

    bool isBackgroundCheckered() const;
    void setBackgroundCheckered(bool checkered);

    void setOrientation(Qt::Orientation orientation);
    Qt::Orientation orientation() const;

    void setColorComponent(ColorComponent component);
    ColorComponent colorComponent() const;

public slots:
    void setColor(const QColor &color);

signals:
    void colorChanged(const QColor &color);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    QScopedPointer<class QtColorLinePrivate> d_ptr;
    Q_DECLARE_PRIVATE(QtColorLine)
    Q_DISABLE_COPY_MOVE(QtColorLine)
};

QT_END_NAMESPACE

#endif
