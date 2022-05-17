// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QWidget>
#include <QApplication>
#include <QDebug>
#include <QFile>
#include <qevent.h>

QT_USE_NAMESPACE

QIODevice *qout;

class Widget : public QWidget
{
public:
    Widget(){ setAttribute(Qt::WA_InputMethodEnabled); }
    QSize sizeHint() const override { return QSize(20, 20); }
    bool event(QEvent *e) override
    {
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
