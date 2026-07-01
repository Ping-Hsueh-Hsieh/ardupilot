#pragma once
#include <AP_Param/AP_Param.h>
#include "AP_BattCagi.h"
#include "AP_BattEkfImpl.h"
#include "AP_BattRemTimeCalc.h"

class AP_BattEkf : public AP_BattEkfImpl
{
   public:
    void update(void);
    void init(void);
    static const struct AP_Param::GroupInfo var_info[];

   private:
    bool _disable = false;
    AP_Int8 param_disable;
    void gcs_writer(void);
    bool c_agi_first = true;
    AP_BattRemTimeCalc rem_calc;
    AP_BattCagi c_agi_awtls = AP_BattCagi(this->get_bat());

    // NOTE: GCS message handler
    uint8_t msg_id = 0;
    static constexpr uint8_t gcs_writer_period = 10;

    enum GcsMsg
    {
        SOC = 0,
        REM_FW,
        REM_RTL,
        SOH,
        ESTCAP,
        NUM,
    };

    static_assert(GcsMsg::NUM <= gcs_writer_period, "[ERROR] violate GcsMsg::NUM <= gcs_writer_period");
};
