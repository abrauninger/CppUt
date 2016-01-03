#include "CppUt.h"
#include <cstdio>


static UnitTestMetadata s_unitTests;


TEST_CLASS(MyTestClass2, s_unitTests)
{
public:
	TEST_METHOD(MyTestMethod)
	{
		wprintf(L"Executing MyTestMethod.\n");
	}

	TEST_METHOD(MySecondTestMethod)
	{
		wprintf(L"And now executing MySecondTestMethod.\n");
	}
};


int main()
{
	CppUt::RunUnitTests(s_unitTests);
}