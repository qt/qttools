// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <catch/catch.hpp>

#include <qdoc/templategenerator.h>
#include <qdoc/injabridge.h>
#include <qdoc/ir/member.h>

#include <QDir>
#include <QFile>
#include <QJsonArray>

#include <variant>

using namespace Qt::StringLiterals;

// NOTE: TemplateGenerator is a skeleton implementation that extends Generator.
// Full integration testing requires the complete QDoc infrastructure (Config,
// QDocDatabase, Node hierarchy, etc.). These tests verify basic behavior and
// signal intent for future comprehensive testing once the IR system is in place.

SCENARIO("TemplateGenerator compilation", "[TemplateGenerator][Generator][Compilation]") {

    GIVEN("The TemplateGenerator class") {
        WHEN("The header is included") {
            THEN("It compiles successfully") {
                REQUIRE(true);
            }
        }
    }
}

SCENARIO("TemplateGenerator format identification", "[TemplateGenerator][Generator]") {

    GIVEN("A TemplateGenerator instance") {
        // Note: FileResolver requires complex setup, so we verify that
        // TemplateGenerator can be instantiated and reports correct format
        // metadata without full infrastructure
        WHEN("Querying the output format") {
            THEN("It reports 'template' as its format identifier") {
                // TODO: Replace placeholder with real instantiation once a FileResolver
                // factory or test double is available:
                //   TemplateGenerator generator(fileResolver);
                //   REQUIRE(generator.format() == "template");
                INFO("TemplateGenerator::format() should return \"template\"");
                REQUIRE(true);  // Placeholder
            }
        }

        WHEN("Querying the file extension") {
            THEN("It reports 'html' as its default extension") {
                // TODO: Replace placeholder with real instantiation once a FileResolver
                // factory or test double is available:
                //   TemplateGenerator generator(fileResolver);
                //   REQUIRE(generator.fileExtension() == "html");
                INFO("TemplateGenerator::fileExtension() should return \"html\"");
                REQUIRE(true);  // Placeholder
            }
        }
    }
}

TEST_CASE("Shared status sentences render once after all group headings",
          "[TemplateGenerator][deprecation]")
{
    const QString format = GENERATE(QStringLiteral("html"), QStringLiteral("md"));
    const bool isPlural = GENERATE(false, true);
    const QString version = GENERATE(QString(), QStringLiteral("6.5"));

    IR::SharedDetailGroup group;
    group.firstIndex = 0;
    group.count = 2;
    group.status = Status::Deprecated;
    group.isPlural = isPlural;
    group.typeNoun = isPlural ? "methods"_L1 : "property group"_L1;
    group.deprecatedSince = version;
    if (!version.isEmpty())
        group.deprecatedSinceQualified = "GroupQml "_L1 + version;
    IR::ContentBlock body;
    body.type = IR::BlockType::Paragraph;
    IR::InlineContent text;
    text.type = IR::InlineType::Text;
    text.text = "Shared detail."_L1;
    body.inlineContent.append(text);
    group.body.append(body);

    IR::MemberIR member;
    member.anchorId = "first"_L1;
    member.synopsis = "first()"_L1;
    member.status = Status::Deprecated;
    member.typeNoun = "method"_L1;
    member.sharedGroup = 0;
    IR::SectionIR section;
    section.members.append(member);
    member.anchorId = "second"_L1;
    member.synopsis = "second()"_L1;
    section.members.append(member);
    section.sharedGroups.append(group);

    const InjaBridge::IncludeCallback include = [](const QString &name) -> std::optional<QString> {
        QFile file(QDir(QString::fromUtf8(QDOC_TEMPLATE_DIR)).filePath(name));
        if (!file.open(QIODevice::ReadOnly))
            return std::nullopt;
        return QString::fromUtf8(file.readAll());
    };
    const QString source = "{% for section in sections %}{% for member in section.members %}"
                           "{% include \"partials/"_L1 + format + "/member_detail."_L1 + format
            + "\" %}{% endfor %}{% endfor %}"_L1;
    const auto result = InjaBridge::render(
            source, QJsonObject{ { "sections"_L1, QJsonArray{ section.toJson() } } }, include);
    REQUIRE(std::holds_alternative<QString>(result));
    const auto &output = std::get<QString>(result);
    const QString subject = isPlural ? "These methods are"_L1 : "This property group is"_L1;
    const QString since = version.isEmpty() ? QString() : " since GroupQml "_L1 + version;
    const QString sentence = subject + " deprecated"_L1 + since
            + ". We strongly advise against using "_L1 + (isPlural ? "them"_L1 : "it"_L1)
            + " in new code."_L1;

    CAPTURE(format, isPlural, version);
    CHECK(output.count("first()"_L1) == 1);
    CHECK(output.count("second()"_L1) == 1);
    CHECK(output.count("Shared detail."_L1) == 1);
    CHECK(output.count(sentence) == 1);
    CHECK_FALSE(output.contains("This method is deprecated"_L1));
    CHECK(output.indexOf("first()"_L1) < output.indexOf("second()"_L1));
    CHECK(output.indexOf("second()"_L1) < output.indexOf(sentence));
    CHECK(output.indexOf(sentence) < output.indexOf("Shared detail."_L1));
    if (format == "md"_L1)
        CHECK(output.contains(sentence + "\n\nShared detail."_L1));
}
