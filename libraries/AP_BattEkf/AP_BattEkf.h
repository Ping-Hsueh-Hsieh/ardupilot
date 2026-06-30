#pragma once
#include "AP_BattEkfImpl.h"
#include <AP_Param/AP_Param.h>
#include <GCS_MAVLink/GCS.h>

class AP_BattEkf : public AP_BattEkfImpl
{
   public:
    static const struct AP_Param::GroupInfo var_info[];
    void init(void);
    void update(void);
    void send_mavlink(mavlink_channel_t chan) const;

   private:
    bool _disable = false;
    AP_Int8 param_disable;
};
