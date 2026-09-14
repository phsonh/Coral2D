#pragma once
#include <cstdint>
#include <limits>

namespace core::object::Unit {

	// UnitPool 槽位的安全句柄
	struct UnitHandle {
		// 非法值定为uint32最大值
		static constexpr std::uint32_t invalid_index = std::numeric_limits<std::uint32_t>::max();

		std::uint32_t index = invalid_index;   // 槽位索引
		std::uint64_t generation = 0;          // 槽位代数
		
		// 判断句柄是否具有有效的槽位索引
		[[nodiscard]] bool is_valid() const noexcept;

		// 定义两个UnitHandle结构体的比较运算
		friend bool operator==(UnitHandle const&, UnitHandle const&) = default;
	};
}
