// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef QTPROPERTYBROWSERUTILS_H
#define QTPROPERTYBROWSERUTILS_H

#include <QtCore/QMap>
#include <QtGui/QIcon>
#include <QtWidgets/QWidget>
#include <QtCore/QStringList>

QT_BEGIN_NAMESPACE

class QMouseEvent;
class QCheckBox;
class QLineEdit;

class QtCursorDatabase
{
public:
    QtCursorDatabase();
    void clear();

    QStringList cursorShapeNames() const;
    QMap<int, QIcon> cursorShapeIcons() const;
    QString cursorToShapeName(const QCursor &cursor) const;
    QIcon cursorToShapeIcon(const QCursor &cursor) const;
    int cursorToValue(const QCursor &cursor) const;
#ifndef QT_NO_CURSOR
    QCursor valueToCursor(int value) const;
#endif

    static QtCursorDatabase *instance();

private:
    void appendCursor(Qt::CursorShape shape, const QString &name, const QIcon &icon);
    QStringList m_cursorNames;
    QMap<int, QIcon> m_cursorIcons;
    QMap<int, Qt::CursorShape> m_valueToCursorShape;
    QMap<Qt::CursorShape, int> m_cursorShapeToValue;
};

class QtPropertyBrowserUtils
{
public:
    static QPixmap brushValuePixmap(const QBrush &b);
    static QIcon brushValueIcon(const QBrush &b);
    static QString colorValueText(const QColor &c);
    static QPixmap fontValuePixmap(const QFont &f);
    static QIcon fontValueIcon(const QFont &f);
    static QString fontValueText(const QFont &f);
    static QString dateFormat();
    static QString timeFormat();
    static QString dateTimeFormat();
};

class QtBoolEdit : public QWidget {
    Q_OBJECT
public:
    QtBoolEdit(QWidget *parent = 0);

    bool textVisible() const { return m_textVisible; }
    void setTextVisible(bool textVisible);

    Qt::CheckState checkState() const;
    void setCheckState(Qt::CheckState state);

    bool isChecked() const;
    void setChecked(bool c);

    bool blockCheckBoxSignals(bool block);

Q_SIGNALS:
    void toggled(bool);

protected:
    void mousePressEvent(QMouseEvent * event) override;

private:
    QCheckBox *m_checkBox;
    bool m_textVisible;
};

QT_END_NAMESPACE

#endif
