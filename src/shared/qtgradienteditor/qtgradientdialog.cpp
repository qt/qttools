// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qtgradientdialog_p.h"
#include "ui_qtgradientdialog.h"
#include <QtWidgets/QPushButton>

QT_BEGIN_NAMESPACE

class QtGradientDialogPrivate : public QObject
{
    Q_OBJECT
    QtGradientDialog *q_ptr = nullptr;
    Q_DECLARE_PUBLIC(QtGradientDialog)
public:
    void slotAboutToShowDetails(bool details, int extensionWidthHint);

    Ui::QtGradientDialog m_ui;
};

void QtGradientDialogPrivate::slotAboutToShowDetails(bool details, int extensionWidthHint)
{
    if (details) {
        q_ptr->resize(q_ptr->size() + QSize(extensionWidthHint, 0));
    } else {
        q_ptr->setMinimumSize(1, 1);
        q_ptr->resize(q_ptr->size() - QSize(extensionWidthHint, 0));
        q_ptr->setMinimumSize(0, 0);
    }
}

/*!
    \class QtGradientDialog

    \brief The QtGradientDialog class provides a dialog for specifying gradients.

    The gradient dialog's function is to allow users to edit gradients.
    For example, you might use this in a drawing program to allow the user to set the brush gradient.

    \table
    \row
    \li \inlineimage qtgradientdialog.png
    \li \inlineimage qtgradientdialogextension.png
    \header
    \li Details extension hidden
    \li Details extension visible
    \endtable

    Starting from the top of the dialog there are several buttons:

    \image qtgradientdialogtopbuttons.png

    The first three buttons allow for changing a type of the gradient (QGradient::Type), while the second three allow for
    changing spread of the gradient (QGradient::Spread). The last button shows or hides the details extension of the dialog.
    Conceptually the default view with hidden details provides the full functional control over gradient editing.
    The additional extension with details allows to set gradient's parameters more precisely. The visibility
    of extension can be controlled by detailsVisible property. Moreover, if you don't want the user to
    switch on or off the visibility of extension you can set the detailsButtonVisible property to false.

    Below top buttons there is an area where edited gradient is interactively previewed.
    In addition the user can edit gradient type's specific parameters directly in this area by dragging
    appropriate handles.

    \table
    \row
    \li \inlineimage qtgradientdialoglineareditor.png
    \li \inlineimage qtgradientdialogradialeditor.png
    \li \inlineimage qtgradientdialogconicaleditor.png
    \header
    \li Editing linear type
    \li Editing radial type
    \li Editing conical type
    \row
    \li The user can change the start and final point positions by dragging the circular handles.
    \li The user can change the center and focal point positions by dragging the circular handles
        and can change the gradient's radius by dragging horizontal or vertical line.
    \li The user can change the center point by dragging the circular handle
        and can change the gradient's angle by dragging the big wheel.
    \endtable

    In the middle of the dialog there is an area where the user can edit gradient stops.

    \table
    \row
    \li \inlineimage qtgradientdialogstops.png
    \li \inlineimage qtgradientdialogstopszoomed.png
    \endtable

    The top part of this area contains stop handles, and bottom part shows the preview of gradient stops path.
    In order to create a new gradient stop double click inside the view over the desired position.
    If you double click on existing stop handle in the top part of the view, clicked handle will be duplicated
    (duplicate will contain the same color).
    The stop can be activated by clicking on its handle. You can activate previous or next stop by pressing
    left or right key respectively. To jump to the first or last stop press home or end key respectively.
    The gradient stops editor supports multiselection.
    Clicking a handle holding the shift modifier key down will select a range of stops between
    the active stop and clicked one. Clicking a handle holding control modifier key down will remove from or
    add to selection the clicked stop depending if it was or wasn't already selected respectively.
    Multiselection can also be created using rubberband (by pressing the left mouse button outside
    of any handle and dragging).
    Sometimes it's hard to select a stop because its handle can be partially covered by other handle.
    In that case the user can zoom in the view by spinning mouse wheel.
    The selected stop handles can be moved by drag & drop. In order to remove selected stops press delete key.
    For convenience context menu is provided with the following actions:

    \list
    \li New Stop - creates a new gradient stop
    \li Delete - removes the active and all selected stops
    \li Flip All - mirrors all stops
    \li Select All - selects all stops
    \li Zoom In - zooms in
    \li Zoom Out - zooms out
    \li Zoom All - goes back to original 100% zoom
    \endlist

    The bottom part of the QtGradientDialog contains a set of widgets allowing to control the color of
    the active and selected stops.

    \table
    \row
    \li \inlineimage qtgradientdialogcolorhsv.png
    \li \inlineimage qtgradientdialogcolorrgb.png
    \endtable


    The color button shows the color of the active gradient stop. It also allows for choosing
    a color from standard color dialog and applying it to the
    active stop and all selected stops. It's also possible to drag a color directly from the color button
    and to drop it in gradient stops editor at desired position (it will create new stop with dragged color)
    or at desired stop handle (it will change the color of that handle).

    To the right of color button there is a set of 2 radio buttons which allows to switch between
    HVS and RGB color spec.

    Finally there are 4 color sliders working either in HSVA (hue saturation value alpha) or
    RGBA (red green blue alpha) mode, depending on which radio button is chosen. The radio buttons
    can be controlled programatically by spec() and setSpec() methods. The sliders show the
    color of the active stop. By double clicking inside color slider you can set directly the desired color.
    Changes of slider's are applied to stop selection in the way that the color
    component being changed is applied to stops in selection only, while other components
    remain unchanged in selected stops (e.g. when the user is changing the saturation,
    new saturation is applied to selected stops preventing original hue, value and alpha in multiselection).

    The convenient static functions getGradient() provide modal gradient dialogs, e.g.:

    \snippet doc/src/snippets/code/tools_shared_qtgradienteditor_qtgradientdialog.cpp 0

    In order to have more control over the properties of QtGradientDialog use
    standard QDialog::exec() method:

    \snippet doc/src/snippets/code/tools_shared_qtgradienteditor_qtgradientdialog.cpp 1

    \sa {Gradient View Example}
*/

