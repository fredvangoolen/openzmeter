S1 = ../../Source/Devices/S1

s1monitor: S1Monitor.cpp $(S1)/DeviceS1Frame.cpp $(S1)/DeviceS1Serial.cpp $(S1)/DeviceS1Frame.h $(S1)/DeviceS1Serial.h
	g++ -O2 -Wall -std=c++17 -I $(S1) -o $@ S1Monitor.cpp $(S1)/DeviceS1Frame.cpp $(S1)/DeviceS1Serial.cpp -lm

clean:
	rm -f s1monitor

.PHONY: clean
