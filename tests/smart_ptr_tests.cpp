#include "../include/stc/smart_ptr.h"
#include "tests_framework.h"

using namespace stc;

class test_class
{
public:
	int a = 42;
	float b = 3.14f;
};

CREATE_TEST(shared_ptr_test)
{
	// Make
	shared_ptr ptr = make_shared<test_class>();
	CHECK(ptr.use_count() == 1);
	CHECK(ptr.get() != nullptr);
	CHECK(ptr->a == 42);
	CHECK(ptr->b == 3.14f);

	// Copy
	shared_ptr ptr2 = ptr;
	CHECK(ptr.use_count() == 2);
	CHECK(ptr2.use_count() == 2);
	CHECK(ptr.get() == ptr2.get());

	// Move
	shared_ptr ptr3 = std::move(ptr2);
	CHECK(ptr.use_count() == 2);
	CHECK(ptr3.use_count() == 2);
	CHECK(ptr2.use_count() == 0);
	CHECK(ptr2.get() == nullptr);

	// Reset
	ptr.reset();
	CHECK(ptr.use_count() == 0);
	CHECK(ptr.get() == nullptr);
	CHECK(ptr3.use_count() == 1);

	// Final reset
	ptr3.reset();
	CHECK(ptr3.use_count() == 0);
	CHECK(ptr3.get() == nullptr);
}

CREATE_TEST(weak_ptr_test)
{
	// Make
	shared_ptr ptr = make_shared<test_class>();
	weak_ptr weak = ptr;
	CHECK(weak.use_count() == 1);
	CHECK(!weak.expired());

	// Copy
	weak_ptr weak2 = weak;
	CHECK(weak.use_count() == 1);
	CHECK(weak2.use_count() == 1);

	// Move
	weak_ptr weak3 = std::move(weak2);
	CHECK(weak2.use_count() == 0);
	CHECK(weak2.expired());
	CHECK(weak3.use_count() == 1);
	CHECK(!weak3.expired());

	// Lock
	shared_ptr ptr2 = weak3.lock();
	CHECK(ptr2.use_count() == 2);
	CHECK(ptr2.get());
	ptr2.reset();

	// Reset
	ptr.reset();
	CHECK(weak.use_count() == 0);
	CHECK(weak.expired());
	CHECK(weak3.expired());

	// Lock after reset
	shared_ptr ptr3 = weak.lock();
	CHECK(ptr3.use_count() == 0);
	CHECK(!ptr3.get());
}
