#include <cstdio>

using TestMethodType = void (*)();

class TestMethodMetadata
{
public:
	TestMethodMetadata(const wchar_t* methodName, TestMethodType methodFunction) :
		m_methodName(methodName),
		m_methodFunction(methodFunction)
	{
	}

	const wchar_t* MethodName() const
	{
		return m_methodName;
	}

	TestMethodType MethodFunction() const
	{
		return m_methodFunction;
	}

private:
	const wchar_t* m_methodName;
	TestMethodType m_methodFunction;
};


class MyTestClass
{
public:
	TestMethodMetadata m_metadataMyTestMethod { L"MyTestMethod", &MyTestMethod };

	static void MyTestMethod()
	{
		printf("Executing MyTestMethod.");
	}
};


int main()
{
	printf("Hello world!\n");

	MyTestClass testClass;
	testClass.m_metadataMyTestMethod.MethodFunction()();
}