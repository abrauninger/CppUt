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


struct TestAllocator
{
	template <class T, class... Args>
	static T* New(Args&&... args)
	{
		return new T(std::forward<Args>(args)...);
	}
};

class BaseTestClass
{

};

class MyTestClass;

class TestClassMetadata
{
public:
	TestClassMetadata()
	{
		BaseTestClass* pInstance = ClassInstance();
		(pInstance);
	}

	static BaseTestClass* ClassInstance()
	{
		// We use a C-style cast instead of static_cast, because static_cast produces a compiler error
		// because the compiler doesn't know for sure that MyTestClass derives from BaseTestClass.
		// TODO: How does the C-style cast work here?  Is it equivalent to a static_cast or reinterpret_cast?  Can we write it without a C-style cast?
		
		// Note: We would not be able to use the 'new' operator directly, because the compiler would complain that
		// MyTestClass does not have a constructor (due to the fact that it's only been forward-declared at this point).
		// By using a templated function instead, we can work around it.
		// TODO: Understand more about why that is.
		return (BaseTestClass*) TestAllocator::New<MyTestClass>();
	}
};

static TestClassMetadata s_foo;

class MyTestClass : BaseTestClass
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