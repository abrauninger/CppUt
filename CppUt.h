#pragma once

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <SafeInt.h>
#include <StrSafe.h>
#include <tuple>
#include <utility>
#include <vector>
#include <Windows.h>

// DbgHelp.h needs to be #included after other stuff (Windows.h, etc.)
#pragma warning(push)
#pragma warning(disable: 4091)
#include <DbgHelp.h>
#pragma warning(pop)

#undef min

using namespace msl::utilities;

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
		const TestClassMetadata& _MyTestClassMetadata() \
		{ \
			return c_classMetadata_##className; \
		} \
		\
	private: \
		const TestClassMetadata c_classMetadata_##className { L#className }; \
		TestClassMetadataAdder m_classAdder_##className { ::CppUt::Details::GlobalMetadata(), _MyTestClassMetadata() }; \
	}; \
	\
	class className : className##_Base \


namespace CppUt {

namespace Details {

struct TestFailure
{
	std::wstring Message;
	std::vector<void*> StackFrames;
};

UnitTestMetadata& GlobalMetadata();
std::vector<void*> GetCurrentStackFrames();
void AddFailure(TestFailure&& failure);

} // namespace CppUt::Details

void RunUnitTests();

}

class TestAssert
{
public:
	static void Fail(const wchar_t* message)
	{
		CppUt::Details::TestFailure failure;
		failure.Message = message;
		failure.StackFrames = CppUt::Details::GetCurrentStackFrames();

		CppUt::Details::AddFailure(std::move(failure));
	}

	static void IsTrue(bool condition, const wchar_t* failureMessage)
	{
		if (!condition)
			Fail(failureMessage);
	}

	static void AreEqual(uint32_t value1, uint32_t value2)
	{
		AreEqualImpl<uint32_t>(value1, value2);
	}

	static void AreEqual(int32_t value1, int32_t value2)
	{
		AreEqualImpl<int32_t>(value1, value2);
	}

	static void AreEqual(int32_t value1, uint32_t value2)
	{
		AreEqualImpl<int32_t>(value1, SafeInt<int32_t>(value2));
	}

	static void AreEqual(int32_t value1, size_t value2)
	{
		AreEqualImpl<size_t>(SafeInt<size_t>(value1), value2);
	}

	static void AreEqual(size_t value1, size_t value2)
	{
		AreEqualImpl<size_t>(value1, value2);
	}

	static void AreEqual(const wchar_t* value1, const wchar_t* value2)
	{
		IsTrue(wcscmp(value1, value2) == 0, L"AreEqual failed");
	}

private:
	template <class T>
	static void AreEqualImpl(const T& value1, const T& value2)
	{
		IsTrue(value1 == value2, L"AreEqual failed");
	}
};