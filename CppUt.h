#pragma once

#include <cstdint>
#include <cstdio>
#include <tuple>
#include <utility>
#include <Windows.h>

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

inline constexpr uint8_t ShiftBit(bool bit, uint8_t numberToShift)
{
	return static_cast<uint8_t>(bit) << numberToShift;
}

inline constexpr uint8_t Color(bool red, bool green, bool blue, bool intense)
{
	return ShiftBit(intense, 3) | ShiftBit(red, 2) | ShiftBit(green, 1) | ShiftBit(blue, 0);
}

enum class ConsoleColor : uint8_t
{
	Red = Color(true, false, false, true),
	Green = Color(false, true, false, true),
	White = Color(true, true, true, true),
};

template <class... Args>
void Print(const wchar_t* formatString, Args&&... args)
{
	wprintf(formatString, std::forward<Args>(args)...);
}

template <class... Args>
void PrintWithColor(ConsoleColor color, const wchar_t* formatString, Args&&... args)
{
	auto hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

	CONSOLE_SCREEN_BUFFER_INFO bufferInfo;
	GetConsoleScreenBufferInfo(hStdOut, &bufferInfo);

	SetConsoleTextAttribute(hStdOut, static_cast<uint8_t>(color));

	wprintf(formatString, std::forward<Args>(args)...);

	SetConsoleTextAttribute(hStdOut, bufferInfo.wAttributes);
}

inline std::pair<uint16_t, uint16_t> ColumnPositionAndWindowWidth()
{
	CONSOLE_SCREEN_BUFFER_INFO bufferInfo;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &bufferInfo);

	return { bufferInfo.dwCursorPosition.X, bufferInfo.dwMaximumWindowSize.X };
}

inline void PrintTestClassName(const TestClassMetadata& testClass)
{
	Print(L"\n%s\n", testClass.ClassName());
}

inline void PrintTestName(const TestMethodMetadata& testMethod)
{
	Print(L"  %s", testMethod.MethodName());
}

static const wchar_t* c_succeededString = L"SUCCEEDED";
static const wchar_t* c_failedString = L"FAILED";

inline void PrintSuccessOrFailure(ConsoleColor color, const wchar_t* resultString)
{
	uint16_t position;
	uint16_t width;
	std::tie(position, width) = ColumnPositionAndWindowWidth();

	auto resultStringLength = wcslen(resultString);

	auto dotCount = width - position - resultStringLength - 1;

	if (dotCount < 0)
		dotCount = 0;

	for (uint16_t i = 0; i < dotCount; ++i)
		Print(L".");

	PrintWithColor(color, L"%s\n", resultString);
}

inline void PrintSuccess()
{
	PrintSuccessOrFailure(ConsoleColor::Green, c_succeededString);
}

inline void PrintFailure()
{
	PrintSuccessOrFailure(ConsoleColor::Red, c_failedString);
}

__declspec(thread) bool s_testSuccess = true;

inline void RunTestMethod(const TestMethodMetadata& testMethod)
{
	PrintTestName(testMethod);

	s_testSuccess = true;

	testMethod.MethodFunction()();

	if (s_testSuccess)
		PrintSuccess();
	else
		PrintFailure();
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