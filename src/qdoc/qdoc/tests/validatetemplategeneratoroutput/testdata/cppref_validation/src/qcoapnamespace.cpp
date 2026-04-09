// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qcoapnamespace_p.h"

QT_BEGIN_NAMESPACE

/*!
    \namespace QtCoap

    \inmodule QtCoap
    \brief Contains miscellaneous identifiers used throughout the Qt CoAP module.
    \keyword {QtCoap Namespace}
*/

/*!
    \enum QtCoap::ResponseCode

    This enum represents the response code from the CoAP protocol, as defined in
    \l{https://tools.ietf.org/html/rfc7252#section-5.9}{RFC 7252} and
    \l{https://tools.ietf.org/html/rfc7959#section-2.9}{RFC 7959}.

    \value EmptyMessage             The response code for an empty message.

    \value Created                  The request was successful and has resulted in new resources
                                    being created.
                                    This response code corresponds to HTTP 201 "Created".

    \value Deleted                  The resource has been successfully deleted. This response code
                                    corresponds to HTTP 204 "No Content" but is only used in
                                    response to requests that cause the resource to cease being
                                    available, such as DELETE and, in certain circumstances, POST.

    \value Valid                    This response code is related to HTTP 304 "Not Modified" but
                                    is only used to indicate that the response identified by the
                                    entity-tag given in the ETag Option is valid.

    \value Changed                  The resource has been successfully modified.
                                    This response code corresponds to HTTP 204 "No Content" but
                                    is only used in response to POST and PUT requests.

    \value Content                  The request has succeeded. This response code corresponds to
                                    HTTP 200 "OK" but is only used in response to GET requests.

    \value Continue                 Indicates that the transfer of the current block was successful,
                                    and the server will send more blocks.

    \value BadRequest               The request was not recognized.
                                    This response code corresponds to HTTP 400 "Bad Request".

    \value Unauthorized             The client is not authorized to perform the requested action.
                                    This response code corresponds to HTTP 401 "Unauthorized".

    \value BadOption                The request could not be understood by the server due to
                                    unrecognized options.

    \value Forbidden                Access to this resource is forbidden.
                                    This response code corresponds to HTTP 403 "Forbidden".

    \value NotFound                 The resource requested was not found.
                                    This response code corresponds to HTTP 404 "Not Found".

    \value MethodNotAllowed         The server does not allow the method used for the URL requested.
                                    This response code corresponds to HTTP 405 "Method Not Allowed"
                                    but with no parallel to the "Allow" header field.

    \value NotAcceptable            No resource satisfying the request's acceptance criteria
                                    was found. This response code corresponds to HTTP 406
                                    "Not Acceptable", but with no response entity.

    \value RequestEntityIncomplete  The server has not received all blocks, of the request body,
                                    that it needs to proceed.

    \value PreconditionFailed       Preconditions given in the request header fields evaluated to
                                    \c false when tested on the server.
                                    This response code corresponds to HTTP 412
                                    "Precondition Failed".

    \value RequestEntityTooLarge    The request payload is larger than the server is willing or
                                    able to process. This response code corresponds to HTTP 413
                                    "Request Entity Too Large".

    \value UnsupportedContentFormat The payload is in a format not supported by this method on
                                    the target resource. This response code corresponds to HTTP 415
                                    "Unsupported Media Type".

    \value InternalServerFault      The server encountered an unexpected condition that prevented
                                    it from fulfilling the request. This response code corresponds
                                    to HTTP 500 "Internal Server Error".

    \value NotImplemented           The server does not support the functionality required to
                                    fulfill the request. This response code corresponds to HTTP 501
                                    "Not Implemented".

    \value BadGateway               An error occurred with an upstream server.
                                    This response code corresponds to HTTP 502 "Bad Gateway".

    \value ServiceUnavailable       The service is currently unavailable.
                                    This response code corresponds to HTTP 503
                                    "Service Unavailable".

    \value GatewayTimeout           The server, while acting as a gateway or proxy, did not
                                    receive a timely response from an upstream server.
                                    This response code corresponds to HTTP 504 "Gateway Timeout".

    \value ProxyingNotSupported     The server is unable or unwilling to act as a forward-proxy
                                    for the URI specified in the Proxy-Uri Option or using
                                    the scheme specified in Proxy-Scheme.

    \value InvalidCode              An invalid response code.
*/

