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

#ifndef TRACER_H
#define TRACER_H

#include <QtCore/QtGlobal>

QT_BEGIN_NAMESPACE

class Tracer
{
public:
    Tracer(const char *func) : m_func(func)
    {
        qDebug("Entering function %s.", m_func);
    }

    ~Tracer()
    {
        qDebug("Leaving function %s.", m_func);
    }

private:
    const char * const m_func;
};

QT_END_NAMESPACE

// #define TRACING_REQUESTED
#ifdef TRACING_REQUESTED
#define TRACE_OBJ Tracer traceObj__(Q_FUNC_INFO);
#else
#define TRACE_OBJ
#endif

#endif // TRACER_H
