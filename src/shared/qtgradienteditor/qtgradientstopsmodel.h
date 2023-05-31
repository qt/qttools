// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QTGRADIENTSTOPSMODEL_H
#define QTGRADIENTSTOPSMODEL_H

#include <QtCore/QObject>
#include <QtCore/QMap>

QT_BEGIN_NAMESPACE

class QColor;

class QtGradientStopsModel;

class QtGradientStop
{
public:
    qreal position() const;
    QColor color() const;
    QtGradientStopsModel *gradientModel() const;

private:
    void setColor(const QColor &color);
    void setPosition(qreal position);
    friend class QtGradientStopsModel;
    QtGradientStop(QtGradientStopsModel *model = 0);
    ~QtGradientStop();
    QScopedPointer<class QtGradientStopPrivate> d_ptr;
};

class QtGradientStopsModel : public QObject
{
    Q_OBJECT
public:
    using PositionStopMap = QMap<qreal, QtGradientStop *>;

    QtGradientStopsModel(QObject *parent = 0);
    ~QtGradientStopsModel();

    PositionStopMap stops() const;
    QtGradientStop *at(qreal pos) const;
    QColor color(qreal pos) const; // calculated between points
    QList<QtGradientStop *> selectedStops() const;
    QtGradientStop *currentStop() const;
    bool isSelected(QtGradientStop *stop) const;
    QtGradientStop *firstSelected() const;
    QtGradientStop *lastSelected() const;
    QtGradientStopsModel *clone() const;

    QtGradientStop *addStop(qreal pos, const QColor &color);
    void removeStop(QtGradientStop *stop);
    void moveStop(QtGradientStop *stop, qreal newPos);
    void swapStops(QtGradientStop *stop1, QtGradientStop *stop2);
    void changeStop(QtGradientStop *stop, const QColor &newColor);
    void selectStop(QtGradientStop *stop, bool select);
    void setCurrentStop(QtGradientStop *stop);

    void moveStops(double newPosition); // moves current stop to newPos and all selected stops are moved accordingly
    void clear();
    void clearSelection();
    void flipAll();
    void selectAll();
    void deleteStops();

signals:
    void stopAdded(QtGradientStop *stop);
    void stopRemoved(QtGradientStop *stop);
    void stopMoved(QtGradientStop *stop, qreal newPos);
    void stopsSwapped(QtGradientStop *stop1, QtGradientStop *stop2);
    void stopChanged(QtGradientStop *stop, const QColor &newColor);
    void stopSelected(QtGradientStop *stop, bool selected);
    void currentStopChanged(QtGradientStop *stop);

private:
    QScopedPointer<class QtGradientStopsModelPrivate> d_ptr;
    Q_DECLARE_PRIVATE(QtGradientStopsModel)
    Q_DISABLE_COPY_MOVE(QtGradientStopsModel)
};

QT_END_NAMESPACE

#endif
