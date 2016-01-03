#include <cstdint>		// TODO: Needed?
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


class TestClassMetadata
{
public:
	TestClassMetadata(const wchar_t* className) :
		m_className(className)
	{
	}

	const wchar_t* ClassName() const
	{
		return m_className;
	}

	const TestClassMetadata* NextClass = nullptr;
	const TestMethodMetadata* HeadMethod = nullptr;

private:
	const wchar_t* m_className;
};


static const TestClassMetadata* s_pHeadClass = nullptr;


class TestMethodMetadataAdder
{
public:
	TestMethodMetadataAdder(const TestClassMetadata *pClass, const TestMethodMetadata* pMethod)
	{
		const_cast<TestMethodMetadata*>(pMethod)->NextMethod = pClass->HeadMethod;
		const_cast<TestClassMetadata*>(pClass)->HeadMethod = pMethod;
	}
};


class TestClassMetadataAdder
{
public:
	TestClassMetadataAdder(const TestClassMetadata* pClass)
	{
		const_cast<TestClassMetadata*>(pClass)->NextClass = s_pHeadClass;
		s_pHeadClass = pClass;
	}
};


#define TEST_METHOD(methodName) \
	const TestMethodMetadata c_methodMetadata_##methodName { L#methodName, &methodName }; \
	TestMethodMetadataAdder m_methodAdder_##methodName { _MyTestClassMetadata(), &c_methodMetadata_##methodName }; \
	static void methodName() \


#define TEST_CLASS(className) \
	class className; \
	\
	/* ClassInstantiator is necessary in order for any of the test methods in the test class to register themselves. */ \
	static ClassInstantiator<className> s_classInstance_##className; \
	\
	\
	/* The base class is necessary so that the TEST_METHOD macro can refer to '_MyTestClassMetadata'. */ \
	class className##_Base \
	{ \
	protected: \
		const TestClassMetadata* _MyTestClassMetadata() \
		{ \
			return &c_classMetadata_##className; \
		} \
		\
	private: \
		const TestClassMetadata c_classMetadata_##className { L#className }; \
		TestClassMetadataAdder m_classAdder_##className { _MyTestClassMetadata() }; \
	}; \
	\
	class className : className##_Base \


TEST_CLASS(MyTestClass2)
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
	wprintf(L"RUNNING UNIT TESTS\n\n");

	const TestClassMetadata* pCurrentClass = s_pHeadClass;
	while (pCurrentClass != nullptr)
	{
		wprintf(L"Executing test methods in test class '%s'\n", pCurrentClass->ClassName());

		const TestMethodMetadata* pCurrentMethod = pCurrentClass->HeadMethod;
		while (pCurrentMethod != nullptr)
		{
			wprintf(L"Executing test method '%s'\n", pCurrentMethod->MethodName());
			pCurrentMethod->MethodFunction()();
			pCurrentMethod = pCurrentMethod->NextMethod;
		}

		pCurrentClass = pCurrentClass->NextClass;
	}
}