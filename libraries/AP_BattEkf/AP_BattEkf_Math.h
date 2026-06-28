#pragma once
#include <stdint.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

struct Sample
{
    uint64_t time;
    float curr;
    float volt;
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

inline std::pair<float, float> eval_poly(const std::array<float, 5>& coeffs, float x)
{
    float a4 = coeffs[0], a3 = coeffs[1], a2 = coeffs[2], a1 = coeffs[3], a0 = coeffs[4];
    float x2 = x * x;
    float x3 = x2 * x;
    float x4 = x3 * x;

    float y = a4 * x4 + a3 * x3 + a2 * x2 + a1 * x + a0;
    float dy = 4.0 * a4 * x3 + 3.0 * a3 * x2 + 2.0 * a2 * x + a1;
    return {y, dy};
}

inline float brent_root(const std::array<float, 5>& coeffs, float a, float b, float tol, int maxits)
{
    float fa = eval_poly(coeffs, a).first;
    float fb = eval_poly(coeffs, b).first;

    if (std::abs(fa) < 1e-15) return a;
    if (std::abs(fb) < 1e-15) return b;
    if ((fa * fb) > 0.0) return -1.f;  // Safe fallback if bracket fails

    float c = a;
    float fc = fa;
    float s = 0.0;
    float fs = 0.0;
    float d = b - a;
    bool mflag = true;

    for (int iter = 0; iter < maxits; ++iter) {
        if (std::abs(fa - fc) > 1e-15 && std::abs(fb - fc) > 1e-15) {
            // Inverse quadratic interpolation
            s = (a * fb * fc) / ((fa - fb) * (fa - fc)) + (b * fa * fc) / ((fb - fa) * (fb - fc)) + (c * fa * fb) / ((fc - fa) * (fc - fb));
        } else {
            // Secant method
            s = b - fb * (b - a) / (fb - fa);
        }

        // Check if conditions for bisection are met
        bool cond1 = (s < (3.0 * a + b) / 4.0) || (s > b);
        bool cond2 = mflag && (std::abs(s - b) >= std::abs(b - c) / 2.0);
        bool cond3 = (!mflag) && (std::abs(s - b) >= std::abs(c - d) / 2.0);
        bool cond4 = mflag && (std::abs(b - c) < tol);
        bool cond5 = (!mflag) && (std::abs(c - d) < tol);

        if (cond1 || cond2 || cond3 || cond4 || cond5) {
            s = (a + b) / 2.0;
            mflag = true;
        } else {
            mflag = false;
        }

        fs = eval_poly(coeffs, s).first;
        d = c;
        c = b;
        fc = fb;

        if (fa * fs < 0.0) {
            b = s;
            fb = fs;
        } else {
            a = s;
            fa = fs;
        }

        if (std::abs(fa) < std::abs(fb)) {
            std::swap(a, b);
            std::swap(fa, fb);
        }

        if ((std::abs(b - a) < tol) || (std::abs(fb) < 1e-15)) {
            return b;
        }
    }
    return b;
}

inline std::vector<float> unique_sorted(std::vector<float>& vals, float tol)
{
    if (vals.empty()) return vals;
    std::sort(vals.begin(), vals.end());

    std::vector<float> out;
    out.push_back(vals[0]);

    for (size_t i = 1; i < vals.size(); ++i) {
        if (std::abs(vals[i] - vals[i - 1]) > tol) {
            out.push_back(vals[i]);
        }
    }
    return out;
}

inline std::vector<float> find_positive_roots(const std::array<float, 5>& coeffs, float xmin, float xmax, float step, float tol)
{
    std::vector<float> roots;
    float x0 = xmin;
    float f0 = eval_poly(coeffs, x0).first;
    int samples = std::max(2, static_cast<int>(std::floor((xmax - xmin) / step)) + 1);

    for (int i = 1; i <= samples; ++i) {
        float x1 = std::min(xmax, xmin + i * step);
        float f1 = eval_poly(coeffs, x1).first;

        if ((std::abs(f0) < 1e-15) && (x0 > 0.0)) {
            roots.push_back(x0);
        }

        if (f0 * f1 < 0.0) {
            float r = brent_root(coeffs, x0, x1, tol, 100);
            if (r > 0.0) {
                roots.push_back(r);
            }
        }

        if (std::abs(f1) < 1e-12 && x1 > 0.0) {
            roots.push_back(x1);
        }

        x0 = x1;
        f0 = f1;
        if (x0 >= xmax) break;
    }

    return unique_sorted(roots, std::sqrt(tol));
}

inline size_t argmin(const std::vector<float>& vec)
{
    if (vec.empty()) return 0;
    auto min_it = std::min_element(vec.begin(), vec.end());
    return std::distance(vec.begin(), min_it);
}
}  // namespace MathUtils
