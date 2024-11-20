//
// Created by User on 21/11/2024.
//

#include <catch2/catch_test_macros.hpp>

#include "coords.hpp"

using namespace chess;

TEST_CASE( "File test equal", "[coords]" ) {
    REQUIRE(File::FILE_B == FILE_B);
    REQUIRE(File::FILE_C == FILE_C);
    REQUIRE(File::FILE_E == FILE_E);
}

TEST_CASE( "File test letter", "[coords]" ) {
    REQUIRE((char) FILE_A == 'a');
    REQUIRE((char) FILE_D == 'd');
    REQUIRE((char) FILE_F == 'f');
}


TEST_CASE( "File test int", "[coords]" ) {
    REQUIRE((int) FILE_B == 1);
    REQUIRE((int) FILE_D == 3);
    REQUIRE((int) FILE_E == 4);
}

TEST_CASE( "Rank test equal", "[coords]" ) {
    REQUIRE(Rank::RANK_2 == RANK_2);
    REQUIRE(Rank::RANK_5 == RANK_5);
    REQUIRE(Rank::RANK_7 == RANK_7);
}

TEST_CASE( "Rank test letter", "[coords]" ) {
    REQUIRE((char) RANK_1 == '1');
    REQUIRE((char) RANK_6 == '6');
    REQUIRE((char) RANK_8 == '8');
}


TEST_CASE( "Rank test int", "[coords]" ) {
    REQUIRE((int) RANK_3 == 2);
    REQUIRE((int) RANK_4 == 3);
    REQUIRE((int) RANK_6 == 5);
}