// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef PALETTEEDITOR_H
#define PALETTEEDITOR_H

#include "ui_paletteeditor.h"
#include <QtWidgets/qitemdelegate.h>

QT_BEGIN_NAMESPACE

class QAction;
class QListView;
class QMenu;
class QLabel;
class QtColorButton;
class QDesignerFormEditorInterface;

namespace qdesigner_internal {

class PaletteEditor: public QDialog
{
    Q_OBJECT
public:
    ~PaletteEditor() override;

    static QPalette getPalette(QDesignerFormEditorInterface *core,
                QWidget* parent, const QPalette &init = QPalette(),
                const QPalette &parentPal = QPalette(), int *result = nullptr);

    QPalette palette() const;
    void setPalette(const QPalette &palette);
    void setPalette(const QPalette &palette, const QPalette &parentPalette);

private slots:

    void buildButtonColorChanged();
    void activeRadioClicked();
    void inactiveRadioClicked();
    void disabledRadioClicked();
    void computeRadioClicked();
    void detailsRadioClicked();

    void paletteChanged(const QPalette &palette);
    void viewContextMenuRequested(QPoint pos);
    void save();
    void load();

protected:

private:
    PaletteEditor(QDesignerFormEditorInterface *core, QWidget *parent);
    void buildPalette();

    void updatePreviewPalette();
    void updateStyledButton();

    QPalette::ColorGroup currentColorGroup() const
        { return m_currentColorGroup; }

    Ui::PaletteEditor ui;
    QPalette m_editPalette;
    QPalette m_parentPalette;
    class PaletteModel *m_paletteModel;
    QDesignerFormEditorInterface *m_core;
    QAction *m_lighterAction = nullptr;
    QAction *m_darkerAction = nullptr;
    QAction *m_copyColorAction = nullptr;
    QMenu *m_contextMenu = nullptr;
    QPalette::ColorGroup m_currentColorGroup = QPalette::Active;
    bool m_modelUpdated = false;
    bool m_paletteUpdated = false;
    bool m_compute = true;
};


class PaletteModel : public QAbstractTableModel
{
    Q_OBJECT
    Q_PROPERTY(QPalette::ColorRole colorRole READ colorRole)
public:
    explicit PaletteModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                int role = Qt::DisplayRole) const override;

    QPalette getPalette() const;
    void setPalette(const QPalette &palette, const QPalette &parentPalette);

    QBrush brushAt(const QModelIndex &index) const;

    QPalette::ColorRole colorRole() const { return QPalette::NoRole; }
    void setCompute(bool on) { m_compute = on; }

    quint64 rowMask(const QModelIndex &index) const;

signals:
    void paletteChanged(const QPalette &palette);
private:
    struct RoleEntry
    {
        QString name;
        QPalette::ColorRole role;
    };

    QPalette::ColorGroup columnToGroup(int index) const;
    int groupToColumn(QPalette::ColorGroup group) const;
    QPalette::ColorRole roleAt(int row) const { return m_roleEntries.at(row).role; }
    int rowOf(QPalette::ColorRole role) const;

    QPalette m_palette;
    QPalette m_parentPalette;
    QList<RoleEntry> m_roleEntries;
    bool m_compute = true;
};

class BrushEditor : public QWidget
{
    Q_OBJECT
public:
    explicit BrushEditor(QDesignerFormEditorInterface *core, QWidget *parent = nullptr);

    void setBrush(const QBrush &brush);
    QBrush brush() const;
    bool changed() const;
signals:
    void changed(QWidget *widget);
private slots:
    void brushChanged();
private:
    QtColorButton *m_button;
    bool m_changed = false;
    QDesignerFormEditorInterface *m_core;
};

class RoleEditor : public QWidget
{
    Q_OBJECT
public:
    explicit RoleEditor(QWidget *parent = nullptr);

    void setLabel(const QString &label);
    void setEdited(bool on);
    bool edited() const;
signals:
    void changed(QWidget *widget);
private slots:
    void emitResetProperty();
private:
    QLabel *m_label;
    bool m_edited = false;
};

class ColorDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    explicit ColorDelegate(QDesignerFormEditorInterface *core, QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    void setEditorData(QWidget *ed, const QModelIndex &index) const override;
    void setModelData(QWidget *ed, QAbstractItemModel *model,
                      const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *ed, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;

    void paint(QPainter *painter, const QStyleOptionViewItem &opt,
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const override;
private:
    QDesignerFormEditorInterface *m_core;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // PALETTEEDITOR_H
