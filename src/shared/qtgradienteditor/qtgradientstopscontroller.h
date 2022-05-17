// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QTGRADIENTSTOPSCONTROLLER_H
#define QTGRADIENTSTOPSCONTROLLER_H

#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

namespace Ui {
    class QtGradientEditor;
}

class QtGradientStopsController : public QObject
{
    Q_OBJECT
public:
    QtGradientStopsController(QObject *parent = 0);
    ~QtGradientStopsController();

    void setUi(Ui::QtGradientEditor *editor);

    void setGradientStops(const QGradientStops &stops);
    QGradientStops gradientStops() const;

    QColor::Spec spec() const;
    void setSpec(QColor::Spec spec);

signals:
    void gradientStopsChanged(const QGradientStops &stops);

private:
    QScopedPointer<class QtGradientStopsControllerPrivate> d_ptr;
    Q_DECLARE_PRIVATE(QtGradientStopsController)
    Q_DISABLE_COPY_MOVE(QtGradientStopsController)
};

QT_END_NAMESPACE

#endif
