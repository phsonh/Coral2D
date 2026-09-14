#pragma once
#include <cstdint>
#include <limits>


namespace core::object::Unit {

	// 基础逻辑质点。
	struct Unit {
		static constexpr std::int32_t maximum_timer = std::numeric_limits<std::int32_t>::max();
		double x = 0.0;                            // 当前帧横坐标
		double y = 0.0;                            // 当前帧纵坐标
		float dx = 0.0;                            // [Lua只读] 相对于上一帧的横坐标变化量
		float dy = 0.0;                            // [Lua只读] 相对于上一帧的纵坐标变化量
		double vx = 0.0;                           // 当前帧速度水平分量
		double vy = 0.0;                           // 当前帧速度竖直分量
		double ax = 0.0;                           // 当前帧加速度水平分量
		double ay = 0.0;                           // 当前帧加速度竖直分量
		std::int32_t timer = 0;                    // 逻辑帧计时器
		
		void reset() noexcept;                     // 恢复为新建 Unit 的默认初始状态
	};

}
