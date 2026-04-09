// Copyright (C) 2017 Witekio.
// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include <QtCoap/qcoapglobal.h>
#include <QtCore/qobject.h>
#include <QtCore/qrandom.h>

#ifndef QCOAPNAMESPACE_H
#define QCOAPNAMESPACE_H

QT_BEGIN_NAMESPACE

#define FOR_EACH_COAP_ERROR(X) \
    X(BadRequest, 0x80) X(Unauthorized, 0x81) X(BadOption, 0x82) X(Forbidden, 0x83)  \
    X(NotFound, 0x84) X(MethodNotAllowed, 0x85) X(NotAcceptable, 0x86) \
    X(RequestEntityIncomplete, 0x88) X(PreconditionFailed, 0x8C) X(RequestEntityTooLarge, 0x8D) \
    X(UnsupportedContentFormat, 0x8E) X(InternalServerFault, 0xA0) X(NotImplemented, 0xA1) \
    X(BadGateway, 0xA2) X(ServiceUnavailable, 0xA3) X(GatewayTimeout, 0xA4) \
    X(ProxyingNotSupported, 0xA5)

namespace QtCoap
{
    Q_NAMESPACE_EXPORT(Q_COAP_EXPORT)

    enum class ResponseCode : quint8 {
        EmptyMessage = 0x00,
        Created = 0x41, // 2.01
        Deleted = 0x42, // 2.02
        Valid   = 0x43, // 2.03
        Changed = 0x44, // 2.04
        Content = 0x45, // 2.05
        Continue = 0x5F, // 2.31

#define SINGLE_CODE(name, value) name = value,
        FOR_EACH_COAP_ERROR(SINGLE_CODE)
#undef SINGLE_CODE

        InvalidCode = 0xFF
    };
    Q_ENUM_NS(ResponseCode)

    enum class Error : quint8 {
        Ok,
        HostNotFound,
        AddressInUse,
        TimeOut,

#define SINGLE_ERROR(name, ignored) name,
        FOR_EACH_COAP_ERROR(SINGLE_ERROR)
#undef SINGLE_ERROR

        Unknown
    };
    Q_ENUM_NS(Error)

    enum class Method : quint8 {
        Invalid,
        Get,
        Post,
        Put,
        Delete,
#if 0
        //! TODO Support other methods included in RFC 8132
        //! https://tools.ietf.org/html/rfc8132
        Fetch,
        Patch,
        IPatch,
#endif
        Other
    };
    Q_ENUM_NS(Method)

    enum Port {
        DefaultPort = 5683,
        DefaultSecurePort = 5684
    };
    Q_ENUM_NS(Port)

    enum class SecurityMode : quint8 {
        NoSecurity = 0,
        PreSharedKey,
        RawPublicKey,
        Certificate
    };
    Q_ENUM_NS(SecurityMode)

    enum class MulticastGroup : quint8 {
        AllCoapNodesIPv4,
        AllCoapNodesIPv6LinkLocal,
        AllCoapNodesIPv6SiteLocal
    };
    Q_ENUM_NS(MulticastGroup)

    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")
}

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QtCoap::ResponseCode)
Q_DECLARE_METATYPE(QtCoap::Error)
Q_DECLARE_METATYPE(QtCoap::Method)
Q_DECLARE_METATYPE(QtCoap::SecurityMode)
Q_DECLARE_METATYPE(QtCoap::MulticastGroup)

#endif // QCOAPNAMESPACE_H
