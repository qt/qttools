{#-
Copyright (C) 2026 The Qt Company Ltd.
SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

Fixture-local page template. It renders the title and the brief, and nothing
else: the point is a small, obviously correct artefact that proves the run
kept going after the module page was skipped. Keeping the template here rather
than relying on QDoc's built-in page.md means this fixture's expected output
doesn't shift every time the default template changes.
-#}
# {{ fullTitle }}

{% if default(brief, "") != "" %}
{{ brief }}
{% endif %}