/*!
    Constructs a gradient dialog with \a parent as parent widget.
*/

QtGradientDialog::QtGradientDialog(QWidget *parent)
    : QDialog(parent), d_ptr(new QtGradientDialogPrivate())
{
//    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    d_ptr->q_ptr = this;
    d_ptr->m_ui.setupUi(this);
    QPushButton *button = d_ptr->m_ui.buttonBox->button(QDialogButtonBox::Ok);
    if (button)
        button->setAutoDefault(false);
    button = d_ptr->m_ui.buttonBox->button(QDialogButtonBox::Cancel);
    if (button)
        button->setAutoDefault(false);
    connect(d_ptr->m_ui.gradientEditor, &QtGradientEditor::aboutToShowDetails,
            d_ptr.data(), &QtGradientDialogPrivate::slotAboutToShowDetails);
}

/*!
    Destroys the gradient dialog
*/

QtGradientDialog::~QtGradientDialog()
{
}

/*!
    \property QtGradientDialog::gradient
    \brief the gradient of the dialog
*/
void QtGradientDialog::setGradient(const QGradient &gradient)
{
    d_ptr->m_ui.gradientEditor->setGradient(gradient);
}

QGradient QtGradientDialog::gradient() const
{
    return d_ptr->m_ui.gradientEditor->gradient();
}

/*!
    \property QtGradientDialog::backgroundCheckered
    \brief whether the background of widgets able to show the colors with alpha channel is checkered.

    \table
    \row
    \li \inlineimage qtgradientdialogbackgroundcheckered.png
    \li \inlineimage qtgradientdialogbackgroundtransparent.png
    \row
    \li \snippet doc/src/snippets/code/tools_shared_qtgradienteditor_qtgradientdialog.cpp 2
    \li \snippet doc/src/snippets/code/tools_shared_qtgradienteditor_qtgradientdialog.cpp 3
    \endtable

    When this property is set to true (the default) widgets inside gradient dialog like color button,
    color sliders, gradient stops editor and gradient editor will show checkered background
    in case of transparent colors. Otherwise the background of these widgets is transparent.
*/

bool QtGradientDialog::isBackgroundCheckered() const
{
    return d_ptr->m_ui.gradientEditor->isBackgroundCheckered();
}

void QtGradientDialog::setBackgroundCheckered(bool checkered)
{
    d_ptr->m_ui.gradientEditor->setBackgroundCheckered(checkered);
}

/*!
    \property QtGradientDialog::detailsVisible
    \brief whether details extension is visible.

    When this property is set to true the details extension is visible. By default
    this property is set to false and the details extension is hidden.

    \sa detailsButtonVisible
*/
bool QtGradientDialog::detailsVisible() const
{
    return d_ptr->m_ui.gradientEditor->detailsVisible();
}

void QtGradientDialog::setDetailsVisible(bool visible)
{
    d_ptr->m_ui.gradientEditor->setDetailsVisible(visible);
}

/*!
    \property QtGradientDialog::detailsButtonVisible
    \brief whether the details button allowing for showing and hiding details extension is visible.

    When this property is set to true (the default) the details button is visible and the user
    can show and hide details extension interactively. Otherwise the button is hidden and the details
    extension is always visible or hidded depending on the value of detailsVisible property.

    \sa detailsVisible
*/
bool QtGradientDialog::isDetailsButtonVisible() const
{
    return d_ptr->m_ui.gradientEditor->isDetailsButtonVisible();
}

void QtGradientDialog::setDetailsButtonVisible(bool visible)
{
    d_ptr->m_ui.gradientEditor->setDetailsButtonVisible(visible);
}

/*!
    Returns the current QColor::Spec used for the color sliders in the dialog.
*/
QColor::Spec QtGradientDialog::spec() const
{
    return d_ptr->m_ui.gradientEditor->spec();
}

/*!
    Sets the current QColor::Spec to \a spec used for the color sliders in the dialog.
*/
void QtGradientDialog::setSpec(QColor::Spec spec)
{
    d_ptr->m_ui.gradientEditor->setSpec(spec);
}

/*!
    Executes a modal gradient dialog, lets the user to specify a gradient, and returns that gradient.

    If the user clicks \gui OK, the gradient specified by the user is returned. If the user clicks \gui Cancel, the \a initial gradient is returned.

    The dialog is constructed with the given \a parent. \a caption is shown as the window title of the dialog and
    \a initial is the initial gradient shown in the dialog. If the \a ok parameter is not-null,
    the value it refers to is set to true if the user clicks \gui OK, and set to false if the user clicks \gui Cancel.
*/
QGradient QtGradientDialog::getGradient(bool *ok, const QGradient &initial, QWidget *parent, const QString &caption)
{
    QtGradientDialog dlg(parent);
    if (!caption.isEmpty())
        dlg.setWindowTitle(caption);
    dlg.setGradient(initial);
    const int res = dlg.exec();
    if (ok) {
        *ok = (res == QDialog::Accepted) ? true : false;
    }
    if (res == QDialog::Accepted)
        return dlg.gradient();
    return initial;
}

/*!
    This method calls getGradient(ok, QLinearGradient(), parent, caption).
*/
QGradient QtGradientDialog::getGradient(bool *ok, QWidget *parent, const QString &caption)
{
    return getGradient(ok, QLinearGradient(), parent, caption);
}

QT_END_NAMESPACE

#include "qtgradientdialog.moc"
