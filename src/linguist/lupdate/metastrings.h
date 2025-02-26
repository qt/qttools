// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef METASTRINGS_H
#define METASTRINGS_H

#include <QString>
#include <QHash>

QT_BEGIN_NAMESPACE

class MetaStrings
{
public:
    struct MagicComment
    {
        QString context;
        QString comment;
    };

    bool parse(QString &string);

    void clear();

    bool hasData() const;

    std::optional<MagicComment> magicComment() const { return m_magicComment; }
    QString extracomment() const { return m_extracomment; }
    QString msgid() const { return m_msgid; }
    QString sourcetext() const { return m_sourcetext; }
    QString label() const { return m_label; }
    QHash<QString, QString> extra() const { return m_extra; }
    QString popError() { return std::move(m_error); }

private:
    std::optional<MagicComment> m_magicComment; // TRANSLATOR
    QString m_extracomment; //:
    QString m_msgid; //=
    QString m_sourcetext; //%
    QString m_label; //@
    QHash<QString, QString> m_extra; //~
    QString m_error;
};

QT_END_NAMESPACE

#endif // METASTRINGS_H
