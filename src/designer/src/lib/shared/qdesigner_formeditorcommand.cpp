// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0


#include "qdesigner_formeditorcommand_p.h"
#include <QtDesigner/abstractformeditor.h>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

// ---- QDesignerFormEditorCommand ----
QDesignerFormEditorCommand::QDesignerFormEditorCommand(const QString &description, QDesignerFormEditorInterface *core)
    : QUndoCommand(description),
      m_core(core)
{
}

QDesignerFormEditorInterface *QDesignerFormEditorCommand::core() const
{
    return m_core;
}

} // namespace qdesigner_internal

QT_END_NAMESPACE
