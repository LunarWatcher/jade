#include <chrono>

namespace jade {

using Clock = std::chrono::steady_clock;

template <class Type, class BaseClass>
concept CheckType = std::is_base_of<BaseClass, Type>::value;

}
