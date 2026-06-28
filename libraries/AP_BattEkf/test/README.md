# Test

Test [EKF](../AP_BattEkf.cpp) against the [flight record](./record/output_data.csv).

# How to run the test

```sh
mkdir -p bin
mkdir -p out
make -B
python show.py  # show the result with matplotlib
```
