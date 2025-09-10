// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef DISTANCEFIELDMODELWORKER_H
#define DISTANCEFIELDMODELWORKER_H

#include <QObject>
#include <QRawFont>
#include <QtGui/private/qtextengine_p.h>

QT_BEGIN_NAMESPACE

struct CmapSubtable0;
struct CmapSubtable4;
struct CmapSubtable6;
struct CmapSubtable10;
struct CmapSubtable12;
class DistanceFieldModelWorker : public QObject
{
    Q_OBJECT
public:
    explicit DistanceFieldModelWorker(QObject *parent = nullptr);

    Q_INVOKABLE void generateOneDistanceField();
    Q_INVOKABLE void loadFont(const QString &fileName);

    void readCmapSubtable(const CmapSubtable0 *subtable, const void *end);
    void readCmapSubtable(const CmapSubtable4 *subtable, const void *end);
    void readCmapSubtable(const CmapSubtable6 *subtable, const void *end);
    void readCmapSubtable(const CmapSubtable10 *subtable, const void *end);
    void readCmapSubtable(const CmapSubtable12 *subtable, const void *end);

signals:
    void fontLoaded(quint16 glyphCount, bool doubleResolution, qreal pixelSize);
    void fontGenerated();
    void distanceFieldGenerated(const QImage &distanceField,
                                const QPainterPath &path,
                                glyph_t glyphId,
                                quint32 cmapAssignment);
    void error(const QString &errorString);

private:
    void readGlyphCount();
    void readCmap();

    QRawFont m_font;
    quint16 m_glyphCount;
    quint16 m_nextGlyphId;
    bool m_doubleGlyphResolution;
    QHash<glyph_t, quint32> m_cmapping;
};

QT_END_NAMESPACE

#endif // DISTANCEFIELDMODELWORKER_H
