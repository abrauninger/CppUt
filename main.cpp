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

	const TestMethodMetadata* NextMethod = nullptr;

private:
	const wchar_t* m_methodName;
	TestMethodType m_methodFunction;
};


static const TestMethodMetadata* s_pHeadMethod = nullptr;


class TestMethodMetadataAdder
{
public:
	TestMethodMetadataAdder(const TestMethodMetadata* pMetadata)
	{
		const_cast<TestMethodMetadata*>(pMetadata)->NextMethod = s_pHeadMethod;
		s_pHeadMethod = pMetadata;
	}
};


#define TEST_METHOD(methodName) \
	const TestMethodMetadata m_metadata_##methodName { L"methodName", &methodName }; \
	TestMethodMetadataAdder m_adder_##methodName { &m_metadata_##methodName }; \
	static void methodName()


template <class T>
struct ClassInstantiator
{
	ClassInstantiator()
	{
		// Using the 'new' operator is a convenient way to ensure that the type 'T' is instantiated by the compiler,
		// meaning that the code for type 'T' is emitted.
		// Note that if we just declared T as a local variable here, MSVC would compile it but it would crash at run-time.
		// I'm not sure why the crash happens; this could be investigated more if we want.
		T* pT = new T();
		(pT);
	}
};

class MyTestClass;

static ClassInstantiator<MyTestClass> s_classInstance_MyTestClass;

class MyTestClass
{
public:
	TEST_METHOD(MyTestMethod)
	{
		printf("Executing MyTestMethod.\n");
	}

	TEST_METHOD(MySecondTestMethod)
	{
		printf("And now executing MySecondTestMethod.\n");
	}
};


int main()
{
	printf("Hello world!\n");

	const TestMethodMetadata* pCurrentMethod = s_pHeadMethod;
	while (pCurrentMethod != nullptr)
	{
		pCurrentMethod->MethodFunction()();
		pCurrentMethod = pCurrentMethod->NextMethod;
	}
}