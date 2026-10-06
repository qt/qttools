// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef MEMBER_LINK_CONTEXT_H
#define MEMBER_LINK_CONTEXT_H

/*!
    \class MemberLinkContext
    \inmodule MemberLink
    \brief A class that carries the member link context test cases.

    The detailed description of the page links to the beta member:
    \l beta

    That link is authored in the page body, so it must keep resolving
    in the page context and reach the member anchor of this page.
*/
class MemberLinkContext
{
public:
    /*!
        Alpha does its own work. The other member, \l beta, does related
        work.

        \target member-detail
        \section1 Alpha Notes
        Follow \l member-detail.

        \sa beta, member-detail
    */
    void alpha();

    /*!
        Beta does related work. This member also links to a member that
        does not exist: \l MissingMember

        \target member-detail
        \section1 Beta Notes
        Follow \l member-detail.

        \sa member-detail
    */
    void beta();

    /*!
        \fn void MemberLinkContext::sharedOne()
        \fn void MemberLinkContext::sharedTwo()

        Both shared operations do the same work. They refer to \l beta
        from the shared comment that documents the pair.
    */
    void sharedOne();
    void sharedTwo();
};

#endif // MEMBER_LINK_CONTEXT_H
