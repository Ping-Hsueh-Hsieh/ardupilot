#pragma once
#include <array>
#include "AP_BattEkf_Math.h"
#include "AP_BattEkf_Model.h"
#include "AP_BattEkfImpl.h"

class AP_BattEkf : public AP_BattEkfImpl
{
   public:
    void update(void);
};
