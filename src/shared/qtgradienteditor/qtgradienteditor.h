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

#ifndef QTGRADIENTEDITOR_H
#define QTGRADIENTEDITOR_H

#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class QtGradientEditor : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QGradient gradient READ gradient WRITE setGradient)
    Q_PROPERTY(bool backgroundCheckered READ isBackgroundCheckered WRITE setBackgroundCheckered)
    Q_PROPERTY(bool detailsVisible READ detailsVisible WRITE setDetailsVisible)
    Q_PROPERTY(bool detailsButtonVisible READ isDetailsButtonVisible WRITE setDetailsButtonVisible)
public:
    QtGradientEditor(QWidget *parent = 0);
    ~QtGradientEditor();

    void setGradient(const QGradient &gradient);
    QGradient gradient() const;

    bool isBackgroundCheckered() const;
    void setBackgroundCheckered(bool checkered);

    bool detailsVisible() const;
    void setDetailsVisible(bool visible);

    bool isDetailsButtonVisible() const;
    void setDetailsButtonVisible(bool visible);

    QColor::Spec spec() const;
    void setSpec(QColor::Spec spec);

signals:
    void gradientChanged(const QGradient &gradient);
    void aboutToShowDetails(bool details, int extenstionWidthHint);

private:
    QScopedPointer<class QtGradientEditorPrivate> d_ptr;
    Q_DECLARE_PRIVATE(QtGradientEditor)
    Q_DISABLE_COPY_MOVE(QtGradientEditor)
};

QT_END_NAMESPACE

#endif
