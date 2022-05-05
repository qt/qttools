/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Designer of the Qt Toolkit.
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

#ifndef EMBEDDEDOPTIONSPAGE_H
#define EMBEDDEDOPTIONSPAGE_H

#include <QtDesigner/abstractoptionspage.h>
#include <QtCore/qpointer.h>
#include <QtWidgets/qwidget.h>

QT_BEGIN_NAMESPACE

class QDesignerFormEditorInterface;

namespace qdesigner_internal {

class EmbeddedOptionsControlPrivate;

/* EmbeddedOptions Control. Presents the user with a list of embedded
 * device profiles he can modify/add/delete. */
class EmbeddedOptionsControl : public QWidget {
    Q_DISABLE_COPY_MOVE(EmbeddedOptionsControl)
    Q_OBJECT
public:
    explicit EmbeddedOptionsControl(QDesignerFormEditorInterface *core, QWidget *parent = nullptr);
    ~EmbeddedOptionsControl();

    bool isDirty() const;

public slots:
    void loadSettings();
    void saveSettings();

private slots:
    void slotAdd();
    void slotEdit();
    void slotDelete();
    void slotProfileIndexChanged(int);

private:
    friend class EmbeddedOptionsControlPrivate;

    EmbeddedOptionsControlPrivate *m_d;
};

// EmbeddedOptionsPage
class EmbeddedOptionsPage : public QDesignerOptionsPageInterface
{
    Q_DISABLE_COPY_MOVE(EmbeddedOptionsPage)
public:
    explicit EmbeddedOptionsPage(QDesignerFormEditorInterface *core);

    QString name() const override;
    QWidget *createPage(QWidget *parent) override;
    void finish() override;
    void apply() override;

private:
    QDesignerFormEditorInterface *m_core;
    QPointer<EmbeddedOptionsControl> m_embeddedOptionsControl;
};

} // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // EMBEDDEDOPTIONSPAGE_H
