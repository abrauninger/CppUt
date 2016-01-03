#pragma once

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


class UnitTestMetadata
{
public:
	const TestClassMetadata* HeadClass = nullptr;
};


class TestMethodMetadataAdder
{
public:
	TestMethodMetadataAdder(const TestClassMetadata& cl, const TestMethodMetadata& method)
	{
		const_cast<TestMethodMetadata&>(method).NextMethod = cl.HeadMethod;
		const_cast<TestClassMetadata&>(cl).HeadMethod = &method;
	}
};


class TestClassMetadataAdder
{
public:
	TestClassMetadataAdder(const UnitTestMetadata& unitTests, const TestClassMetadata& cl)
	{
		const_cast<TestClassMetadata&>(cl).NextClass = unitTests.HeadClass;
		const_cast<UnitTestMetadata&>(unitTests).HeadClass = &cl;
	}
};


#define TEST_METHOD(methodName) \
	const TestMethodMetadata c_methodMetadata_##methodName { L#methodName, &methodName }; \
	TestMethodMetadataAdder m_methodAdder_##methodName { _MyTestClassMetadata(), c_methodMetadata_##methodName }; \
	static void methodName() \


#define TEST_CLASS(className, unitTestMetadata) \
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
		const TestClassMetadata& _MyTestClassMetadata() \
		{ \
			return c_classMetadata_##className; \
		} \
		\
	private: \
		const TestClassMetadata c_classMetadata_##className { L#className }; \
		TestClassMetadataAdder m_classAdder_##className { unitTestMetadata, _MyTestClassMetadata() }; \
	}; \
	\
	class className : className##_Base \


namespace CppUt {

inline void PrintTestClassName(const TestClassMetadata& testClass)
{
	wprintf(L"\n%s\n", testClass.ClassName());
}

inline void PrintTestName(const TestMethodMetadata& testMethod)
{
	wprintf(L"  %s", testMethod.MethodName());
}

inline void PrintSuccess(const TestMethodMetadata& /*testMethod*/)
{
	wprintf(L"....SUCCEEDED\n");
}

inline void PrintFailure(const TestMethodMetadata& /*testMethod*/)
{
	wprintf(L"....FAILED\n");
}

__declspec(thread) bool s_testSuccess = true;

inline void RunTestMethod(const TestMethodMetadata& testMethod)
{
	PrintTestName(testMethod);

	s_testSuccess = true;

	testMethod.MethodFunction()();

	if (s_testSuccess)
		PrintSuccess(testMethod);
	else
		PrintFailure(testMethod);
}

inline void RunUnitTests(const UnitTestMetadata& unitTests)
{
	const TestClassMetadata* pCurrentClass = unitTests.HeadClass;
	while (pCurrentClass != nullptr)
	{
		PrintTestClassName(*pCurrentClass);

		const TestMethodMetadata* pCurrentMethod = pCurrentClass->HeadMethod;
		while (pCurrentMethod != nullptr)
		{
			RunTestMethod(*pCurrentMethod);
			pCurrentMethod = pCurrentMethod->NextMethod;
		}

		pCurrentClass = pCurrentClass->NextClass;
	}
}

}

struct TestAssert
{
	static void IsTrue(bool condition)
	{
		if (!condition)
			CppUt::s_testSuccess = false;
	}
};