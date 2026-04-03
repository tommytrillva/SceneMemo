#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <cmath>

struct TestCase {
    std::string name;
    std::function<bool()> func;
};

inline std::vector<TestCase>& getTests() {
    static std::vector<TestCase> tests;
    return tests;
}

struct TestRegistrar {
    TestRegistrar(const char* name, std::function<bool()> func) {
        getTests().push_back({name, func});
    }
};

#define TEST(name) \
    static bool test_##name(); \
    static TestRegistrar reg_##name(#name, test_##name); \
    static bool test_##name()

#define EXPECT(cond) do { if (!(cond)) { std::cerr << "  FAIL: " << #cond << " at " << __FILE__ << ":" << __LINE__ << "\n"; return false; } } while(0)
#define EXPECT_NEAR(a, b, eps) do { if (std::abs((a) - (b)) > (eps)) { std::cerr << "  FAIL: " << #a << "=" << (a) << " != " << #b << "=" << (b) << " at " << __FILE__ << ":" << __LINE__ << "\n"; return false; } } while(0)
