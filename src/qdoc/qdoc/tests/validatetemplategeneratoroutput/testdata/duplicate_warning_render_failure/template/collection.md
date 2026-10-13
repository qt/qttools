{#-
Copyright (C) 2026 The Qt Company Ltd.
SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

Regression-test template: deliberately accesses a field that does not exist
in the IR, so the collection page fails to render in this format while the
other formats complete.
-#}
# {{ fullTitle }}

{% if hasCollection and collection.isModule %}
**Requires:** `{{ collection.definitelyMissingField }}`
{% endif %}
