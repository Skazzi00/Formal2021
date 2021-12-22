#include "gtest/gtest.h"
#include "earley.hpp"

using namespace formal::earley;

TEST(earleyTest, test1) {
    std::vector<Rule> g = {{'S',"S(S)"}, {'S', ""}};
    ASSERT_TRUE(earley(g, "()"));
    ASSERT_TRUE(earley(g, "a"));
    ASSERT_TRUE(earley(g, "()()"));
    ASSERT_TRUE(earley(g, "()()((()()))"));
    ASSERT_FALSE(earley(g, "())"));
    ASSERT_FALSE(earley(g, ")("));
}