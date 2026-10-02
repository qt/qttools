**Contents**

- [Types](#types)
- [Type Documentation](#type-documentation)

# QtCoap

namespace QtCoap

Contains miscellaneous identifiers used throughout the Qt CoAP module.

| Key | Value |
| --- | --- |
| Header | `qcoapnamespace.h` |
| CMake | `find_package(Qt6 REQUIRED COMPONENTS Coap)` `target_link_libraries(mytarget PRIVATE Qt6::Coap)` |
| qmake | `QT += coap` |


## Types

| Member | Description |
| --- | --- |
| `enum class Error` |  |
| `enum class Method` |  |
| `enum class MulticastGroup` |  |
| `enum Port` |  |
| `enum class ResponseCode` |  |
| `enum class SecurityMode` |  |


## Type Documentation

<a id="Error-enum"></a>
### enum class Error

Indicates the error condition found during processing of the request.
| Constant | Description |
| --- | --- |
| `QtCoap::Error::Ok` | No error condition.|
| `QtCoap::Error::HostNotFound` | The remote host name was not found.|
| `QtCoap::Error::AddressInUse` | The address is already in use.|
| `QtCoap::Error::TimeOut` | The response did not arrive in time.|
| `QtCoap::Error::BadRequest` | The request was not recognized.|
| `QtCoap::Error::Unauthorized` | The client is not authorized to perform the requested action.|
| `QtCoap::Error::BadOption` | The request could not be understood by the server due to one or more unrecognized or malformed options.|
| `QtCoap::Error::Forbidden` | Access to this resource is forbidden.|
| `QtCoap::Error::NotFound` | The resource requested was not found.|
| `QtCoap::Error::MethodNotAllowed` | The server does not allow the method used for the URL requested.|
| `QtCoap::Error::NotAcceptable` | No resource satisfying the request's acceptance criteria was found.|
| `QtCoap::Error::RequestEntityIncomplete` | The server has not received the blocks of the request body that it needs to proceed. The client has not sent all blocks, has not sent them in the order required by the server, or sent them long enough ago that the server has already discarded them.|
| `QtCoap::Error::PreconditionFailed` | One or more conditions given in the request header fields evaluated to false when tested on the server.|
| `QtCoap::Error::RequestEntityTooLarge` | The request payload is larger than the server is willing or able to process.|
| `QtCoap::Error::UnsupportedContentFormat` | The payload is in a format not supported by this method on the target resource.|
| `QtCoap::Error::InternalServerFault` | The server encountered an unexpected condition that prevented it from fulfilling the request.|
| `QtCoap::Error::NotImplemented` | The server does not support the functionality required to fulfill the request.|
| `QtCoap::Error::BadGateway` | An error occurred with an upstream server.|
| `QtCoap::Error::ServiceUnavailable` | The service is currently unavailable.|
| `QtCoap::Error::GatewayTimeout` | The server, while acting as a gateway or proxy, did not receive a timely response from an upstream server it needed to access in order to complete the request.|
| `QtCoap::Error::ProxyingNotSupported` | The server is unable or unwilling to act as a forward-proxy for the URI specified in the Proxy-Uri Option or using the scheme specified in Proxy-Scheme.|
| `QtCoap::Error::Unknown` | An unknown error occurred.|

<a id="Method-enum"></a>
### enum class Method

This enum specifies CoAP request methods.
| Constant | Description |
| --- | --- |
| `QtCoap::Method::Invalid` | The default request method for an empty request.|
| `QtCoap::Method::Get` | GET method.|
| `QtCoap::Method::Post` | POST method.|
| `QtCoap::Method::Put` | PUT method.|
| `QtCoap::Method::Delete` | DELETE method.|
| `QtCoap::Method::Other` | Other request method.|

<a id="MulticastGroup-enum"></a>
### enum class MulticastGroup

This enum represents CoAP multicast group addresses defined in [RFC 7252](https://tools.ietf.org/html/rfc7252#section-12.8).
| Constant | Description |
| --- | --- |
| `QtCoap::MulticastGroup::AllCoapNodesIPv4` | IPv4 "All CoAP Nodes" address 224.0.1.187, from the "IPv4 Multicast Address Space Registry".|
| `QtCoap::MulticastGroup::AllCoapNodesIPv6LinkLocal` | IPv6 "All CoAP Nodes" link-local scoped address FF02::FD, from the "IPv6 Multicast Address Space Registry".|
| `QtCoap::MulticastGroup::AllCoapNodesIPv6SiteLocal` | IPv6 "All CoAP Nodes" site-local scoped address FF05::FD, from the "IPv6 Multicast Address Space Registry".|

<a id="Port-enum"></a>
### enum Port

This enum specifies the default CoAP port values.
| Constant | Description |
| --- | --- |
| `QtCoap::DefaultPort` | The default port used for the non-secure transmission.|
| `QtCoap::DefaultSecurePort` | The default port used for the secure transmission.|

<a id="ResponseCode-enum"></a>
### enum class ResponseCode

This enum represents the response code from the CoAP protocol, as defined in [RFC 7252](https://tools.ietf.org/html/rfc7252#section-5.9) and [RFC 7959](https://tools.ietf.org/html/rfc7959#section-2.9).
| Constant | Description |
| --- | --- |
| `QtCoap::ResponseCode::EmptyMessage` | The response code for an empty message.|
| `QtCoap::ResponseCode::Created` | The request was successful and has resulted in new resources being created. This response code corresponds to HTTP 201 "Created".|
| `QtCoap::ResponseCode::Deleted` | The resource has been successfully deleted. This response code corresponds to HTTP 204 "No Content" but is only used in response to requests that cause the resource to cease being available, such as DELETE and, in certain circumstances, POST.|
| `QtCoap::ResponseCode::Valid` | This response code is related to HTTP 304 "Not Modified" but is only used to indicate that the response identified by the entity-tag given in the ETag Option is valid.|
| `QtCoap::ResponseCode::Changed` | The resource has been successfully modified. This response code corresponds to HTTP 204 "No Content" but is only used in response to POST and PUT requests.|
| `QtCoap::ResponseCode::Content` | The request has succeeded. This response code corresponds to HTTP 200 "OK" but is only used in response to GET requests.|
| `QtCoap::ResponseCode::Continue` | Indicates that the transfer of the current block was successful, and the server will send more blocks.|
| `QtCoap::ResponseCode::BadRequest` | The request was not recognized. This response code corresponds to HTTP 400 "Bad Request".|
| `QtCoap::ResponseCode::Unauthorized` | The client is not authorized to perform the requested action. This response code corresponds to HTTP 401 "Unauthorized".|
| `QtCoap::ResponseCode::BadOption` | The request could not be understood by the server due to unrecognized options.|
| `QtCoap::ResponseCode::Forbidden` | Access to this resource is forbidden. This response code corresponds to HTTP 403 "Forbidden".|
| `QtCoap::ResponseCode::NotFound` | The resource requested was not found. This response code corresponds to HTTP 404 "Not Found".|
| `QtCoap::ResponseCode::MethodNotAllowed` | The server does not allow the method used for the URL requested. This response code corresponds to HTTP 405 "Method Not Allowed" but with no parallel to the "Allow" header field.|
| `QtCoap::ResponseCode::NotAcceptable` | No resource satisfying the request's acceptance criteria was found. This response code corresponds to HTTP 406 "Not Acceptable", but with no response entity.|
| `QtCoap::ResponseCode::RequestEntityIncomplete` | The server has not received all blocks, of the request body, that it needs to proceed.|
| `QtCoap::ResponseCode::PreconditionFailed` | Preconditions given in the request header fields evaluated to false when tested on the server. This response code corresponds to HTTP 412 "Precondition Failed".|
| `QtCoap::ResponseCode::RequestEntityTooLarge` | The request payload is larger than the server is willing or able to process. This response code corresponds to HTTP 413 "Request Entity Too Large".|
| `QtCoap::ResponseCode::UnsupportedContentFormat` | The payload is in a format not supported by this method on the target resource. This response code corresponds to HTTP 415 "Unsupported Media Type".|
| `QtCoap::ResponseCode::InternalServerFault` | The server encountered an unexpected condition that prevented it from fulfilling the request. This response code corresponds to HTTP 500 "Internal Server Error".|
| `QtCoap::ResponseCode::NotImplemented` | The server does not support the functionality required to fulfill the request. This response code corresponds to HTTP 501 "Not Implemented".|
| `QtCoap::ResponseCode::BadGateway` | An error occurred with an upstream server. This response code corresponds to HTTP 502 "Bad Gateway".|
| `QtCoap::ResponseCode::ServiceUnavailable` | The service is currently unavailable. This response code corresponds to HTTP 503 "Service Unavailable".|
| `QtCoap::ResponseCode::GatewayTimeout` | The server, while acting as a gateway or proxy, did not receive a timely response from an upstream server. This response code corresponds to HTTP 504 "Gateway Timeout".|
| `QtCoap::ResponseCode::ProxyingNotSupported` | The server is unable or unwilling to act as a forward-proxy for the URI specified in the Proxy-Uri Option or using the scheme specified in Proxy-Scheme.|
| `QtCoap::ResponseCode::InvalidCode` | An invalid response code.|

<a id="SecurityMode-enum"></a>
### enum class SecurityMode

Specifies the security mode used for securing a CoAP connection, as defined in [RFC 7252](https://tools.ietf.org/html/rfc7252#section-9).
| Constant | Description |
| --- | --- |
| `QtCoap::SecurityMode::NoSecurity` | There is no protocol-level security (DTLS is disabled).|
| `QtCoap::SecurityMode::PreSharedKey` | DTLS is enabled. PSK authentication will be used for security.|
| `QtCoap::SecurityMode::RawPublicKey` | DTLS is enabled. An asymmetric key pair without a certificate (a raw public key) will be used for security. This mode is not supported yet.|
| `QtCoap::SecurityMode::Certificate` | DTLS is enabled. An asymmetric key pair with an X.509 certificate will be used for security.|


---

*Built with QDoc's template engine.*
