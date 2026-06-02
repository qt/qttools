**Contents**

- [Using the Module](#using-the-module)
- [Building with CMake](#building-with-cmake)
- [Building with qmake](#building-with-qmake)
- [Articles and Guides](#articles-and-guides)
- [Examples](#examples)
- [Reference](#reference)
- [Licenses and Attributions](#licenses-and-attributions)

# Qt CoAP

Provides classes and functions to make CoAP programming simple and portable.

Constrained Application Protocol ([CoAP](https://coap.technology/)) is a machine-to-machine (M2M) web transfer protocol for use with constrained nodes and constrained networks in the Internet of Things (IoT). It is designed to easily interface with HTTP for integration with the Web, while meeting specialized requirements such as multicast support, very low overhead, and simplicity for constrained environments.
The Qt CoAP module implements the client side of CoAP defined by [RFC 7252](https://datatracker.ietf.org/doc/html/rfc7252). Generally, CoAP is designed to use datagram-oriented transport such as UDP, so the current implementation of the transport is based on UDP. However implementing custom transports based on TCP, WebSocket, and so on, is also possible.
The Qt CoAP module supports:
- Security based on Datagram TLS (DTLS) over UDP
- Group communication defined by [RFC 7390](https://datatracker.ietf.org/doc/html/rfc7390)
- Blockwise transfers defined by [RFC 7959](https://datatracker.ietf.org/doc/html/rfc7959)
- Resource observation defined by [RFC 7641](https://datatracker.ietf.org/doc/html/rfc7641)
- Resource discovery (multicast and single server)

## Using the Module

Using a Qt module requires linking against the module library, either directly or through other dependencies. Several build tools have dedicated support for this, including [CMake](https://doc.qt.io/qt-6/cmake-manual.html) and [qmake](qtcoaptest-quicksecureclient-example.md#qmake).

### Building with CMake

Use the `find_package()` command to locate the needed module components in the `Qt6` package:
```cpp
find_package(Qt6 REQUIRED COMPONENTS Coap)
target_link_libraries(mytarget PRIVATE Qt6::Coap)

```

See also the [Build with CMake](https://doc.qt.io/qt-6/cmake-qt5-and-qt6-compatibility.html) overview.

### Building with qmake

To configure the module for building with qmake, add the module as a value of the `QT` variable in the project's .pro file:
```cpp
QT += coap

```


## Articles and Guides

- [Overview](qtcoap-overview.md)


## Examples

- [Qt CoAP Examples](qtcoap-examples.md)


## Reference

- [C++ Classes](qtcoap-module.md)


## Licenses and Attributions

Qt CoAP is available under commercial licenses from [The Qt Company](https://www.qt.io/). In addition, it is available under the [GNU General Public License, version 3](https://www.gnu.org/licenses/gpl-3.0.html).



---

*Built with QDoc's template engine.*