/*!
    \enum QtCoap::Error

    Indicates the error condition found during processing of the request.

    \value Ok                               No error condition.

    \value HostNotFound                     The remote host name was not found.

    \value AddressInUse                     The address is already in use.

    \value TimeOut                          The response did not arrive in time.

    \value BadRequest                       The request was not recognized.

    \value Unauthorized                     The client is not authorized to perform
                                            the requested action.

    \value BadOption                        The request could not be understood by
                                            the server due to one or more unrecognized
                                            or malformed options.

    \value Forbidden                        Access to this resource is forbidden.

    \value NotFound                         The resource requested was not found.

    \value MethodNotAllowed                 The server does not allow the method used
                                            for the URL requested.

    \value NotAcceptable                    No resource satisfying the request's acceptance
                                            criteria was found.

    \value RequestEntityIncomplete          The server has not received the blocks of
                                            the request body that it needs to proceed.
                                            The client has not sent all blocks,
                                            has not sent them in the order required by the
                                            server, or sent them long enough ago
                                            that the server has already discarded them.

    \value PreconditionFailed               One or more conditions given in the request
                                            header fields evaluated to false when tested
                                            on the server.

    \value RequestEntityTooLarge            The request payload is larger than the
                                            server is willing or able to process.

    \value UnsupportedContentFormat         The payload is in a format not supported
                                            by this method on the target resource.

    \value InternalServerFault              The server encountered an unexpected
                                            condition that prevented it from
                                            fulfilling the request.

    \value NotImplemented                   The server does not support the
                                            functionality required to fulfill the
                                            request.

    \value BadGateway                       An error occurred with an upstream
                                            server.

    \value ServiceUnavailable               The service is currently unavailable.

    \value GatewayTimeout                   The server, while acting as a gateway
                                            or proxy, did not receive a timely
                                            response from an upstream server it needed
                                            to access in order to complete the request.

    \value ProxyingNotSupported             The server is unable or unwilling to act
                                            as a forward-proxy for the URI specified
                                            in the Proxy-Uri Option or using the scheme
                                            specified in Proxy-Scheme.

    \value Unknown                          An unknown error occurred.
*/

/*!
    \enum QtCoap::Method

    This enum specifies CoAP request methods.

    \value Invalid                  The default request method for an empty request.
    \value Get                      GET method.
    \value Post                     POST method.
    \value Put                      PUT method.
    \value Delete                   DELETE method.
    \value Other                    Other request method.
*/

/*!
    \enum QtCoap::Port

    This enum specifies the default CoAP port values.

    \value DefaultPort              The default port used for the non-secure transmission.
    \value DefaultSecurePort        The default port used for the secure transmission.
*/

/*!
  \enum QtCoap::SecurityMode

  Specifies the security mode used for securing a CoAP connection, as defined in
  \l{https://tools.ietf.org/html/rfc7252#section-9}{RFC 7252}.

  \value NoSecurity         There is no protocol-level security (DTLS is disabled).

  \value PreSharedKey       DTLS is enabled. PSK authentication will be used for security.

  \value RawPublicKey       DTLS is enabled. An asymmetric key pair without a certificate
                            (a raw public key) will be used for security. This mode is not
                            supported yet.

  \value Certificate        DTLS is enabled. An asymmetric key pair with an X.509 certificate
                            will be used for security.
*/

/*!
    \enum QtCoap::MulticastGroup

    This enum represents CoAP multicast group addresses defined in
    \l{https://tools.ietf.org/html/rfc7252#section-12.8}{RFC 7252}.

    \value AllCoapNodesIPv4             IPv4  "All CoAP Nodes" address \e {224.0.1.187}, from
                                        the "IPv4 Multicast Address Space Registry".

    \value AllCoapNodesIPv6LinkLocal    IPv6 "All CoAP Nodes" link-local scoped address
                                        \e {FF02::FD}, from the "IPv6 Multicast Address Space
                                        Registry".

    \value AllCoapNodesIPv6SiteLocal    IPv6 "All CoAP Nodes" site-local scoped address
                                        \e {FF05::FD}, from the "IPv6 Multicast Address Space
                                        Registry".
*/

/*!
    \internal

    Returns \c true if \a code corresponds to an error, returns \c false otherwise.
*/
bool QtCoap::isError(QtCoap::ResponseCode code)
{
    return static_cast<int>(code) >= 0x80;
}

/*!
    \internal

    Returns the QtCoap::Error corresponding to the \a code passed to this
    method.
*/
QtCoap::Error QtCoap::errorForResponseCode(QtCoap::ResponseCode code)
{
    if (!isError(code))
        return QtCoap::Error::Ok;

    switch (code) {
#define SINGLE_CASE(name, ignored) case ResponseCode::name: return Error::name;
        FOR_EACH_COAP_ERROR(SINGLE_CASE)
#undef SINGLE_CASE
    default:
        return Error::Unknown;
    }
}

/*!
    \internal

    Returns the internal random generator used for generating token values and
    message IDs.
*/
QRandomGenerator &QtCoap::randomGenerator()
{
    static QRandomGenerator randomGenerator = QRandomGenerator::securelySeeded();
    return randomGenerator;
}

QT_END_NAMESPACE
