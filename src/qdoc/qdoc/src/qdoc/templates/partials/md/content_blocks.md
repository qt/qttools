{#-
Copyright (C) 2025 The Qt Company Ltd.
SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
-#}
{% if length(content.blocks) > 0 %}
{% for block in content.blocks %}
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
- {% for p in item.children %}{% if p.type == "paragraph" %}{% for i in p.inlines %}{% if i.type == "text" %}{{ i.text }}{% else if i.type == "code" %}`{{ i.text }}`{% else if i.type == "bold" %}**{% for c in i.children %}{% if c.type == "code" %}`{{ c.text }}`{% else %}{{ c.text }}{% endif %}{% endfor %}**{% else if i.type == "italic" %}_{% for c in i.children %}{% if c.type == "code" %}`{{ c.text }}`{% else %}{{ c.text }}{% endif %}{% endfor %}_{% else if i.type == "link" %}{% if existsIn(i, "href") %}[{% for c in i.children %}{{ c.text }}{% endfor %}]({{ i.href }}){% else %}{% for c in i.children %}{{ c.text }}{% endfor %}{% endif %}{% else if i.type == "teletype" %}`{% for c in i.children %}{{ c.text }}{% endfor %}`{% else %}{{ i.text }}{% endif %}{% endfor %}{% endif %}{% endfor %}{{ "" }}
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
{% for child in block.children %}
{% if child.type == "section-heading" %}
{% if child.attributes.level == 1 %}# {% else if child.attributes.level == 2 %}## {% else if child.attributes.level == 3 %}### {% else if child.attributes.level == 4 %}#### {% endif %}{{ child.text }}

{% else if child.type == "paragraph" %}
{% for i in child.inlines %}{% if i.type == "text" %}{{ i.text }}{% else if i.type == "code" %}`{{ i.text }}`{% else if i.type == "bold" %}**{% for c in i.children %}{{ c.text }}{% endfor %}**{% else if i.type == "italic" %}_{% for c in i.children %}{{ c.text }}{% endfor %}_{% else if i.type == "link" %}{% if existsIn(i, "href") %}[{% for c in i.children %}{{ c.text }}{% endfor %}]({{ i.href }}){% else %}{% for c in i.children %}{{ c.text }}{% endfor %}{% endif %}{% else if i.type == "teletype" %}`{% for c in i.children %}{{ c.text }}{% endfor %}`{% else if i.type == "underline" %}<u>{% for c in i.children %}{{ c.text }}{% endfor %}</u>{% else if i.type == "strikethrough" %}~~{% for c in i.children %}{{ c.text }}{% endfor %}~~{% else if i.type == "subscript" %}<sub>{% for c in i.children %}{{ c.text }}{% endfor %}</sub>{% else if i.type == "superscript" %}<sup>{% for c in i.children %}{{ c.text }}{% endfor %}</sup>{% else if i.type == "parameter" %}_{% for c in i.children %}{{ c.text }}{% endfor %}_{% else if i.type == "line-break" %}
{% else if i.type == "image" %}{% if existsIn(i, "href") %}![{% if existsIn(i, "title") %}{{ i.title }}{% endif %}]({{ i.href }}){% endif %}{% else if i.type == "keyword" %}{% else if i.type == "target" %}{% else %}{{ i.text }}{% endif %}{% endfor %}

{% else if child.type == "code-block" %}
```{{ child.attributes.language }}
{{ child.text }}
```

{% else if child.type == "list" %}
{% for item in child.children %}
- {% for p in item.children %}{% if p.type == "paragraph" %}{% for i in p.inlines %}{% if i.type == "text" %}{{ i.text }}{% else if i.type == "code" %}`{{ i.text }}`{% else if i.type == "bold" %}**{% for c in i.children %}{% if c.type == "code" %}`{{ c.text }}`{% else %}{{ c.text }}{% endif %}{% endfor %}**{% else if i.type == "italic" %}_{% for c in i.children %}{% if c.type == "code" %}`{{ c.text }}`{% else %}{{ c.text }}{% endif %}{% endfor %}_{% else if i.type == "link" %}{% if existsIn(i, "href") %}[{% for c in i.children %}{{ c.text }}{% endfor %}]({{ i.href }}){% else %}{% for c in i.children %}{{ c.text }}{% endfor %}{% endif %}{% else if i.type == "teletype" %}`{% for c in i.children %}{{ c.text }}{% endfor %}`{% else %}{{ i.text }}{% endif %}{% endfor %}{% endif %}{% endfor %}{{ "" }}
{% endfor %}

{% else if child.type == "definition-list" %}
{% for entry in child.children %}
{% if entry.type == "definition-term" %}**{{ entry.text }}**
{% else if entry.type == "definition-description" %}:   {{ entry.text }}
{% endif %}{% endfor %}

{% else if child.type == "note" %}
> **Note:** {{ child.text }}

{% else if child.type == "warning" %}
> **Warning:** {{ child.text }}

{% else if child.type == "important" %}
> **Important:** {{ child.text }}

{% else if child.type == "horizontal-rule" %}
---

{% else if child.type == "table" %}
{#- CommonMark tables don't support colspan/rowspan; cells render without spanning -#}
{% for row in child.rows %}
| {% for cell in row.cells %}{% if cell.type == "table-cell" %}{{ cell.text }} | {% endif %}{% endfor %}

{% if row.type == "table-header-row" %}|{% for cell in row.cells %}{% if cell.type == "table-cell" %} --- |{% endif %}{% endfor %}

{% endif %}{% endfor %}

{% else if child.type == "div" %}
{{ child.text }}

{% else %}
{{ child.text }}

{% endif %}
{% endfor %}

{% else %}
{{ block.text }}

{% endif %}
{% endfor %}
{% else if content.text %}
{{ content.text }}

{% endif %}
