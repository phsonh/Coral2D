#pragma once
#include "Unit.h"
#include "UnitHandle.h"
#include <cstdint>
#include <vector>

namespace core::object::Unit {

	class UnitPool {
	public:
		explicit UnitPool(std::uint32_t capacity);

		// 创建一个 Unit，池满时返回无效 Handle。
		[[nodiscard]] UnitHandle create();

		// 销毁指定 Unit。
		// Handle 无效或已过期时返回 false。
		[[nodiscard]] bool destroy(UnitHandle handle) noexcept;

		// 根据 Handle 获取 Unit。
		// Handle 无效或已过期时返回 nullptr。
		[[nodiscard]] Unit* get(UnitHandle handle) noexcept;
		[[nodiscard]] Unit const* get(UnitHandle handle) const noexcept;

		// 对象池容量。
		[[nodiscard]] std::uint32_t capacity() const noexcept;

	private:
		struct Slot {
			Unit unit{};

			// 当前槽位的代数。
			std::uint64_t generation = 1;

			// 空闲时，指向下一个空闲槽位。
			std::uint32_t next_free = UnitHandle::invalid_index;

			// 当前槽位是否正在使用。
			bool active = false;
		};

		std::vector<Slot> slots_;

		// 空闲链表的第一个槽位。
		std::uint32_t free_head_ = UnitHandle::invalid_index;
	};

}