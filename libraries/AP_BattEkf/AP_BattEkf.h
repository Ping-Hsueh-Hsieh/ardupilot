#pragma once
#include "AP_BattEkfImpl.h"
#include <AP_Param/AP_Param.h>

class AP_BattEkf : public AP_BattEkfImpl
{
   public:
    void update(void);
    void init(void);
    static const struct AP_Param::GroupInfo var_info[];

   private:
    bool _disable = false;
    AP_Int8 param_disable;
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
