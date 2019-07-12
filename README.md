# CppUt
Lightweight C++ unit test framework, with syntax similar to the Microsoft Unit Testing Framework for C++.

## Usage

```C++
#include <CppUt.h>

TEST_CLASS(Tests)
{
  TEST_METHOD(DeathStarIsOperational)
  {
    TestAssert::IsTrue(DeathStar::IsOperational());
  }
};
```

### Why CppUt, and not Google Test, Boost.Test, etc.?

No good reason.

I wrote CppUt several years ago as a coding exercise, and I've ended up using it in most of my projects.

I've been meaning to replace CppUt with a more well-maintained unit-test framework, but I haven't gotten around to it yet.
