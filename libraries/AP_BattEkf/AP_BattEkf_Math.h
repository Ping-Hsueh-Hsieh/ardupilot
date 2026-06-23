#pragma once
#include <stdint.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

struct Sample
{
    uint32_t time;
    float curr;
    float volt;
};

struct Bv
{
    float tau;
    float rbv;

    static Bv create_default() { return {20.0f, 1e-4f}; }
};

struct EkfRes
{
    float est_soc;
    float est_ibv;
    float est_volt;
    std::array<float, 4> SigmaX;
};

namespace MathUtils {
inline float clamp(float x, float a, float b)
{
    return (x < a) ? a : ((x > b) ? b : x);
}

inline float interp(float x, const std::vector<float>& xp, const std::vector<float>& fp)
{
    if (xp.empty() || fp.empty()) return 0.0f;
    if (x <= xp.front()) return fp.front();
    if (x >= xp.back()) return fp.back();

    for (size_t i = 0; i < xp.size() - 1; ++i) {
        if (x >= xp[i] && x <= xp[i + 1]) {
            float t = (x - xp[i]) / (xp[i + 1] - xp[i]);
            return fp[i] + t * (fp[i + 1] - fp[i]);
        }
    }
    return fp.front();
}

// 2x2 Matrix & Vector operations (row-major: {a11, a12, a21, a22})
inline std::array<float, 2> mat2_mul_vec2(const std::array<float, 4>& A, const std::array<float, 2>& x)
{
    return {A[0] * x[0] + A[1] * x[1], A[2] * x[0] + A[3] * x[1]};
}

inline std::array<float, 4> mat2_mul_mat2(const std::array<float, 4>& A, const std::array<float, 4>& B)
{
    // Row-Major Layout:
    // [0] [1]
    // [2] [3]
    return {
        A[0] * B[0] + A[1] * B[2],  // Row 1, Col 1
        A[0] * B[1] + A[1] * B[3],  // Row 1, Col 2
        A[2] * B[0] + A[3] * B[2],  // Row 2, Col 1
        A[2] * B[1] + A[3] * B[3]   // Row 2, Col 2
    };
}

inline std::array<float, 4> mat2_add(const std::array<float, 4>& A, const std::array<float, 4>& B)
{
    return {A[0] + B[0], A[1] + B[1], A[2] + B[2], A[3] + B[3]};
}

inline std::array<float, 4> mat2_sub(const std::array<float, 4>& A, const std::array<float, 4>& B)
{
    return {A[0] - B[0], A[1] - B[1], A[2] - B[2], A[3] - B[3]};
}

inline std::array<float, 4> mat2_trans(const std::array<float, 4>& A)
{
    return {A[0], A[2], A[1], A[3]};
}

inline std::array<float, 4> mat2_scale(const std::array<float, 4>& A, float s)
{
    return {A[0] * s, A[1] * s, A[2] * s, A[3] * s};
}

inline std::array<float, 4> outer2(const std::array<float, 2>& u, const std::array<float, 2>& v)
{
    return {u[0] * v[0], u[0] * v[1], u[1] * v[0], u[1] * v[1]};
}

inline float quad1(const std::array<float, 2>& C, const std::array<float, 4>& Sx)
{
    float t1 = C[0] * Sx[0] + C[1] * Sx[2];
    float t2 = C[0] * Sx[1] + C[1] * Sx[3];
    return t1 * C[0] + t2 * C[1];
}

inline float inv1(float x)
{
    return 1.0f / x;
}

inline std::array<float, 4> symmetrize2(const std::array<float, 4>& A)
{
    float avg = (A[1] + A[2]) * 0.5f;
    return {A[0], avg, avg, A[3]};
}
}  // namespace MathUtils
