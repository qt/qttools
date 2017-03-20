/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Assistant of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qhelpsearchquerywidget.h"

#include <QtCore/QAbstractListModel>
#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QtGlobal>

#include <QtWidgets/QCompleter>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLayout>
#include <QtWidgets/QLineEdit>
#include <QtGui/QFocusEvent>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QToolButton>

QT_BEGIN_NAMESPACE

class QHelpSearchQueryWidgetPrivate : public QObject
{
    Q_OBJECT

private:
    struct QueryHistory {
        explicit QueryHistory() : curQuery(-1) {}
        QList<QList<QHelpSearchQuery> > queries;
        int curQuery;
    };

    class CompleterModel : public QAbstractListModel
    {
    public:
        explicit CompleterModel(QObject *parent)
          : QAbstractListModel(parent) {}

        int rowCount(const QModelIndex &parent = QModelIndex()) const override
        {
            return parent.isValid() ? 0 : termList.size();
        }

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
        {
            if (!index.isValid() || index.row() >= termList.count()||
                (role != Qt::EditRole && role != Qt::DisplayRole))
                return QVariant();
            return termList.at(index.row());
        }

        void addTerm(const QString &term)
        {
            if (!termList.contains(term)) {
                beginResetModel();
                termList.append(term);
                endResetModel();
            }
        }

    private:
        QStringList termList;
    };

    QHelpSearchQueryWidgetPrivate()
        : QObject()
        , m_compactMode(false)
        , m_searchCompleter(new CompleterModel(this), this)
    {
        m_searchButton = 0;
        m_lineEdit = 0;
    }

    ~QHelpSearchQueryWidgetPrivate()
    {
        // nothing todo
    }

    void retranslate()
    {
        m_searchLabel->setText(QHelpSearchQueryWidget::tr("Search for:"));
        m_prevQueryButton->setToolTip(QHelpSearchQueryWidget::tr("Previous search"));
        m_nextQueryButton->setToolTip(QHelpSearchQueryWidget::tr("Next search"));
        m_searchButton->setText(QHelpSearchQueryWidget::tr("Search"));
    }

    void saveQuery(const QList<QHelpSearchQuery> &query, QueryHistory &queryHist)
    {
        // We only add the query to the list if it is different from the last one.
        bool insert = false;
        if (queryHist.queries.empty())
            insert = true;
        else {
            const QList<QHelpSearchQuery> &lastQuery = queryHist.queries.last();
            if (lastQuery.size() != query.size()) {
                insert = true;
            } else {
                for (int i = 0; i < query.size(); ++i) {
                    if (query.at(i).fieldName != lastQuery.at(i).fieldName
                        || query.at(i).wordList != lastQuery.at(i).wordList) {
                        insert = true;
                        break;
                    }
                }
            }
        }
        if (insert) {
            queryHist.queries.append(query);
            for (const QHelpSearchQuery &queryPart : query) {
                static_cast<CompleterModel *>(m_searchCompleter.model())->
                        addTerm(queryPart.wordList.join(QLatin1Char(' ')));
            }
        }
    }

    void nextOrPrevQuery(int maxOrMinIndex, int addend, QToolButton *thisButton,
        QToolButton *otherButton)
    {
        QueryHistory *queryHist;
        queryHist = &m_queries;
        m_lineEdit->clear();

        // Otherwise, the respective button would be disabled.
        Q_ASSERT(queryHist->curQuery != maxOrMinIndex);

        queryHist->curQuery += addend;
        const QList<QHelpSearchQuery> &query =
            queryHist->queries.at(queryHist->curQuery);
        for (const QHelpSearchQuery &queryPart : query)
            m_lineEdit->setText(queryPart.wordList.join(QLatin1Char(' ')));

        if (queryHist->curQuery == maxOrMinIndex)
            thisButton->setEnabled(false);
        otherButton->setEnabled(true);
    }

    void enableOrDisableToolButtons()
    {
        m_prevQueryButton->setEnabled(m_queries.curQuery > 0);
        m_nextQueryButton->setEnabled(m_queries.curQuery
            < m_queries.queries.size() - 1);
    }

private slots:
    bool eventFilter(QObject *ob, QEvent *event) override
    {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *const keyEvent = static_cast<QKeyEvent *>(event);
            if (keyEvent->key() == Qt::Key_Down) {
                if (m_queries.curQuery + 1 < m_queries.queries.size())
                    nextQuery();
                return true;
            }
            if (keyEvent->key() == Qt::Key_Up) {
                if (m_queries.curQuery > 0)
                    prevQuery();
                return true;
            }

        }
        return QObject::eventFilter(ob, event);
    }

    void searchRequested()
    {
        QList<QHelpSearchQuery> queryList;
        queryList.append(QHelpSearchQuery(QHelpSearchQuery::DEFAULT,
            QStringList(m_lineEdit->text())));
        QueryHistory &queryHist = m_queries;
        saveQuery(queryList, queryHist);
        queryHist.curQuery = queryHist.queries.size() - 1;
        if (queryHist.curQuery > 0)
            m_prevQueryButton->setEnabled(true);
        m_nextQueryButton->setEnabled(false);
    }

    void nextQuery()
    {
        nextOrPrevQuery(m_queries.queries.size() - 1, 1, m_nextQueryButton,
                m_prevQueryButton);
    }

    void prevQuery()
    {
        nextOrPrevQuery(0, -1, m_prevQueryButton, m_nextQueryButton);
    }

