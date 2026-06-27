#pragma once
#include "AP_BattEkfImpl.h"

class AP_BattEkf : public AP_BattEkfImpl
{
   public:
    void update(void);
};
