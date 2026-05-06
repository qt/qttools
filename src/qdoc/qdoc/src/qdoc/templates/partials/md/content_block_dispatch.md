{#-
Copyright (C) 2026 The Qt Company Ltd.
SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
-#}
{#-
Renders one content block. Expects a variable named `block` to be in scope.

Callers iterate over a block list and include this partial per block, with
the section branch handled in the caller (iterates section.children and
includes this partial per child) so the partial does not need to recurse
into itself. Inja resolves includes at parse time; a self-including partial
triggers parser recursion.
-#}
{% if block.type == "paragraph" %}
{% for i in block.inlines %}{% if i.type == "text" %}{{ i.text }}{% else if i.type == "code" %}`{{ i.text }}`{% else if i.type == "bold" %}**{% for c in i.children %}{{ c.text }}{% endfor %}**{% else if i.type == "italic" %}_{% for c in i.children %}{{ c.text }}{% endfor %}_{% else if i.type == "link" %}{% if existsIn(i, "href") %}[{% for c in i.children %}{{ c.text }}{% endfor %}]({{ i.href }}){% else %}{% for c in i.children %}{{ c.text }}{% endfor %}{% endif %}{% else if i.type == "teletype" %}`{% for c in i.children %}{{ c.text }}{% endfor %}`{% else if i.type == "underline" %}<u>{% for c in i.children %}{{ c.text }}{% endfor %}</u>{% else if i.type == "strikethrough" %}~~{% for c in i.children %}{{ c.text }}{% endfor %}~~{% else if i.type == "subscript" %}<sub>{% for c in i.children %}{{ c.text }}{% endfor %}</sub>{% else if i.type == "superscript" %}<sup>{% for c in i.children %}{{ c.text }}{% endfor %}</sup>{% else if i.type == "parameter" %}_{% for c in i.children %}{{ c.text }}{% endfor %}_{% else if i.type == "line-break" %}
{% else if i.type == "image" %}{% if existsIn(i, "href") %}![{% if existsIn(i, "title") %}{{ i.title }}{% endif %}]({{ i.href }}){% endif %}{% else if i.type == "keyword" %}{% else if i.type == "target" %}{% else %}{{ i.text }}{% endif %}{% endfor %}

{% else if block.type == "code-block" %}
```{{ block.attributes.language }}
{{ block.text }}
```

{% else if block.type == "section-heading" %}
{% if block.attributes.level == 1 %}# {% else if block.attributes.level == 2 %}## {% else if block.attributes.level == 3 %}### {% else if block.attributes.level == 4 %}#### {% else if block.attributes.level == 5 %}##### {% else if block.attributes.level == 6 %}###### {% endif %}{{ block.text }}

{% else if block.type == "list" %}
{% for item in block.children %}
- {% if existsIn(item, "children") %}{% for p in item.children %}{% if p.type == "paragraph" %}{% for i in p.inlines %}{% if i.type == "text" %}{{ i.text }}{% else if i.type == "code" %}`{{ i.text }}`{% else if i.type == "bold" %}**{% for c in i.children %}{% if c.type == "code" %}`{{ c.text }}`{% else %}{{ c.text }}{% endif %}{% endfor %}**{% else if i.type == "italic" %}_{% for c in i.children %}{% if c.type == "code" %}`{{ c.text }}`{% else %}{{ c.text }}{% endif %}{% endfor %}_{% else if i.type == "link" %}{% if existsIn(i, "href") %}[{% for c in i.children %}{{ c.text }}{% endfor %}]({{ i.href }}){% else %}{% for c in i.children %}{{ c.text }}{% endfor %}{% endif %}{% else if i.type == "teletype" %}`{% for c in i.children %}{{ c.text }}{% endfor %}`{% else %}{{ i.text }}{% endif %}{% endfor %}{% endif %}{% endfor %}{% else if existsIn(item, "inlines") %}{% for i in item.inlines %}{% if i.type == "text" %}{{ i.text }}{% else if i.type == "code" %}`{{ i.text }}`{% else if i.type == "bold" %}**{% for c in i.children %}{{ c.text }}{% endfor %}**{% else if i.type == "italic" %}_{% for c in i.children %}{{ c.text }}{% endfor %}_{% else if i.type == "link" %}{% if existsIn(i, "href") %}[{% for c in i.children %}{{ c.text }}{% endfor %}]({{ i.href }}){% else %}{% for c in i.children %}{{ c.text }}{% endfor %}{% endif %}{% else if i.type == "teletype" %}`{% for c in i.children %}{{ c.text }}{% endfor %}`{% else %}{{ i.text }}{% endif %}{% endfor %}{% else %}{{ item.text }}{% endif %}{{ "" }}
{% endfor %}

{% else if block.type == "definition-list" %}
{% if block.attributes.listType == "value" %}
| Constant | Description |
| --- | --- |
{% for entry in block.children %}{% if entry.type == "definition-term" %}| `{% if existsIn(entry, "children") and length(entry.children) > 0 %}{% for p in entry.children %}{% if p.type == "paragraph" %}{% for i in p.inlines %}{{ i.text }}{% endfor %}{% endif %}{% endfor %}{% else if existsIn(entry, "inlines") %}{% for i in entry.inlines %}{{ i.text }}{% endfor %}{% else %}{{ entry.text }}{% endif %}` | {% else if entry.type == "definition-description" %}{% if existsIn(entry, "children") and length(entry.children) > 0 %}{% for p in entry.children %}{% if p.type == "paragraph" %}{% for i in p.inlines %}{{ i.text }}{% endfor %}{% endif %}{% endfor %}{% else if existsIn(entry, "inlines") %}{% for i in entry.inlines %}{{ i.text }}{% endfor %}{% endif %} |
{% endif %}{% endfor %}
{% else %}
{% for entry in block.children %}
{% if entry.type == "definition-term" %}**{% if existsIn(entry, "children") and length(entry.children) > 0 %}{% for p in entry.children %}{% if p.type == "paragraph" %}{% for i in p.inlines %}{{ i.text }}{% endfor %}{% endif %}{% endfor %}{% else if existsIn(entry, "inlines") %}{% for i in entry.inlines %}{{ i.text }}{% endfor %}{% else %}{{ entry.text }}{% endif %}**
{% else if entry.type == "definition-description" %}:   {% if existsIn(entry, "children") and length(entry.children) > 0 %}{% for p in entry.children %}{% if p.type == "paragraph" %}{% for i in p.inlines %}{{ i.text }}{% endfor %}{% endif %}{% endfor %}{% else if existsIn(entry, "inlines") %}{% for i in entry.inlines %}{{ i.text }}{% endfor %}{% endif %}
{% endif %}{% endfor %}
{% endif %}

{% else if block.type == "note" %}
> **Note:** {{ block.text }}

{% else if block.type == "warning" %}
> **Warning:** {{ block.text }}

{% else if block.type == "important" %}
> **Important:** {{ block.text }}

{% else if block.type == "horizontal-rule" %}
---

{% else if block.type == "div" %}
{{ block.text }}

{% else if block.type == "table" %}
{#- CommonMark tables don't support colspan/rowspan; cells render without spanning -#}
{% for row in block.rows %}
| {% for cell in row.cells %}{% if cell.type == "table-cell" %}{{ cell.text }} | {% endif %}{% endfor %}

{% if row.type == "table-header-row" %}|{% for cell in row.cells %}{% if cell.type == "table-cell" %} --- |{% endif %}{% endfor %}

{% endif %}{% endfor %}
{% else if block.type == "section" %}
{#- Reaching this branch is an invariant violation: the caller is expected
    to intercept section blocks and iterate their children into the
    dispatch. The loud marker is intentional — a silent text fallback would
    let nested-section misconfiguration ship. -#}
> **UNEXPECTED SECTION BLOCK:** callers must handle section blocks before invoking the dispatch partial. The partial does not recurse, so nested sections cannot render here. Pipeline bug.

{% else if block.type == "list-placeholder" %}
> **UNEXPANDED LIST PLACEHOLDER:** {% if existsIn(block.attributes, "argument") %}{{ block.attributes.argument }}{% else %}<unknown>{% endif %} (variant: {% if existsIn(block.attributes, "variant") %}{{ block.attributes.variant }}{% else %}<unknown>{% endif %}). The list-expander pass did not run. Pipeline bug.

{% else if block.type == "catalog" %}
{#- Catalog children use a different vocabulary (entry-shape with direct
    inlines, no paragraph wrappers) and the catalog branches in current
    generators only ever produce section-heading, table, and list children.
    Rendering is inlined here rather than recursing into the dispatch. A
    new block type added under catalog must be handled separately in this
    branch. -#}
{% for child in block.children %}
{% if child.type == "section-heading" %}
{% if child.attributes.level == 1 %}# {% else if child.attributes.level == 2 %}## {% else if child.attributes.level == 3 %}### {% else if child.attributes.level == 4 %}#### {% else if child.attributes.level == 5 %}##### {% else if child.attributes.level == 6 %}###### {% endif %}{% for i in child.inlines %}{% if i.type == "text" %}{{ i.text }}{% endif %}{% endfor %}

{% else if child.type == "table" %}
{{ "\n" }}
| Name | Description |
| --- | --- |
{% for row in child.rows %}{% if row.type == "table-row" %}| {% for cell in row.cells %}{% if cell.type == "table-cell" %}{% if not loop.is_first %}{{ " " }}| {% endif %}{% for i in cell.inlines %}{% if i.type == "link" %}{% if existsIn(i, "href") %}[{% for c in i.children %}{{ escape_md_table(c.text) }}{% endfor %}]({{ i.href }}){% else %}{% for c in i.children %}{{ escape_md_table(c.text) }}{% endfor %}{% endif %}{% else if i.type == "text" %}{{ escape_md_table(i.text) }}{% else %}{{ escape_md_table(i.text) }}{% endif %}{% endfor %}{% endif %}{% endfor %}{{ " " }}|
{% endif %}{% endfor %}

{% else if child.type == "list" %}
{% for item in child.children %}
- {% for i in item.inlines %}{% if i.type == "link" %}{% if existsIn(i, "href") %}[{% for c in i.children %}{{ c.text }}{% endfor %}]({{ i.href }}){% else %}{% for c in i.children %}{{ c.text }}{% endfor %}{% endif %}{% else if i.type == "text" %}{{ i.text }}{% else %}{{ i.text }}{% endif %}{% endfor %}{{ "" }}
{% endfor %}

{% endif %}
{% endfor %}

{% else %}
{{ block.text }}

{% endif %}
