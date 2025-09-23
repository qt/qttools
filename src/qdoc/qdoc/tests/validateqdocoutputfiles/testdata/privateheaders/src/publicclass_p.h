// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "publicclass.h"

/*!
    \internal
    \class PublicClass::PrivateData
    \inmodule PrivateHeadersModule
    \brief Private implementation class for PublicClass.

    This class is in a private header (_p.h) and contains internal
    implementation details. It should only appear in documentation
    when showinternal is enabled AND private headers are being processed.
*/
class PublicClass::PrivateData
{
public:
    /*!
        \internal
        \brief Constructor for private data.

        This should only appear when showinternal is enabled.
    */
    PrivateData();

    /*!
        \internal
        \brief Internal helper function.

        This function handles internal implementation details.
    */
    void internalHelper();

    /*!
        \internal
        \brief Internal data member.

        Stores internal state for the public class.
    */
    int internalData = 0;

    /*!
        \brief Regular private member.

        This is a regular (non-internal) private member in a private header.
        It should appear when showinternal is enabled, regardless of
        the \internal marking.
    */
    bool regularPrivateMember = false;

    /*!
        \internal
        \variable PublicClass::PrivateData::headerCache
        \brief Cached header data with anonymous struct pattern from Qt.

        Replicates the pattern from qnetworkrequest_p.h that causes
        anonymous type names with full file paths.
    */
    mutable struct {
        QList<QPair<QByteArray, QByteArray>> headersList;
        bool isCached = false;
        int cacheVersion = 1;
    } headerCache;

    /*!
        \internal
        \variable PublicClass::PrivateData::addressData
        \brief Network address data with complex anonymous union.

        Similar to qhostaddress_p.h pattern with nested anonymous types.
    */
    union {
        struct {
            quint32 ipv4Address;
            quint16 port;
        } ipv4;
        struct {
            quint64 addressParts[2];
            struct { quint32 components[4]; } ipv6_32;
            struct { quint64 components[2]; } ipv6_64;
        } ipv6;
        struct {
            char rawData[16];
            bool isValid;
        } raw;
    } addressData;

    /*!
        \internal
        \variable PublicClass::PrivateData::requestMetadata
        \brief Multiple nested anonymous structures for disambiguation.

        Creates complex anonymous type hierarchy to force Clang path naming.
    */
    struct {
        mutable struct {
            bool hasAuth = false;
            union {
                struct { char username[64]; char password[64]; } basic;
                struct { char token[256]; int expiryTime; } bearer;
            } credentials;
        } authentication;
        struct {
            union {
                struct { int major, minor, patch; } version;
                quint32 combined;
            } httpVersion;
            struct {
                bool keepAlive;
                int timeoutSeconds;
            } connection;
        } protocol;
    } requestMetadata;
};

/*!
    \internal
    \class PrivateOnlyClass
    \inmodule PrivateHeadersModule
    \brief A class that exists only in private headers.

    This entire class is defined in a private header and marked internal.
    It should only appear when both showinternal is enabled AND
    private headers are being processed by QDoc.
*/
class PrivateOnlyClass
{
public:
    /*!
        \internal
        \brief Constructor of private-only class.
    */
    PrivateOnlyClass();

    /*!
        \internal
        \brief Internal method in private class.
    */
    void privateClassMethod();
};

/*!
    \internal
    \brief Internal global function in private header.
    \relates PrivateHeadersModule

    This function exists only in the private header and should only
    appear when showinternal is enabled and private headers are processed.
*/
void internalPrivateHeaderFunction();

