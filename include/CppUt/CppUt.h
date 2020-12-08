#pragma once

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <SafeInt.h>
#include <string>
#include <StrSafe.h>
#include <tuple>
#include <utility>
#include <vector>

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

struct TestFailureException
{
	std::wstring Message;
	std::vector<void*> StackFrames;
};

UnitTestMetadata& GlobalMetadata();
std::vector<void*> GetCurrentStackFrames();
void BreakInDebugger();

} // namespace CppUt::Details

void RunUnitTests(bool waitForDebugger = false);

}

class TestAssert
{
public:
	static void Fail(const wchar_t* message)
	{
		CppUt::Details::TestFailureException failure;
		failure.Message = message;
		failure.StackFrames = CppUt::Details::GetCurrentStackFrames();

		CppUt::Details::BreakInDebugger();

		throw failure;
	}

	static void IsTrue(bool condition)
	{
		IsTrue(condition, L"Assert failed.");
	}

	static void IsTrue(bool condition, const wchar_t* failureMessage)
	{
		if (!condition)
			Fail(failureMessage);
	}

	static void IsFalse(bool condition)
	{
		IsTrue(!condition);
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

#if INTPTR_MAX == INT64_MAX
	// 64-bit only (since on 32-bit these end up being duplicate declarations of the previous methods)
	static void AreEqual(int32_t value1, size_t value2)
	{
		AreEqualImpl<size_t>(SafeInt<size_t>(value1), value2);
	}

	static void AreEqual(size_t value1, size_t value2)
	{
		AreEqualImpl<size_t>(value1, value2);
	}
#endif

private:
	template <class Char>
	static void AssertCharacterStringsAreEqual(const Char* value1, const Char* value2)
	{
		AreEqual(std::char_traits<Char>::length(value1), std::char_traits<Char>::length(value2));
		IsTrue(std::char_traits<Char>::compare(value1, value2, std::char_traits<Char>::length(value1)) == 0, L"AreEqual failed");
	}

public:
	static void AreEqual(const wchar_t* value1, const wchar_t* value2)
	{
		AssertCharacterStringsAreEqual(value1, value2);
	}

	static void AreEqual(const char32_t* value1, const char32_t* value2)
	{
		AssertCharacterStringsAreEqual(value1, value2);
	}

	static constexpr double c_epsilon = 0.0001;

	static void AreEqual(double value1, double value2)
	{
		IsTrue(value1 >= value2 - c_epsilon && value1 <= value2 + c_epsilon);
	}

	static void AreEqual(int32_t value1, double value2)
	{
		AreEqual(static_cast<double>(value1), value2);
	}

private:
	template <class T>
	static void AreEqualImpl(const T& value1, const T& value2)
	{
		IsTrue(value1 == value2, L"AreEqual failed");
	}
};