// UnitTests.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

#include "Types.h"

#include "gtest\gtest.h"

using namespace MicroServiceDddLib;

class A
{
public:
    A() {}
    ~A() {}
};

class B
{
public:
    B() {}
    ~B() {}
};

class C
{
public:
    C() {}
    ~C() {}
};

TEST(Injector, BasicFunctionality)
{
    auto& inj{ Injector::Instance() };

    auto const pA{ std::make_shared<A>() };
    auto const pB{ std::make_shared<B>() };
    inj.Set(std::make_shared<A>());
    inj.Set(pA);    // Should replace.
    inj.Set(pB);
    EXPECT_EQ(inj.Get<A>(), pA);
    EXPECT_EQ(inj.Get<B>(), pB);
    try
    {
        auto const c{ inj.Get<C>() };
        FAIL() << "Not expected to succeed";
    }
    catch (std::logic_error const&)
    {
        // Expected to be thrown.
    }

    std::shared_ptr<A> pCloneA;
    std::shared_ptr<B> pCloneB;
    std::shared_ptr<C> pCloneC;
    EXPECT_TRUE(inj.TryGet(pCloneA));
    EXPECT_TRUE(inj.TryGet(pCloneB));
    EXPECT_FALSE(inj.TryGet(pCloneC));
    EXPECT_EQ(pCloneA, pA);
    EXPECT_EQ(pCloneB, pB);
    EXPECT_EQ(pCloneC, nullptr);
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
