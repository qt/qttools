// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef PIXMAPEDITOR_H
#define PIXMAPEDITOR_H

#include <QtWidgets/qdialog.h>

#include <QtGui/qpixmap.h>

QT_BEGIN_NAMESPACE

class QLabel;
class QHBoxLayout;
class QToolButton;

class QDesignerFormEditorInterface;

namespace qdesigner_internal {

class DesignerPixmapCache;
class IconThemeEditor;

class IconThemeDialog : public QDialog
{
    Q_OBJECT
public:
    static QString getTheme(QWidget *parent, const QString &theme, bool *ok);
private:
    explicit IconThemeDialog(QWidget *parent);
    IconThemeEditor *m_editor;
};

class PixmapEditor : public QWidget
{
    Q_OBJECT
public:
    explicit PixmapEditor(QDesignerFormEditorInterface *core, QWidget *parent);

    void setSpacing(int spacing);
    void setPixmapCache(DesignerPixmapCache *cache);
    void setIconThemeModeEnabled(bool enabled);
public slots:
    void setPath(const QString &path);
    void setTheme(const QString &theme);
    void setDefaultPixmap(const QPixmap &pixmap);
    void setDefaultPixmapIcon(const QIcon &icon);

signals:
    void pathChanged(const QString &path);
    void themeChanged(const QString &theme);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

private slots:
    void defaultActionActivated();
    void resourceActionActivated();
    void fileActionActivated();
    void themeActionActivated();
#if QT_CONFIG(clipboard)
    void copyActionActivated();
    void pasteActionActivated();
    void clipboardDataChanged();
#endif
private:
    void updateLabels();
    bool m_iconThemeModeEnabled;
    QDesignerFormEditorInterface *m_core;
    QLabel *m_pixmapLabel;
    QLabel *m_pathLabel;
    QToolButton *m_button;
    QAction *m_resourceAction;
    QAction *m_fileAction;
    QAction *m_themeAction;
    QAction *m_copyAction;
    QAction *m_pasteAction;
    QHBoxLayout *m_layout;
    QPixmap m_defaultPixmap;
    QString m_path;
    QString m_theme;
    DesignerPixmapCache *m_pixmapCache;
};

} // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // PIXMAPEDITOR_H
