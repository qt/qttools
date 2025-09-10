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

#ifndef QTGRADIENTWIDGET_H
#define QTGRADIENTWIDGET_H

#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class QtGradientWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool backgroundCheckered READ isBackgroundCheckered WRITE setBackgroundCheckered)
public:
    QtGradientWidget(QWidget *parent = 0);
    ~QtGradientWidget();

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;
    int heightForWidth(int w) const override;

    bool isBackgroundCheckered() const;
    void setBackgroundCheckered(bool checkered);

    QGradientStops gradientStops() const;

    void setGradientType(QGradient::Type type);
    QGradient::Type gradientType() const;

    void setGradientSpread(QGradient::Spread spread);
    QGradient::Spread gradientSpread() const;

    void setStartLinear(QPointF point);
    QPointF startLinear() const;

    void setEndLinear(QPointF point);
    QPointF endLinear() const;

    void setCentralRadial(QPointF point);
    QPointF centralRadial() const;

    void setFocalRadial(QPointF point);
    QPointF focalRadial() const;

    void setRadiusRadial(qreal radius);
    qreal radiusRadial() const;

    void setCentralConical(QPointF point);
    QPointF centralConical() const;

    void setAngleConical(qreal angle);
    qreal angleConical() const;

public slots:
    void setGradientStops(const QGradientStops &stops);

signals:

    void startLinearChanged(const QPointF &point);
    void endLinearChanged(const QPointF &point);
    void centralRadialChanged(const QPointF &point);
    void focalRadialChanged(const QPointF &point);
    void radiusRadialChanged(qreal radius);
    void centralConicalChanged(const QPointF &point);
    void angleConicalChanged(qreal angle);

protected:
    void paintEvent(QPaintEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;

private:
    QScopedPointer<class QtGradientWidgetPrivate> d_ptr;
    Q_DECLARE_PRIVATE(QtGradientWidget)
    Q_DISABLE_COPY_MOVE(QtGradientWidget)
};

QT_END_NAMESPACE

#endif
