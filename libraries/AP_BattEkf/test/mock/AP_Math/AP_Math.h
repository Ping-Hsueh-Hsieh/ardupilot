#include <cmath>
#include <limits>

template <typename T>
inline bool is_zero(const T val)
{
    // 1. Catch NaN and Infinity using standard library functions
    if (std::isnan(val) || std::isinf(val)) {
        return true;  // Treat them as zero/invalid to prevent crashes
    }

    // 2. Compare against machine epsilon (the smallest representable float step)
    return std::fabs(val) < std::numeric_limits<T>::epsilon();
}
