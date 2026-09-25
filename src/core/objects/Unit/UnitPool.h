#pragma once
#include "Unit.h"
#include "UnitHandle.h"
#include <cstdint>
#include <span>
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
		[[nodiscard]] bool contains(UnitHandle handle) const noexcept;

		// 销毁所有 Unit，并使已有 Handle 全部过期。
		void clear() noexcept;

		// 活跃 Unit 紧密存放，可直接用于逐帧顺序更新。
		// destroy()/clear() 会使受移动对象的指针、引用和迭代器失效。
		[[nodiscard]] std::span<Unit> units() noexcept;
		[[nodiscard]] std::span<Unit const> units() const noexcept;

		// 获取稠密数组中指定 Unit 的安全句柄。
		[[nodiscard]] UnitHandle handle_at(std::uint32_t dense_index) const noexcept;

		// 对象池容量。
		[[nodiscard]] std::uint32_t size() const noexcept;
		[[nodiscard]] std::uint32_t capacity() const noexcept;
		[[nodiscard]] bool empty() const noexcept;
		[[nodiscard]] bool full() const noexcept;

	private:
		struct Slot {
			// 当前槽位的代数。
			std::uint64_t generation = 1;

			// 活跃时是稠密数组下标；空闲时是空闲链表的下一项。
			std::uint32_t index = UnitHandle::invalid_index;

			bool active = false;
		};

		// 稀疏槽位只保存 Handle 元数据，热数据 Unit 独立紧密存放。
		std::vector<Slot> slots_;
		std::vector<Unit> units_;
		std::vector<std::uint32_t> dense_to_slot_;

		// 空闲链表的第一个槽位。
		std::uint32_t free_head_ = UnitHandle::invalid_index;

		[[nodiscard]] bool matches(UnitHandle handle) const noexcept;
	};

}
