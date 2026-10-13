{#-
Copyright (C) 2026 The Qt Company Ltd.
SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

Fixture-local page template. It renders the title and the brief, and nothing
else: keeping the template local means the expected output does not follow
QDoc's default page template.
-#}
# {{ fullTitle }}

{% if default(brief, "") != "" %}
{{ brief }}
{% endif %}
