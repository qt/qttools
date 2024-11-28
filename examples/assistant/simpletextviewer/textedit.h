// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <QTextEdit>
#include <QUrl>

class TextEdit : public QTextEdit
{
    Q_OBJECT

public:
    TextEdit(QWidget *parent = nullptr);
    void setContents(const QString &fileName);

signals:
    void fileNameChanged(const QString &fileName);

private:
    QVariant loadResource(int type, const QUrl &name) override;
    QUrl srcUrl;
};

#endif
