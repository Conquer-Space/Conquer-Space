/* Conquer Space
* Copyright (C) 2021 Conquer Space
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
#include <gtest/gtest.h>

#include <hjson.h>
#include <algorithm>

#include "common/systems/names/namegenerator.h"
#include "common/util/random/stdrandom.h"

using cqsp::common::systems::names::NameGenerator;

TEST(NameGeneratorTest, Test) {
    NameGenerator gen;
    cqsp::common::util::StdRandom std_random(31415926535);
    Hjson::Value val;
    val = Hjson::UnmarshalFromFile("../data/core/data/names/name_gen_test.hjson");
    gen.LoadNameGenerator(val);
    gen.SetRandom(&std_random);
    
    std::vector<std::string> potential_names{"Aelash", "Ashash", "Aelburn", "Ashburn"};

    for (int i = 0; i < 100; i++) {
        std::string gen_text = gen.Generate("1");
        if (std::find(potential_names.begin(), potential_names.end(), gen_text) == potential_names.end()) {
            ASSERT_EQ(gen_text, "nope");
        }
    }
    
}
