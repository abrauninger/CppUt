#include "CppUt.h"
#include <memory>
#include <Windows.h>

// DbgHelp.h needs to be #included after other stuff (Windows.h, etc.)
#pragma warning(push)
#pragma warning(disable: 4091)
#include <DbgHelp.h>
#pragma warning(pop)

#undef min

namespace CppUt {
namespace Details {

UnitTestMetadata& GlobalMetadata()
{
	static UnitTestMetadata s_metadata;
	return s_metadata;
}

constexpr uint8_t ShiftBit(bool bit, uint8_t numberToShift)
{
	return static_cast<uint8_t>(bit) << numberToShift;
}

constexpr uint8_t Color(bool red, bool green, bool blue, bool intense)
{
	return ShiftBit(intense, 3) | ShiftBit(red, 2) | ShiftBit(green, 1) | ShiftBit(blue, 0);
}

enum class ConsoleColor : uint8_t
{
	Red = Color(true, false, false, true),
	Green = Color(false, true, false, true),
	White = Color(true, true, true, true),
	Gray = Color(true, true, true, false),
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

std::pair<uint16_t, uint16_t> ColumnPositionAndWindowWidth()
{
	CONSOLE_SCREEN_BUFFER_INFO bufferInfo;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &bufferInfo);

	return { bufferInfo.dwCursorPosition.X, bufferInfo.dwMaximumWindowSize.X };
}

void PrintTestClassName(const TestClassMetadata& testClass)
{
	Print(L"\n%s\n", testClass.ClassName());
}

void PrintTestName(const TestMethodMetadata& testMethod)
{
	Print(L"  %s", testMethod.MethodName());
}

static const wchar_t* c_succeededString = L"SUCCEEDED";
static const wchar_t* c_failedString = L"FAILED";

void PrintRightAlignedResultWithDots(ConsoleColor color, const wchar_t* resultString, uint16_t rightEdgeColumnWidth = 0)
{
	uint16_t position;
	uint16_t windowWidth;
	std::tie(position, windowWidth) = ColumnPositionAndWindowWidth();

	auto width = rightEdgeColumnWidth > 0 ? std::min(rightEdgeColumnWidth, windowWidth) : windowWidth;

	auto resultStringLength = wcslen(resultString);

	auto dotCount = width - position - resultStringLength - 1;

	if (dotCount < 0)
		dotCount = 0;

	for (uint16_t i = 0; i < dotCount; ++i)
		Print(L".");

	PrintWithColor(color, L"%s\n", resultString);
}

void PrintSuccessResult()
{
	PrintRightAlignedResultWithDots(ConsoleColor::Green, c_succeededString);
}

void PrintFailureResult()
{
	PrintRightAlignedResultWithDots(ConsoleColor::Red, c_failedString);
}

const size_t c_maxStackFrames = 1024;
const size_t c_maxSymbolNameLength = 1024;

std::vector<void*> GetCurrentStackFrames()
{
	std::vector<void*> frames;
	frames.resize(c_maxStackFrames);

	// Skip the current frame since we don't need to see 'GetCurrentStackFrames' on every callstack.
	auto frameCount = CaptureStackBackTrace(1 /*FramesToSkip*/, c_maxStackFrames, frames.data(), NULL /*BackTraceHash*/);

	frames.resize(frameCount);

	return frames;
}

void PrintCallstack(const std::vector<void*>& stackFrames)
{
	auto process = GetCurrentProcess();
	SymInitialize(process, NULL /*UserSearchPath*/, TRUE /*fInvadeProcess*/);
	SymSetOptions(SYMOPT_LOAD_LINES);

	for (auto&& frame : stackFrames)
	{
		auto address = reinterpret_cast<DWORD64>(frame);

		Print(L"    ");

		uint8_t symbolBuffer[sizeof(SYMBOL_INFOW) + c_maxSymbolNameLength * sizeof(wchar_t)];

		PSYMBOL_INFOW pSymbol = reinterpret_cast<PSYMBOL_INFOW>(symbolBuffer);
		pSymbol->MaxNameLen = c_maxSymbolNameLength;
		pSymbol->SizeOfStruct = sizeof(SYMBOL_INFOW);

		SymFromAddrW(process, address, NULL /*Displacement*/, pSymbol);

		IMAGEHLP_LINEW64 line;
		line.SizeOfStruct = sizeof(IMAGEHLP_LINEW64);

		DWORD displacement;

		if (SymGetLineFromAddrW64(process, address, &displacement, &line))
		{
			PrintWithColor(ConsoleColor::Red, L"%s (%s:%lu)", pSymbol->Name, line.FileName, line.LineNumber);
		}
		else
		{
			PrintWithColor(ConsoleColor::Red, L"%s (0x%0X)", pSymbol->Name, pSymbol->Address);
		}

		Print(L"\n");
	}
}

void PrintFailures(const std::vector<TestFailure>& failures)
{
	PrintFailureResult();

	size_t failureNumber = 1;

	for (auto&& failure : failures)
	{
		Print(L"\n  ");

		if (failures.size() > 1)
			PrintWithColor(ConsoleColor::Red, L"%lld of %lld: ", failureNumber, failures.size());

		PrintWithColor(ConsoleColor::Red, L"%s\n", failure.Message.c_str());

		PrintCallstack(failure.StackFrames);

		++failureNumber;
	}

	Print(L"\n");
}

const uint16_t c_maxResultColumnWidth = 20;

void PrintSummaryResultWithColor(const wchar_t* label, ConsoleColor color, uint32_t count)
{
	Print(L"%s", label);

	wchar_t countString[10];
	StringCchPrintfW(countString, _countof(countString), L"%u", count);

	auto actualColor = count > 0 ? color : ConsoleColor::Gray;

	PrintRightAlignedResultWithDots(actualColor, countString, c_maxResultColumnWidth);
}

struct ResultSummary
{
	uint32_t SucceededCount = 0;
	uint32_t FailedCount = 0;
};

void PrintResultSummary(const ResultSummary& summary)
{
	Print(L"\n");

	PrintSummaryResultWithColor(c_succeededString, ConsoleColor::Green, summary.SucceededCount);
	PrintSummaryResultWithColor(c_failedString, ConsoleColor::Red, summary.FailedCount);

	Print(L"\n");
}


__declspec(thread) std::vector<TestFailure> s_currentFailures { };

void RunTestMethod(const TestMethodMetadata& testMethod, ResultSummary& summary)
{
	PrintTestName(testMethod);

	s_currentFailures = { };

	testMethod.MethodFunction()();

	if (s_currentFailures.size() == 0)
	{
		PrintSuccessResult();
		++summary.SucceededCount;
	}
	else
	{
		PrintFailures(s_currentFailures);
		++summary.FailedCount;
	}
}

void AddFailure(TestFailure&& failure)
{
	s_currentFailures.emplace_back(std::move(failure));
}

} // namespace Details


void RunUnitTests()
{
	const auto& unitTests = CppUt::Details::GlobalMetadata();

	Details::ResultSummary summary;

	const TestClassMetadata* pCurrentClass = unitTests.HeadClass;
	while (pCurrentClass != nullptr)
	{
		Details::PrintTestClassName(*pCurrentClass);

		const TestMethodMetadata* pCurrentMethod = pCurrentClass->HeadMethod;
		while (pCurrentMethod != nullptr)
		{
			Details::RunTestMethod(*pCurrentMethod, summary);
			pCurrentMethod = pCurrentMethod->NextMethod;
		}

		pCurrentClass = pCurrentClass->NextClass;
	}

	PrintResultSummary(summary);
}

} // namespace CppUt