// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qhelpsearchresultwidget.h"

#include <QtCore/qcoreevent.h>
#include <QtCore/qlist.h>
#include <QtCore/qpointer.h>
#include <QtCore/qtextstream.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qlayout.h>
#include <QtWidgets/qlayoutitem.h>
#include <QtWidgets/qtoolbutton.h>
#include <QtWidgets/qtextbrowser.h>

QT_BEGIN_NAMESPACE

static constexpr int ResultsRange = 20;

class QResultWidget : public QTextBrowser
{
    Q_OBJECT
    Q_PROPERTY(QColor linkColor READ linkColor WRITE setLinkColor)

public:
    QResultWidget(QWidget *parent = nullptr)
        : QTextBrowser(parent)
    {
        connect(this, &QTextBrowser::anchorClicked, this, &QResultWidget::requestShowLink);
        setContextMenuPolicy(Qt::NoContextMenu);
        setLinkColor(palette().color(QPalette::Link));
    }

    QColor linkColor() const { return m_linkColor; }
    void setLinkColor(const QColor &color)
    {
        m_linkColor = color;
        const QString sheet = QString::fromLatin1("a { text-decoration: underline; color: %1 }")
                                      .arg(m_linkColor.name());
        document()->setDefaultStyleSheet(sheet);
    }

    void showResultPage(const QList<QHelpSearchResult> &results, bool isIndexing)
    {
        QString htmlFile;
        QTextStream str(&htmlFile);
        str << "<html><head><title>" << tr("Search Results") << "</title></head><body>";

        const int count = results.size();
        if (count != 0) {
            if (isIndexing) {
                str << "<div style=\"text-align:left;"
                       " font-weight:bold; color:red\">" << tr("Note:")
                    << "&nbsp;<span style=\"font-weight:normal; color:black\">"
                    << tr("The search results may not be complete since the "
                          "documentation is still being indexed.")
                    << "</span></div></div><br>";
            }

            for (const QHelpSearchResult &result : results) {
                str << "<div style=\"text-align:left\"><a href=\""
                    << result.url().toString() << "\">"
                    << result.title() << "</a></div>"
                    "<div style =\"margin:5px\">" << result.snippet() << "</div>";
            }
        } else {
            str << "<div align=\"center\"><br><br><h2>"
                << tr("Your search did not match any documents.")
                << "</h2><div>";
            if (isIndexing) {
                str << "<div align=\"center\"><h3>"
                    << tr("(The reason for this might be that the documentation "
                          "is still being indexed.)") << "</h3><div>";
            }
        }

        str << "</body></html>";
        setHtml(htmlFile);
    }

signals:
    void requestShowLink(const QUrl &url);

private slots:
    void doSetSource(const QUrl & /*name*/, QTextDocument::ResourceType /*type*/) override {}

private:
    QColor m_linkColor;
};

class QHelpSearchResultWidgetPrivate
{
public:
    ~QHelpSearchResultWidgetPrivate()
    {
        delete searchEngine; // TODO: This it probably wrong, why the widget owns the engine?
    }

    QToolButton* setupToolButton(const QString &iconPath)
    {
        QToolButton *button = new QToolButton;
        button->setEnabled(false);
        button->setAutoRaise(true);
        button->setIcon(QIcon(iconPath));
        button->setIconSize({12, 12});
        button->setMaximumSize({16, 16});
        return button;
    }

    void updateHitRange()
    {
        int last = 0;
        int first = 0;
        int count = 0;

        if (!searchEngine.isNull()) {
            count = searchEngine->searchResultCount();
            if (count > 0) {
                last = qMin(resultFirstToShow + ResultsRange, count);
                first = resultFirstToShow + 1;
            }
            resultTextBrowser->showResultPage(searchEngine->searchResults(resultFirstToShow, last),
                                              isIndexing);
        }

        hitsLabel->setText(QHelpSearchResultWidget::tr("%1 - %2 of %n Hits", nullptr, count)
                                   .arg(first).arg(last));
        firstResultPage->setEnabled(resultFirstToShow);
        previousResultPage->setEnabled(resultFirstToShow);
        lastResultPage->setEnabled(count - last);
        nextResultPage->setEnabled(count - last);
    }

    QHelpSearchResultWidget *q = nullptr;
    QPointer<QHelpSearchEngine> searchEngine;

    QResultWidget *resultTextBrowser = nullptr;

    QToolButton *firstResultPage = nullptr;
    QToolButton *previousResultPage = nullptr;
    QToolButton *nextResultPage = nullptr;
    QToolButton *lastResultPage = nullptr;
    QLabel *hitsLabel = nullptr;
    int resultFirstToShow = 0;
    bool isIndexing = false;
};

