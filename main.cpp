#include "CppUt.h"
#include <cstdio>


static UnitTestMetadata s_unitTests;


TEST_CLASS(MyTestClass2, s_unitTests)
{
public:
	TEST_METHOD(MyTestMethod)
	{
		TestAssert::IsTrue(true, L"Expected to be true.");
	}

	TEST_METHOD(MySecondTestMethod)
	{
		TestAssert::IsTrue(false, L"Expected to be true.");
		TestAssert::IsTrue(false, L"Another failure here.");
	}

	TEST_METHOD(AThirdTestMethod)
	{
		TestAssert::Fail(L"Doomed from the start!");
	}
};


int main()
{
	CppUt::RunUnitTests(s_unitTests);
}