// UnitPool.cpp

#include "UnitPool.h"

#include <cstddef>
#include <limits>

namespace core::object::Unit {

	UnitPool::UnitPool(std::uint32_t capacity)
		: slots_(capacity)
	{
		// capacity == 0 时，没有任何空闲槽位。
		if (capacity == 0) {
			return;
		}

		// 第一个空闲槽位是 0。
		free_head_ = 0;

		// 把所有 Slot 串成：
		//
		// 0 -> 1 -> 2 -> 3 -> ... -> invalid
		for (std::uint32_t index = 0; index < capacity; ++index) {
			Slot& slot = slots_[index];

			slot.unit.reset();
			slot.generation = 1;
			slot.active = false;

			if (index + 1 < capacity) {
				slot.next_free = index + 1;
			}
			else {
				slot.next_free = UnitHandle::invalid_index;
			}
		}
	}


	UnitHandle UnitPool::create() {
		// free_head_ 无效，说明没有空闲 Slot。
		if (free_head_ == UnitHandle::invalid_index) {
			return UnitHandle{};
		}

		// 从空闲链表头取出一个 Slot。
		const std::uint32_t index = free_head_;
		Slot& slot = slots_[index];

		// 空闲链表头移动到下一个 Slot。
		free_head_ = slot.next_free;

		// 初始化即将使用的 Slot。
		slot.unit.reset();
		slot.active = true;

		// 这个 Slot 已经不属于空闲链表。
		slot.next_free = UnitHandle::invalid_index;

		// 返回当前 Slot 对应的安全句柄。
		return UnitHandle{
			index,
			slot.generation,
		};
	}


	bool UnitPool::destroy(UnitHandle handle) noexcept {
		// get() 会同时检查：
		// 1. index 是否合法
		// 2. Slot 是否 active
		// 3. generation 是否匹配
		if (get(handle) == nullptr) {
			return false;
		}

		const std::uint32_t index = handle.index;
		Slot& slot = slots_[index];

		// 当前 Unit 已经被销毁。
		slot.active = false;

		// generation 到达最大值后，不再复用这个 Slot。
		// 避免 generation 回绕后极旧 Handle 再次匹配。
		if (slot.generation ==
			std::numeric_limits<std::uint64_t>::max())
		{
			slot.next_free = UnitHandle::invalid_index;
			return true;
		}

		// 进入下一代。
		++slot.generation;

		// 把这个 Slot 插回空闲链表头部。
		slot.next_free = free_head_;
		free_head_ = index;

		return true;
	}


	Unit* UnitPool::get(UnitHandle handle) noexcept {
		// 默认构造出来的无效 Handle。
		if (!handle.is_valid()) {
			return nullptr;
		}

		const std::size_t index =
			static_cast<std::size_t>(handle.index);

		// 防止越界。
		if (index >= slots_.size()) {
			return nullptr;
		}

		Slot& slot = slots_[index];

		// Slot 当前没有对象。
		if (!slot.active) {
			return nullptr;
		}

		// Handle 指向的是这个 Slot 的旧 generation。
		if (slot.generation != handle.generation) {
			return nullptr;
		}

		return &slot.unit;
	}


	Unit const* UnitPool::get(UnitHandle handle) const noexcept {
		if (!handle.is_valid()) {
			return nullptr;
		}

		const std::size_t index =
			static_cast<std::size_t>(handle.index);

		if (index >= slots_.size()) {
			return nullptr;
		}

		Slot const& slot = slots_[index];

		if (!slot.active) {
			return nullptr;
		}

		if (slot.generation != handle.generation) {
			return nullptr;
		}

		return &slot.unit;
	}


	std::uint32_t UnitPool::capacity() const noexcept {
		return static_cast<std::uint32_t>(slots_.size());
	}

}