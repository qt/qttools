// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

// External types for QTBUG-140636 test cases
// Lupdate doesn't process this header (angle bracket include without include paths)

#ifndef EXTERNAL_TYPES_H
#define EXTERNAL_TYPES_H

namespace Tasking {
    struct GroupItem {};
}

namespace External {
    namespace Deeply {
        namespace Nested {
            struct Type {};
        }
    }
}

namespace UndefinedNS {
    struct RetType {};
}

namespace ThirdParty {
    struct Item {};
}

namespace LibraryNS {
    struct Data {};
}

namespace OperatorNS {
    struct Type {};
}

#endif // EXTERNAL_TYPES_H
