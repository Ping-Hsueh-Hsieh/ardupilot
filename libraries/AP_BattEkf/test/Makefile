all: bin/ekf_test
	./bin/ekf_test
bin/ekf_test: ekf_test.cpp ../AP_BattEkfImpl.cpp ../AP_BattCagi.cpp ../AP_BattRemTimeCalc.cpp
	clang++ -ggdb -fsanitize=address -o bin/ekf_test ekf_test.cpp ../AP_BattEkfImpl.cpp ../AP_BattCagi.cpp ../AP_BattRemTimeCalc.cpp -I../ -Imock/
