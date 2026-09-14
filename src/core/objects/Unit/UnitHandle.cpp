#include "UnitHandle.hpp"

namespace core::object::Unit {

	bool UnitHandle::is_valid() const noexcept {
		return index != invalid_index;
	}

}