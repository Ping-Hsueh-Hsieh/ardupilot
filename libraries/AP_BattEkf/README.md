# AP_BattEkf

- Use EKF to estimate battery SOC.
- 2-nd order error system is chosen to balance the computational cost and the SOC estimation accuracy.
- Use LS to estimate capacity SOH with estimated SOC.
- Only Power Module is required for this hardware setup. No external BMS (hardware) required.
- Only these interfaces are required from AP_BattMonitor.
    ```cpp
    AP::battery().voltage()
    AP::battery().current_amps(curr)
    ```

# How to use this library

Take ArduPlane for example.
**NOTE**: This is only the example configuration, please tailor to your need.

1. Add `AP_BattEkf` instance to `Plane.h`.
    ```cpp
    class Plane : public AP_Vehicle {
        ...
    private:
        ...
        AP_BattEkf batt_ekf;
        ...
    }
    ```
2. Add task to scheduler.
    ```cpp
    SCHED_TASK_CLASS(AP_BattEkf, &plane.batt_ekf, update,   10, 300,  66),
    ```
3. Update the battery model [`AP_BattEkf_Model.h`](./AP_BattEkf_Model.h) to the actual battery is used.
    - `class Ocv`: OCV property of battery
    - `class Ir`: Internal resistance (ACIR) of the battery. Assume the `Ir = function(current)`.
    - `class Bv`: Equivalent Butler-Volmer resistance. Assume constant.

# Test

Checkout [test](./test/README.md).

# Document

Checkout [brief explanation about EKF implementation](./doc/main.pdf).
