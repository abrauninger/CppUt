#include "CppUt.h"
#include <cstdio>


static UnitTestMetadata s_unitTests;


TEST_CLASS(MyTestClass2, s_unitTests)
{
public:
	TEST_METHOD(MyTestMethod)
	{
		TestAssert::IsTrue(true);
	}

	TEST_METHOD(MySecondTestMethod)
	{
		TestAssert::IsTrue(false);
	}
};


int main()
{
	CppUt::RunUnitTests(s_unitTests);
}