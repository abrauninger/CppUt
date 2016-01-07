#include "CppUt.h"
#include <memory>

namespace CppUt {
namespace Details {

UnitTestMetadata& GlobalMetadata()
{
	static UnitTestMetadata s_metadata;
	return s_metadata;
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
} // namespace CppUt

// TODO: Remove
void PrintCurrentCallstack()
{
	CppUt::Details::PrintCallstack(CppUt::Details::GetCurrentStackFrames());
}