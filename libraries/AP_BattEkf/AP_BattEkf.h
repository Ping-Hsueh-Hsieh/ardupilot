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
};
