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

	TestMethodMetadata* NextMethod = nullptr;

private:
	const wchar_t* m_methodName;
	TestMethodType m_methodFunction;
};


static TestMethodMetadata* s_pHeadMethod = nullptr;


class TestMethodMetadataAdder
{
public:
	TestMethodMetadataAdder(TestMethodMetadata* pMetadata)
	{
		pMetadata->NextMethod = s_pHeadMethod;
		s_pHeadMethod = pMetadata;
	}
};


class MyTestClass
{
public:
	TestMethodMetadata m_metadataMyTestMethod { L"MyTestMethod", &MyTestMethod };
	TestMethodMetadataAdder m_adderMyTestMethod { &m_metadataMyTestMethod };

	static void MyTestMethod()
	{
		printf("Executing MyTestMethod.");
	}
};


int main()
{
	printf("Hello world!\n");

	MyTestClass testClass;

	TestMethodMetadata* pCurrentMethod = s_pHeadMethod;
	while (pCurrentMethod != nullptr)
	{
		pCurrentMethod->MethodFunction()();
		pCurrentMethod = pCurrentMethod->NextMethod;
	}
}