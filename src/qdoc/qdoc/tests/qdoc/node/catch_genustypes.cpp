// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <catch/catch.hpp>
#include "qdoc/genustypes.h"

SCENARIO("Testing common bits between Genus types", "[Genus]") {
    GIVEN("Various Genus type combinations") {

        WHEN("Comparing a Genus type with itself") {
            THEN("Same type: Genus::DOC with itself should share common bits") {
                REQUIRE(hasCommonGenusType(Genus::DOC, Genus::DOC) == true);
            }

            THEN("Genus::CPP with itself should share common bits") {
                REQUIRE(hasCommonGenusType(Genus::CPP, Genus::CPP) == true);
            }

            THEN("Genus::QML with itself should share common bits") {
                REQUIRE(hasCommonGenusType(Genus::QML, Genus::QML) == true);
            }

            THEN("Genus::API with itself should share common bits") {
                REQUIRE(hasCommonGenusType(Genus::API, Genus::API) == true);
            }

            THEN("Genus::DontCare with itself should not share common bits") {
                REQUIRE(hasCommonGenusType(Genus::DontCare, Genus::DontCare) == false);
            }
        }

        WHEN("Comparing types that should have common bits") {
            THEN("Genus::CPP and Genus::API should share common bits") {
                REQUIRE(hasCommonGenusType(Genus::CPP, Genus::API) == true);
                REQUIRE(hasCommonGenusType(Genus::API, Genus::CPP) == true);
            }

            THEN("Genus::QML and Genus::API should share common bits") {
                REQUIRE(hasCommonGenusType(Genus::QML, Genus::API) == true);
                REQUIRE(hasCommonGenusType(Genus::API, Genus::QML) == true);
            }

            THEN("Genus::CPP and Genus::QML should not share common bits") {
                REQUIRE(hasCommonGenusType(Genus::CPP, Genus::QML) == false);
                REQUIRE(hasCommonGenusType(Genus::QML, Genus::CPP) == false);
            }
        }

        WHEN("Comparing types that should not have common bits") {
            THEN("Genus::DOC and Genus::CPP should not share common bits") {
                REQUIRE(hasCommonGenusType(Genus::DOC, Genus::CPP) == false);
                REQUIRE(hasCommonGenusType(Genus::CPP, Genus::DOC) == false);
            }

            THEN("Genus::DOC and Genus::QML should not share common bits") {
                REQUIRE(hasCommonGenusType(Genus::DOC, Genus::QML) == false);
                REQUIRE(hasCommonGenusType(Genus::QML, Genus::DOC) == false);
            }

            THEN("Genus::DOC and Genus::API should not share common bits") {
                REQUIRE(hasCommonGenusType(Genus::DOC, Genus::API) == false);
                REQUIRE(hasCommonGenusType(Genus::API, Genus::DOC) == false);
            }

            THEN("Genus::DontCare should not share bits with any non-zero type") {
                REQUIRE(hasCommonGenusType(Genus::DontCare, Genus::CPP) == false);
                REQUIRE(hasCommonGenusType(Genus::CPP, Genus::DontCare) == false);

                REQUIRE(hasCommonGenusType(Genus::DontCare, Genus::QML) == false);
                REQUIRE(hasCommonGenusType(Genus::QML, Genus::DontCare) == false);

                REQUIRE(hasCommonGenusType(Genus::DontCare, Genus::DOC) == false);
                REQUIRE(hasCommonGenusType(Genus::DOC, Genus::DontCare) == false);

                REQUIRE(hasCommonGenusType(Genus::DontCare, Genus::API) == false);
                REQUIRE(hasCommonGenusType(Genus::API, Genus::DontCare) == false);
            }
        }
    }
}

SCENARIO("Checking if Genus types are API types", "[Genus]") {
    GIVEN("Different types of Genus") {

        WHEN("Checking types that should be API types") {
            THEN("Genus::CPP should be recognized as an API type") {
                REQUIRE(isApiGenus(Genus::CPP) == true);
            }

            THEN("Genus::QML should be recognized as an API type") {
                REQUIRE(isApiGenus(Genus::QML) == true);
            }

            THEN("Genus::API should be recognized as an API type") {
                REQUIRE(isApiGenus(Genus::API) == true);
            }
        }

        WHEN("Checking types that should not be API types") {
            THEN("Genus::DOC should not be recognized as an API type") {
                REQUIRE(isApiGenus(Genus::DOC) == false);
            }

            THEN("Genus::DontCare should not be recognized as an API type") {
                REQUIRE(isApiGenus(Genus::DontCare) == false);
            }
        }

        WHEN("Verifying API genus consistency") {
            THEN("isApiGenus() behavior should be consistent with hasCommonGenusType() with Genus::API") {
                REQUIRE(isApiGenus(Genus::CPP) == hasCommonGenusType(Genus::CPP, Genus::API));
                REQUIRE(isApiGenus(Genus::QML) == hasCommonGenusType(Genus::QML, Genus::API));
                REQUIRE(isApiGenus(Genus::API) == hasCommonGenusType(Genus::API, Genus::API));
                REQUIRE(isApiGenus(Genus::DOC) == hasCommonGenusType(Genus::DOC, Genus::API));
                REQUIRE(isApiGenus(Genus::DontCare) == hasCommonGenusType(Genus::DontCare, Genus::API));
            }
        }
    }
}

SCENARIO("Verifying Genus enumeration values", "[Genus]") {
    GIVEN("The Genus enumeration") {

        WHEN("Examining individual enumeration values") {
            THEN("Genus::CPP should have the value 0x1") {
                REQUIRE(static_cast<unsigned char>(Genus::CPP) == 0x1);
            }

            THEN("Genus::QML should have the value 0x4") {
                REQUIRE(static_cast<unsigned char>(Genus::QML) == 0x4);
            }

            THEN("Genus::DOC should have the value 0x8") {
                REQUIRE(static_cast<unsigned char>(Genus::DOC) == 0x8);
            }

            THEN("Genus::DontCare should have the value 0x0") {
                REQUIRE(static_cast<unsigned char>(Genus::DontCare) == 0x0);
            }
        }

        WHEN("Examining compound enumeration values") {
            THEN("Genus::API should be a combination of CPP and QML with value 0x5") {
                REQUIRE(static_cast<unsigned char>(Genus::API) == 0x5);
                REQUIRE(static_cast<unsigned char>(Genus::API) ==
                        (static_cast<unsigned char>(Genus::CPP) | static_cast<unsigned char>(Genus::QML)));
            }
        }
    }
}
