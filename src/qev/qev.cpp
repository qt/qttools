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
******************************************************************************/

#include <QWidget>
#include <QApplication>
#include <QDebug>
#include <qevent.h>

QT_USE_NAMESPACE

QIODevice *qout;

class Widget : public QWidget
{
public:
    Widget(){ setAttribute(Qt::WA_InputMethodEnabled); }
    QSize sizeHint() const { return QSize(20, 20); }
    bool event(QEvent *e) {
        if (e->type() == QEvent::ContextMenu)
            return false;
        QDebug(qout) << e << Qt::endl;
        return QWidget::event(e);
    }
};


int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QFile fout;
    fout.open(stdout, QIODevice::WriteOnly);
    qout = &fout;

    Widget w;
    w.show();
    return app.exec();
}