private:
    friend class QHelpSearchQueryWidget;

    bool m_compactMode;
    QLabel *m_searchLabel;
    QPushButton *m_searchButton;
    QLineEdit *m_lineEdit;
    QToolButton *m_nextQueryButton;
    QToolButton *m_prevQueryButton;
    QueryHistory m_queries;
    QCompleter m_searchCompleter;
};

#include "qhelpsearchquerywidget.moc"


/*!
    \class QHelpSearchQueryWidget
    \since 4.4
    \inmodule QtHelp
    \brief The QHelpSearchQueryWidget class provides a simple line edit or
    an advanced widget to enable the user to input a search term in a
    standardized input mask.
*/

/*!
    \fn void QHelpSearchQueryWidget::search()

    This signal is emitted when a the user has the search button invoked.
    After receiving the signal you can ask the QHelpSearchQueryWidget for the
    build list of QHelpSearchQuery's that you may pass to the QHelpSearchEngine's
    search() function.
*/

/*!
    Constructs a new search query widget with the given \a parent.
*/
QHelpSearchQueryWidget::QHelpSearchQueryWidget(QWidget *parent)
    : QWidget(parent)
{
    d = new QHelpSearchQueryWidgetPrivate();

    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->setMargin(0);

    QHBoxLayout* hBoxLayout = new QHBoxLayout();
    d->m_searchLabel = new QLabel(this);
    d->m_lineEdit = new QLineEdit(this);
    d->m_lineEdit->setCompleter(&d->m_searchCompleter);
    d->m_lineEdit->installEventFilter(d);
    d->m_prevQueryButton = new QToolButton(this);
    d->m_prevQueryButton->setArrowType(Qt::LeftArrow);
    d->m_prevQueryButton->setEnabled(false);
    d->m_nextQueryButton = new QToolButton(this);
    d->m_nextQueryButton->setArrowType(Qt::RightArrow);
    d->m_nextQueryButton->setEnabled(false);
    d->m_searchButton = new QPushButton(this);
    hBoxLayout->addWidget(d->m_searchLabel);
    hBoxLayout->addWidget(d->m_lineEdit);
    hBoxLayout->addWidget(d->m_prevQueryButton);
    hBoxLayout->addWidget(d->m_nextQueryButton);
    hBoxLayout->addWidget(d->m_searchButton);

    vLayout->addLayout(hBoxLayout);

    connect(d->m_prevQueryButton, SIGNAL(clicked()), d, SLOT(prevQuery()));
    connect(d->m_nextQueryButton, SIGNAL(clicked()), d, SLOT(nextQuery()));
    connect(d->m_searchButton, SIGNAL(clicked()), this, SIGNAL(search()));
    connect(d->m_lineEdit, SIGNAL(returnPressed()), this, SIGNAL(search()));

    d->retranslate();
    connect(this, SIGNAL(search()), d, SLOT(searchRequested()));
    setCompactMode(true);
}

/*!
    Destroys the search query widget.
*/
QHelpSearchQueryWidget::~QHelpSearchQueryWidget()
{
    delete d;
}

/*!
    Expands the search query widget so that the extended search fields are shown.
*/
void QHelpSearchQueryWidget::expandExtendedSearch()
{
    // TODO: no extended search anymore, deprecate it?
}

/*!
    Collapses the search query widget so that only the default search field is
    shown.
*/
void QHelpSearchQueryWidget::collapseExtendedSearch()
{
    // TODO: no extended search anymore, deprecate it?
}

/*!
    Returns a list of queries to use in combination with the search engines
    search(QList<QHelpSearchQuery> &queryList) function.
*/
QList<QHelpSearchQuery> QHelpSearchQueryWidget::query() const
{
    if (d->m_queries.queries.isEmpty())
        return QList<QHelpSearchQuery>();
    return d->m_queries.queries.last();
}

/*!
    Sets the QHelpSearchQueryWidget input fields to the values specified by
    \a queryList search field name. Please note that one has to call the search
    engine's search(QList<QHelpSearchQuery> &queryList) function to perform the
    actual search.
*/
void QHelpSearchQueryWidget::setQuery(const QList<QHelpSearchQuery> &queryList)
{
    d->m_lineEdit->clear();

    for (const QHelpSearchQuery &q : queryList)
        d->m_lineEdit->setText(d->m_lineEdit->text() + q.wordList.join(QChar::Space) + QChar::Space);

    d->searchRequested();
}

bool QHelpSearchQueryWidget::isCompactMode() const
{
    return d->m_compactMode;
}

void QHelpSearchQueryWidget::setCompactMode(bool on)
{
    if (d->m_compactMode != on) {
        d->m_compactMode = on;
        d->m_prevQueryButton->setVisible(!on);
        d->m_nextQueryButton->setVisible(!on);
        d->m_searchLabel->setVisible(!on);
    }
}

/*!
    \reimp
*/
void QHelpSearchQueryWidget::focusInEvent(QFocusEvent *focusEvent)
{
    if (focusEvent->reason() != Qt::MouseFocusReason) {
        d->m_lineEdit->selectAll();
        d->m_lineEdit->setFocus();
    }
}

/*!
    \reimp
*/
void QHelpSearchQueryWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
        d->retranslate();
    else
        QWidget::changeEvent(event);
}

QT_END_NAMESPACE
