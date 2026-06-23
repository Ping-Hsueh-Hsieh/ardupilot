all: bin/ekf_test
	./bin/ekf_test
bin/ekf_test: ekf_test.cpp ../AP_BattEkfImpl.cpp
	clang++ -ggdb -fsanitize=address -o bin/ekf_test ekf_test.cpp ../AP_BattEkfImpl.cpp -I../
