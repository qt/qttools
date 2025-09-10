// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

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
