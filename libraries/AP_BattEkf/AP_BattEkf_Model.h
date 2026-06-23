#pragma once
#include "AP_BattEkf_Math.h"
#include <vector>

class Ir {
public:
    float c_rate_Ah;
    std::vector<float> bp_currs;
    std::vector<float> irs;

    float luk_ir(float curr) const {
        return MathUtils::interp(curr, bp_currs, irs);
    }

    static Ir create_default() {
        return {
            21.671f,
            {-104.8876f, -62.9326f, -20.9775f, -10.4888f, 0.0000f},
            {0.0017f, 0.0012f, 0.0008f, 0.0003f, 0.0000f}
        };
    }
};

class Ocv {
public:
    std::vector<float> ocv_soc;
    std::vector<float> ocv_vol;
    std::vector<float> docvdsoc_x;
    std::vector<float> docvdsoc_y;

    float luk_soc(float volt) const { return MathUtils::interp(volt, ocv_vol, ocv_soc); }
    float luk_volt(float soc) const { return MathUtils::interp(soc, ocv_soc, ocv_vol); }
    float luk_docvdsoc_y(float soc) const { return MathUtils::interp(soc, docvdsoc_x, docvdsoc_y); }

    static Ocv create_default() {
        return {
            {0.0000f, 0.0281f, 0.0381f, 0.0599f, 0.1005f, 0.1171f, 0.1713f, 0.2271f, 0.3848f, 0.4989f, 0.5924f, 0.7531f, 0.8072f, 0.8637f, 0.8935f, 0.9531f, 0.9973f, 1.0000f},
            {3.5000f, 3.6310f, 3.6507f, 3.6645f, 3.6747f, 3.6852f, 3.7118f, 3.7294f, 3.7618f, 3.7963f, 3.8382f, 3.9380f, 3.9690f, 4.0480f, 4.0640f, 4.1186f, 4.1735f, 4.2000f},
            {0.000000f, 0.040000f, 0.060000f, 0.090000f, 0.110000f, 0.150000f, 0.315000f, 0.720000f, 0.760000f, 0.775000f, 0.840000f, 0.845000f, 0.875700f, 0.882700f, 0.903600f, 0.974420f, 0.984200f, 0.990000f, 1.000000f},
            {6.636597f, 0.919029f, 0.318019f, 0.160917f, 0.825008f, 0.401792f, 0.130068f, 0.540873f, 0.659074f, 0.518795f, 1.838305f, 1.899539f, 0.404280f, 0.395704f, 0.931634f, 1.098123f, 1.232950f, 1.517833f, 1.517833f}
        };
    }
};

class Bat {
public:
    float Q_Ah_orig;
    Ocv ocv;
    float num_of_cell;
    Ir ir;
    Bv bv;
    float c_agi;
    float Q_Ah;
    float Q_As;
    float Q_mAh;
    float Q_mAs;

    Bat(float q_ah, Ocv o, float cells, float c_ag, Ir i, Bv b)
        : Q_Ah_orig(q_ah), ocv(o), num_of_cell(cells), ir(i), bv(b), c_agi(c_ag) {
        update_internal_capacities();
    }

    void update_c_agi(float new_c_agi) {
        this->c_agi = new_c_agi;
        update_internal_capacities();
    }

    static Bat create_default() {
        return Bat(21.671f, Ocv::create_default(), 6.0f, 0.0f, Ir::create_default(), Bv::create_default());
    }

private:
    void update_internal_capacities() {
        Q_Ah = Q_Ah_orig * (1.0f + c_agi);
        Q_As = Q_Ah * 3600.0f;
        Q_mAh = Q_Ah * 1000.0f;
        Q_mAs = Q_As * 1000.0f;
    }
};
