#include "UnitPool.h"

#include <cstddef>
#include <limits>
#include <utility>

namespace core::object::Unit {

	UnitPool::UnitPool(std::uint32_t capacity)
		: slots_(capacity)
	{
		// 后续 create() 不再分配内存，避免游戏过程中出现分配抖动。
		units_.reserve(capacity);
		dense_to_slot_.reserve(capacity);

		if (capacity == 0) {
			return;
		}

		free_head_ = 0;

		for (std::uint32_t index = 0; index < capacity; ++index) {
			Slot& slot = slots_[index];
			slot.generation = 1;
			slot.index = index + 1 < capacity
				? index + 1
				: UnitHandle::invalid_index;
		}
	}

	UnitHandle UnitPool::create() {
		if (free_head_ == UnitHandle::invalid_index) {
			return UnitHandle{};
		}

		const std::uint32_t slot_index = free_head_;
		Slot& slot = slots_[slot_index];
		free_head_ = slot.index;

		const auto dense_index = static_cast<std::uint32_t>(units_.size());
		units_.emplace_back();
		dense_to_slot_.push_back(slot_index);
		slot.index = dense_index;
		slot.active = true;

		return UnitHandle{ slot_index, slot.generation };
	}

	bool UnitPool::destroy(UnitHandle handle) noexcept {
		if (!matches(handle)) {
			return false;
		}

		Slot& removed_slot = slots_[handle.index];
		const std::uint32_t removed_dense_index = removed_slot.index;
		const std::uint32_t last_dense_index =
			static_cast<std::uint32_t>(units_.size() - 1);

		// 用尾元素填洞，让所有活跃 Unit 始终保持连续。
		if (removed_dense_index != last_dense_index) {
			units_[removed_dense_index] = std::move(units_.back());

			const std::uint32_t moved_slot_index = dense_to_slot_.back();
			dense_to_slot_[removed_dense_index] = moved_slot_index;
			slots_[moved_slot_index].index = removed_dense_index;
		}

		units_.pop_back();
		dense_to_slot_.pop_back();
		removed_slot.active = false;

		// 不允许 generation 回绕；耗尽的槽位永久退役。
		if (removed_slot.generation == std::numeric_limits<std::uint64_t>::max()) {
			removed_slot.index = UnitHandle::invalid_index;
			return true;
		}

		++removed_slot.generation;
		removed_slot.index = free_head_;
		free_head_ = handle.index;

		return true;
	}

	Unit* UnitPool::get(UnitHandle handle) noexcept {
		return matches(handle) ? &units_[slots_[handle.index].index] : nullptr;
	}

	Unit const* UnitPool::get(UnitHandle handle) const noexcept {
		return matches(handle) ? &units_[slots_[handle.index].index] : nullptr;
	}

	bool UnitPool::contains(UnitHandle handle) const noexcept {
		return matches(handle);
	}

	void UnitPool::clear() noexcept {
		// 只访问活跃槽位，复杂度为 O(size)，而不是 O(capacity)。
		for (const std::uint32_t slot_index : dense_to_slot_) {
			Slot& slot = slots_[slot_index];
			slot.active = false;
			if (slot.generation == std::numeric_limits<std::uint64_t>::max()) {
				slot.index = UnitHandle::invalid_index;
				continue;
			}

			++slot.generation;
			slot.index = free_head_;
			free_head_ = slot_index;
		}

		units_.clear();
		dense_to_slot_.clear();
	}

	std::span<Unit> UnitPool::units() noexcept {
		return units_;
	}

	std::span<Unit const> UnitPool::units() const noexcept {
		return units_;
	}

	UnitHandle UnitPool::handle_at(std::uint32_t dense_index) const noexcept {
		if (dense_index >= dense_to_slot_.size()) {
			return UnitHandle{};
		}

		const std::uint32_t slot_index = dense_to_slot_[dense_index];
		return UnitHandle{ slot_index, slots_[slot_index].generation };
	}

	std::uint32_t UnitPool::size() const noexcept {
		return static_cast<std::uint32_t>(units_.size());
	}

	std::uint32_t UnitPool::capacity() const noexcept {
		return static_cast<std::uint32_t>(slots_.size());
	}

	bool UnitPool::empty() const noexcept {
		return units_.empty();
	}

	bool UnitPool::full() const noexcept {
		return free_head_ == UnitHandle::invalid_index;
	}

	bool UnitPool::matches(UnitHandle handle) const noexcept {
		if (!handle.is_valid() || handle.index >= slots_.size()) {
			return false;
		}

		const Slot& slot = slots_[handle.index];
		return slot.generation == handle.generation
			&& slot.active;
	}
}
