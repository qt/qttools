// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef HELPVIEWERIMPL_H
#define HELPVIEWERIMPL_H

#include <QtCore/qglobal.h>
#include <QtCore/QUrl>
#include <QtCore/QVariant>

#include <QtGui/QFont>

#if defined(BROWSER_QTWEBKIT)
#  include <QWebView>
#elif defined(BROWSER_QTEXTBROWSER)
#  include <QtWidgets/QTextBrowser>
#endif

#include "helpviewer.h"

QT_BEGIN_NAMESPACE

#if defined(BROWSER_QTWEBKIT)
#define TEXTBROWSER_OVERRIDE
class HelpViewerImpl : public QWebView
#elif defined(BROWSER_QTEXTBROWSER)
#define TEXTBROWSER_OVERRIDE override
class HelpViewerImpl : public QTextBrowser
#endif
{
    Q_OBJECT
    class HelpViewerImplPrivate;

public:
    HelpViewerImpl(qreal zoom, QWidget *parent = nullptr);
    ~HelpViewerImpl() override;

    QFont viewerFont() const;
    void setViewerFont(const QFont &font);

    void scaleUp();
    void scaleDown();

    void resetScale();
    qreal scale() const;

    QString title() const;

    QUrl source() const;
    void doSetSource(const QUrl &url, QTextDocument::ResourceType type) TEXTBROWSER_OVERRIDE;

    QString selectedText() const;
    bool isForwardAvailable() const;
    bool isBackwardAvailable() const;

    bool findText(const QString &text, HelpViewer::FindFlags flags, bool incremental,
        bool fromSearch);

    static const QString AboutBlank;
    static const QString LocalHelpFile;
    static const QString PageNotFoundMessage;

public slots:
#if QT_CONFIG(clipboard)
    void copy();
#endif
    void home() TEXTBROWSER_OVERRIDE;

    void forward() TEXTBROWSER_OVERRIDE;
    void backward() TEXTBROWSER_OVERRIDE;

signals:
    void titleChanged();
#if !defined(BROWSER_QTEXTBROWSER)
    // Provide signals present in QTextBrowser, QTextEdit for browsers that do not inherit QTextBrowser
    void copyAvailable(bool yes);
    void sourceChanged(const QUrl &url);
    void forwardAvailable(bool enabled);
    void backwardAvailable(bool enabled);
    void highlighted(const QUrl &link);
    void printRequested();
#elif !defined(BROWSER_QTWEBKIT)
    // Provide signals present in QWebView for browsers that do not inherit QWebView
    void loadFinished(bool finished);
#endif

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *e) override;

private slots:
    void actionChanged();
    void setLoadFinished();

private:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    QVariant loadResource(int type, const QUrl &name) TEXTBROWSER_OVERRIDE;
    bool handleForwardBackwardMouseButtons(QMouseEvent *e);
    void scrollToTextPosition(int position);

private:
    HelpViewerImplPrivate *d;
};

QT_END_NAMESPACE

#endif  // HELPVIEWERIMPL_H
