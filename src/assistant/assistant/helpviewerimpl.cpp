/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Assistant of the Qt Toolkit.
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

#include "helpviewerimpl.h"
#include "helpviewerimpl_p.h"

#include "helpenginewrapper.h"
#include "tracer.h"

#include <QtCore/QCoreApplication>

#include <QtGui/QMouseEvent>

QT_BEGIN_NAMESPACE

const QString HelpViewerImpl::AboutBlank =
    QCoreApplication::translate("HelpViewer", "<title>about:blank</title>");

const QString HelpViewerImpl::LocalHelpFile = QLatin1String("qthelp://"
    "org.qt-project.qtassistant.%1%2%3/qtassistant/assistant-quick-guide.html")
        .arg(QString::number(QT_VERSION_MAJOR),
             QString::number(QT_VERSION_MINOR),
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
