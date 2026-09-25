#include "core/objects/Unit/UnitPool.h"

#include <cstdlib>
#include <iostream>
#include <utility>
#include <vector>

namespace UnitObject = core::object::Unit;

namespace {

	#define CHECK(expression) \
		do { \
			if (!(expression)) { \
				std::cerr << "CHECK failed: " #expression \
					<< " at line " << __LINE__ << '\n'; \
				std::exit(EXIT_FAILURE); \
			} \
		} while (false)

	void test_zero_capacity() {
		UnitObject::UnitPool pool(0);
		CHECK(pool.empty());
		CHECK(pool.full());
		CHECK(pool.capacity() == 0);
		CHECK(!pool.create().is_valid());
	}

	void test_create_get_and_capacity() {
		UnitObject::UnitPool pool(3);
		const auto first = pool.create();
		const auto second = pool.create();
		const auto third = pool.create();

		CHECK(first.is_valid());
		CHECK(second.is_valid());
		CHECK(third.is_valid());
		CHECK(pool.size() == 3);
		CHECK(pool.full());
		CHECK(!pool.create().is_valid());

		pool.get(first)->x = 10.0;
		pool.get(second)->x = 20.0;
		pool.get(third)->x = 30.0;
		CHECK(pool.units().size() == 3);
		CHECK(pool.units()[1].x == 20.0);
		CHECK(pool.handle_at(1) == second);
		CHECK(!pool.handle_at(3).is_valid());
	}

	void test_swap_remove_keeps_handles_valid() {
		UnitObject::UnitPool pool(3);
		const auto first = pool.create();
		const auto middle = pool.create();
		const auto last = pool.create();
		pool.get(first)->x = 1.0;
		pool.get(middle)->x = 2.0;
		pool.get(last)->x = 3.0;

		CHECK(pool.destroy(middle));
		CHECK(pool.size() == 2);
		CHECK(pool.get(middle) == nullptr);
		CHECK(pool.get(first)->x == 1.0);
		CHECK(pool.get(last)->x == 3.0);
		CHECK(pool.handle_at(1) == last);
		CHECK(!pool.destroy(middle));

		const auto replacement = pool.create();
		CHECK(replacement.index == middle.index);
		CHECK(replacement.generation == middle.generation + 1);
		CHECK(pool.get(replacement)->x == 0.0);
		CHECK(pool.get(middle) == nullptr);
	}

	void test_clear_invalidates_only_live_handles() {
		UnitObject::UnitPool pool(4);
		const auto first = pool.create();
		const auto second = pool.create();
		CHECK(pool.destroy(first));

		pool.clear();
		CHECK(pool.empty());
		CHECK(!pool.full());
		CHECK(!pool.contains(first));
		CHECK(!pool.contains(second));

		for (std::uint32_t i = 0; i < pool.capacity(); ++i) {
			CHECK(pool.create().is_valid());
		}
		CHECK(pool.full());
	}

	void test_const_access() {
		UnitObject::UnitPool pool(1);
		const auto handle = pool.create();
		pool.get(handle)->timer = 42;

		const auto& const_pool = pool;
		CHECK(const_pool.get(handle)->timer == 42);
		CHECK(const_pool.units().front().timer == 42);
		CHECK(const_pool.contains(handle));
	}

	void test_repeated_churn_preserves_all_mappings() {
		UnitObject::UnitPool pool(32);
		std::vector<std::pair<UnitObject::UnitHandle, std::int32_t>> live;
		std::uint32_t random = 0x12345678U;
		std::int32_t next_value = 1;

		for (std::uint32_t operation = 0; operation < 10'000; ++operation) {
			random ^= random << 13;
			random ^= random >> 17;
			random ^= random << 5;

			if (live.empty() || (!pool.full() && (random & 1U) != 0)) {
				const auto handle = pool.create();
				CHECK(handle.is_valid());
				pool.get(handle)->timer = next_value;
				live.emplace_back(handle, next_value++);
			}
			else {
				const std::size_t victim = random % live.size();
				const auto stale_handle = live[victim].first;
				CHECK(pool.destroy(stale_handle));
				CHECK(pool.get(stale_handle) == nullptr);
				live.erase(live.begin() + victim);
			}

			CHECK(pool.size() == live.size());
			for (const auto& [handle, value] : live) {
				CHECK(pool.get(handle) != nullptr);
				CHECK(pool.get(handle)->timer == value);
			}
			for (std::uint32_t dense_index = 0; dense_index < pool.size(); ++dense_index) {
				const auto handle = pool.handle_at(dense_index);
				CHECK(pool.get(handle) == &pool.units()[dense_index]);
			}
		}
	}
}

int main() {
	test_zero_capacity();
	test_create_get_and_capacity();
	test_swap_remove_keeps_handles_valid();
	test_clear_invalidates_only_live_handles();
	test_const_access();
	test_repeated_churn_preserves_all_mappings();
	std::cout << "UnitPool tests passed\n";
}
