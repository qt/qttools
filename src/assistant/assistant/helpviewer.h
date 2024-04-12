// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef HELPVIEWER_H
#define HELPVIEWER_H

#include <QtCore/qglobal.h>
#include <QtCore/QUrl>

#include <QtGui/QFont>
#include <QtGui/QTextDocument>

#include <QtPrintSupport/QPrinter>

#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class HelpViewerPrivate;

class HelpViewer : public QWidget
{
    Q_OBJECT
public:
    enum FindFlag {
        FindBackward = 0x01,
        FindCaseSensitively = 0x02
    };
    Q_DECLARE_FLAGS(FindFlags, FindFlag)

    HelpViewer(qreal zoom, QWidget *parent = nullptr);
    ~HelpViewer() override;

    QFont viewerFont() const;
    void setViewerFont(const QFont &font);

    void scaleUp();
    void scaleDown();

    void resetScale();
    qreal scale() const;

    QString title() const;

    QUrl source() const;
    void reload();
    void setSource(const QUrl &url);

#if QT_CONFIG(printer)
    void print(QPrinter *printer);
#endif

    QString selectedText() const;
    bool isForwardAvailable() const;
    bool isBackwardAvailable() const;

    bool findText(const QString &text, FindFlags flags, bool incremental,
        bool fromSearch);

    static bool isLocalUrl(const QUrl &url);
    static bool canOpenPage(const QString &url);
    static QString mimeFromUrl(const QUrl &url);
    static bool launchWithExternalApp(const QUrl &url);

    // implementation detail, not a part of the interface
    bool eventFilter(QObject *src, QEvent *event) override;

public slots:
#if QT_CONFIG(clipboard)
    void copy();
#endif
    void home();
    void forward();
    void backward();

signals:
    void titleChanged();
    void copyAvailable(bool yes);
    void sourceChanged(const QUrl &url);
    void forwardAvailable(bool enabled);
    void backwardAvailable(bool enabled);
    void highlighted(const QUrl &link);
    void printRequested();
    void loadFinished();
private:
    void doSetSource(const QUrl &url, bool reload);

    HelpViewerPrivate *d;
};

QT_END_NAMESPACE

#endif  // HELPVIEWER_H
