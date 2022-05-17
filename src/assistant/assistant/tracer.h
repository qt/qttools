// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

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