/*!
    \class QHelpSearchResultWidget
    \since 4.4
    \inmodule QtHelp
    \brief The QHelpSearchResultWidget class provides a text browser to display
    search results.
*/

/*!
    \fn void QHelpSearchResultWidget::requestShowLink(const QUrl &link)

    This signal is emitted when a item is activated and its associated
    \a link should be shown.
*/

QHelpSearchResultWidget::QHelpSearchResultWidget(QHelpSearchEngine *engine)
    : QWidget(0)
    , d(new QHelpSearchResultWidgetPrivate)
{
    d->q = this;
    d->searchEngine = engine;

    connect(engine, &QHelpSearchEngine::indexingStarted, this, [this] { d->isIndexing = true; });
    connect(engine, &QHelpSearchEngine::indexingFinished, this, [this] { d->isIndexing = false; });

    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins({});
    vLayout->setSpacing(0);

    QHBoxLayout *hBoxLayout = new QHBoxLayout();
#ifndef Q_OS_MAC
    hBoxLayout->setContentsMargins({});
    hBoxLayout->setSpacing(0);
#endif
    hBoxLayout->addWidget(d->firstResultPage = d->setupToolButton(
        QString::fromUtf8(":/qt-project.org/assistant/images/3leftarrow.png")));

    hBoxLayout->addWidget(d->previousResultPage = d->setupToolButton(
        QString::fromUtf8(":/qt-project.org/assistant/images/1leftarrow.png")));

    d->hitsLabel = new QLabel(tr("0 - 0 of 0 Hits"), this);
    hBoxLayout->addWidget(d->hitsLabel);
    d->hitsLabel->setAlignment(Qt::AlignCenter);
    d->hitsLabel->setMinimumSize(QSize(150, d->hitsLabel->height()));

    hBoxLayout->addWidget(d->nextResultPage = d->setupToolButton(
        QString::fromUtf8(":/qt-project.org/assistant/images/1rightarrow.png")));

    hBoxLayout->addWidget(d->lastResultPage = d->setupToolButton(
        QString::fromUtf8(":/qt-project.org/assistant/images/3rightarrow.png")));

    QSpacerItem *spacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    hBoxLayout->addItem(spacer);

    vLayout->addLayout(hBoxLayout);

    d->resultTextBrowser = new QResultWidget(this);
    vLayout->addWidget(d->resultTextBrowser);

    connect(d->resultTextBrowser, &QResultWidget::requestShowLink,
            this, &QHelpSearchResultWidget::requestShowLink);

    connect(d->nextResultPage, &QAbstractButton::clicked, this, [this] {
        if (!d->searchEngine.isNull()
            && d->resultFirstToShow + ResultsRange < d->searchEngine->searchResultCount()) {
            d->resultFirstToShow += ResultsRange;
        }
        d->updateHitRange();
    });
    connect(d->previousResultPage, &QAbstractButton::clicked, this, [this] {
        if (!d->searchEngine.isNull()) {
            d->resultFirstToShow -= ResultsRange;
            if (d->resultFirstToShow < 0)
                d->resultFirstToShow = 0;
        }
        d->updateHitRange();
    });
    connect(d->lastResultPage, &QAbstractButton::clicked, this, [this] {
        if (!d->searchEngine.isNull())
            d->resultFirstToShow = (d->searchEngine->searchResultCount() - 1) / ResultsRange * ResultsRange;
        d->updateHitRange();
    });
    const auto showFirstPage = [this] {
        if (!d->searchEngine.isNull())
            d->resultFirstToShow = 0;
        d->updateHitRange();
    };
    connect(d->firstResultPage, &QAbstractButton::clicked, this, showFirstPage);
    connect(engine, &QHelpSearchEngine::searchingFinished, this, showFirstPage);
}

/*! \reimp
*/
void QHelpSearchResultWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
        d->updateHitRange();
}

/*!
    Destroys the search result widget.
*/
QHelpSearchResultWidget::~QHelpSearchResultWidget()
{
    delete d;
}

/*!
    Returns a reference of the URL that the item at \a point owns, or an
    empty URL if no item exists at that point.
*/
QUrl QHelpSearchResultWidget::linkAt(const QPoint &point)
{
    if (d->resultTextBrowser)
        return d->resultTextBrowser->anchorAt(point);
    return {};
}

QT_END_NAMESPACE

#include "qhelpsearchresultwidget.moc"
