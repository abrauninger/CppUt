#include <CppUt/CppUt.h>
#include <cstdio>


TEST_CLASS(MyTestClass2)
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
	CppUt::RunUnitTests();
}