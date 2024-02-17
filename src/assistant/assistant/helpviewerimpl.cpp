// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "helpviewerimpl.h"
#include "helpviewerimpl_p.h"

#include "helpenginewrapper.h"
#include "tracer.h"

#include <QtCore/QCoreApplication>

#include <QtGui/QMouseEvent>

QT_BEGIN_NAMESPACE

const QString HelpViewerImpl::AboutBlank =
    QCoreApplication::translate("HelpViewer", "<title>about:blank</title>");

const QString HelpViewerImpl::LocalHelpFile =
        "qthelp://org.qt-project.qtassistant.%1%2%3/qtassistant/assistant-quick-guide.html"_L1.arg(
                QString::number(QT_VERSION_MAJOR), QString::number(QT_VERSION_MINOR),
                QString::number(QT_VERSION_PATCH));

const QString HelpViewerImpl::PageNotFoundMessage =
    QCoreApplication::translate("HelpViewer", "<title>Error 404...</title><div "
    "align=\"center\"><br><br><h1>The page could not be found.</h1><br><h3>'%1'"
    "</h3></div>");

HelpViewerImpl::~HelpViewerImpl()
{
    TRACE_OBJ
    delete d;
}

// -- public slots

void HelpViewerImpl::home()
{
    TRACE_OBJ
    setSource(HelpEngineWrapper::instance().homePage());
}

// -- private slots

void HelpViewerImpl::setLoadFinished()
{
    emit sourceChanged(source());
}

// -- private

bool HelpViewerImpl::handleForwardBackwardMouseButtons(QMouseEvent *event)
{
    TRACE_OBJ
    if (event->button() == Qt::XButton1) {
        backward();
        return true;
    }

    if (event->button() == Qt::XButton2) {
        forward();
        return true;
    }

    return false;
}

QT_END_NAMESPACE
