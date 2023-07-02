/* Conquer Space
 * Copyright (C) 2021-2023 Conquer Space
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "common/systems/names/namegenerator.h"

#include <gmock/gmock-more-matchers.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <hjson.h>

#include <algorithm>

#include "common/util/random/stdrandom.h"

using cqsp::common::systems::names::NameGenerator;

TEST(NameGeneratorTest, BasicTest) {
    NameGenerator gen;
    cqsp::common::util::StdRandom std_random(31415);
    Hjson::Value val;
    val = Hjson::UnmarshalFromFile("../data/core/data/names/name_gen_test.hjson");
    gen.LoadNameGenerator(val[0]);
    gen.SetRandom(&std_random);

    std::vector<std::string> potential_names {"Aelash", "Ashash", "Aelburn", "Ashburn"};

    for (int i = 0; i < 100; i++) {
        std::string gen_text = gen.Generate("1");
        // Windows doesn't support union regex
#ifndef WIN32
        EXPECT_THAT(gen_text, testing::MatchesRegex("(Ael|Ash)(ash|burn)"));
#else
        using testing::AllOf;
        using testing::AnyOf;
        using testing::EndsWith;
        using testing::StartsWith;
        EXPECT_THAT(gen_text,
                    AllOf(AnyOf(StartsWith("Ael"), StartsWith("Ash")), AnyOf(EndsWith("ash"), EndsWith("burn"))));
#endif
    }
}

TEST(NameGeneratorTest, IncorrectFormatTest) {
    // Get a text with the incorrect format
    NameGenerator gen;
    cqsp::common::util::StdRandom std_random(31415);
    Hjson::Value val;
    val = Hjson::UnmarshalFromFile("../data/core/data/names/name_gen_test.hjson");
    gen.LoadNameGenerator(val[0]);
    gen.SetRandom(&std_random);

    std::vector<std::string> potential_names {"Aelash", "Ashash", "Aelburn", "Ashburn"};
    std::string gen_text = gen.Generate("wrong");
    ASSERT_EQ(gen_text, "");
}
