S1 = ../../Source/Devices/S1

s1test: S1Test.cpp $(S1)/DeviceS1Frame.cpp $(S1)/DeviceS1Frame.h
	g++ -O2 -Wall -std=c++17 -fsingle-precision-constant -I $(S1) -o $@ S1Test.cpp $(S1)/DeviceS1Frame.cpp -lm

test: s1test
	./s1test

clean:
	rm -f s1test

.PHONY: test clean
