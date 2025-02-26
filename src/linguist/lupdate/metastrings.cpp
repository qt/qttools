// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "metastrings.h"

namespace {
using namespace Qt::Literals::StringLiterals;

static constexpr QLatin1String CppMagicComment = "TRANSLATOR"_L1;
} // namespace

QT_BEGIN_NAMESPACE

bool MetaStrings::parse(QString &string)
{
    const QChar *ptr = string.unicode();
    if (*ptr == u':' && ptr[1].isSpace()) {
        string.remove(0, 2);
        m_extracomment += string;
        if (!m_extracomment.endsWith(u'\n'))
            m_extracomment.push_back(u'\n');
        m_extracomment.detach();
    } else if (*ptr == u'=' && ptr[1].isSpace()) {
        string.remove(0, 2);
        m_msgid = string.simplified();
        m_msgid.detach();
    } else if (*ptr == u'~' && ptr[1].isSpace()) {
        string.remove(0, 2);
        const QString trimmed = string.trimmed();
        int k = trimmed.indexOf(u' ');
        if (k > -1) {
            QString commentvalue = trimmed.mid(k + 1).trimmed();
            if (commentvalue.startsWith(u'"') && commentvalue.endsWith(u'"')
                && commentvalue.size() != 1) {
                commentvalue = commentvalue.sliced(1, commentvalue.size() - 2);
            }
            m_extra.insert(trimmed.left(k), commentvalue);
        }
    } else if (*ptr == u'%' && ptr[1].isSpace()) {
        m_sourcetext.reserve(m_sourcetext.size() + string.size() - 2);
        ushort *ptr = (ushort *)m_sourcetext.data() + m_sourcetext.size();
        int p = 2, c;
        forever {
            if (p >= string.size())
                break;
            c = string.unicode()[p++].unicode();
            if (isspace(c))
                continue;
            if (c != '"') {
                m_error = "Unexpected character in meta string\n"_L1;
                break;
            }
            forever {
                if (p >= string.size()) {
                whoops:
                    m_error = "Unterminated meta string\n"_L1;
                    break;
                }
                c = string.unicode()[p++].unicode();
                if (c == '"')
                    break;
                if (c == '\\') {
                    if (p >= string.size())
                        goto whoops;
                    c = string.unicode()[p++].unicode();
                    if (c == '\n')
                        goto whoops;
                    *ptr++ = '\\';
                }
                *ptr++ = c;
            }
        }
        m_sourcetext.resize(ptr - (ushort *)m_sourcetext.data());
    } else if (*ptr == u'@' && ptr[1].isSpace()) {
        string.remove(0, 2);
        m_label = string.trimmed().simplified();
        m_label.detach();
    } else if (const QString trimmed = string.trimmed(); trimmed.startsWith(CppMagicComment)) {
        qsizetype idx = CppMagicComment.size();
        QString comment =
                QString::fromRawData(trimmed.unicode() + idx, trimmed.size() - idx).simplified();
        if (int k = comment.indexOf(u' '); k != -1) {
            QString context = comment.left(k);
            comment.remove(0, k + 1);
            m_magicComment.emplace(MagicComment{ std::move(context), std::move(comment) });
        }
    }

    return m_error.isEmpty();
}

void MetaStrings::clear()
{
    m_magicComment.reset();
    m_extracomment.clear();
    m_msgid.clear();
    m_label.clear();
    m_sourcetext.clear();
    m_extra.clear();
}

bool MetaStrings::hasData() const
{
    return !m_msgid.isEmpty() || m_magicComment || !m_sourcetext.isEmpty()
            || !m_extracomment.isEmpty() || !m_extra.isEmpty() || !m_label.isEmpty();
}

QT_END_NAMESPACE
