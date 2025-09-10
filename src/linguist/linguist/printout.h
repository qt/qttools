// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef PRINTOUT_H
#define PRINTOUT_H

#include <QFont>
#include <QPainter>
#include <QRect>
#include <QTextOption>
#include <QList>
#include <QDateTime>

QT_BEGIN_NAMESPACE

class QPrinter;
class QFontMetrics;

class PrintOut
{
public:
    enum Rule { NoRule, ThinRule, ThickRule };
    enum Style { Normal, Strong, Emphasis };

    PrintOut(QPrinter *printer);
    ~PrintOut();

    void setRule(Rule rule);
    void setGuide(const QString &guide);
    void vskip();
    void flushLine(bool mayBreak = false);
    void addBox(int percent, const QString &text = QString(),
                Style style = Normal,
                Qt::Alignment halign = Qt::AlignLeft);

    int pageNum() const { return page; }

    struct Box
    {
        QRect rect;
        QString text;
        QFont font;
        QTextOption options;

        Box( const QRect& r, const QString& t, const QFont& f, const QTextOption &o )
            : rect( r ), text( t ), font( f ), options( o ) { }
    };

private:
    void breakPage(bool init = false);
    void drawRule( Rule rule );

    struct Paragraph {
        QRect rect;
        QList<Box> boxes;

        Paragraph() { }
        Paragraph( QPoint p ) : rect( p, QSize(0, 0) ) { }
    };

    QPrinter *pr;
    QPainter p;
    QFont f8;
    QFont f10;
    QFontMetrics *fmetrics;
    Rule nextRule;
    Paragraph cp;
    int page;
    bool firstParagraph;
    QString g;
    QDateTime dateTime;

    int hmargin;
    int vmargin;
    int voffset;
    int hsize;
    int vsize;
};

QT_END_NAMESPACE

#endif
