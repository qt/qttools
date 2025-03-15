// Copyright (C) 2025 The Qt Company Ltd
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

struct Tr
{
    Q_DECLARE_TR_FUNCTIONS(QtC::Git)
};


void func()
{
    QStringList headers; // Keep in sync with GerritChange::toHtml()
    headers << "#" << Tr::tr("Subject") << Tr::tr("Owner")
            << Tr::tr("Updated") << Tr::tr("Project")
            << Tr::tr("Approvals") << Tr::tr("Status");

    const QString help
        = Tr::tr("Macro mode. Type \"%1\" to stop recording and \"%2\" to play the macro.")
              .arg(endShortcut, executeShortcut);
    QPushButton *acceptButton
        = buttonBox.addButton(Tr::tr("Accept"), QDialogButtonBox::ButtonRole::YesRole);
            
    auto v = m_separatedView->prepareObject<ImageViewer>(item);
            v->setInfo(item->address
                           ? Tr::tr("%1 Object at %2").arg(item->type, item->hexAddress())
                           : Tr::tr("%1 Object at Unknown Address").arg(item->type) + "    "
                                 + Tr::tr("Size: %1x%2, %3 byte, format: %4, depth: %5")
                                       .arg(width)
                                       .arg(height)
                                       .arg(nbytes)
                                       .arg(im.format())
                                       .arg(im.depth()));

}


