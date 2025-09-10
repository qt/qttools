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

#ifndef QTGRADIENTSTOPSWIDGET_H
#define QTGRADIENTSTOPSWIDGET_H

#include <QtWidgets/QAbstractScrollArea>

QT_BEGIN_NAMESPACE

class QtGradientStopsModel;
class QtGradientStopsWidgetPrivate;

class QtGradientStopsWidget : public QAbstractScrollArea
{
    Q_OBJECT
    Q_PROPERTY(bool backgroundCheckered READ isBackgroundCheckered WRITE setBackgroundCheckered)
public:
    QtGradientStopsWidget(QWidget *parent = 0);
    ~QtGradientStopsWidget();

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

    bool isBackgroundCheckered() const;
    void setBackgroundCheckered(bool checkered);

    void setGradientStopsModel(QtGradientStopsModel *model);

    void setZoom(double zoom);
    double zoom() const;

signals:
    void zoomChanged(double zoom);

protected:
    void paintEvent(QPaintEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void focusInEvent(QFocusEvent *e) override;
    void focusOutEvent(QFocusEvent *e) override;
    void contextMenuEvent(QContextMenuEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;
#ifndef QT_NO_DRAGANDDROP
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
#endif

private:
    QScopedPointer<QtGradientStopsWidgetPrivate> d_ptr;
    Q_DECLARE_PRIVATE(QtGradientStopsWidget)
    Q_DISABLE_COPY_MOVE(QtGradientStopsWidget)
};

QT_END_NAMESPACE

#endif
