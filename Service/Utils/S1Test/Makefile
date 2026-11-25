# This file is part of openZmeter, licensed under the GNU General Public License, version 3 or (at your option) any later
# version, without any warranty. See the LICENSE file. SPDX-License-Identifier: GPL-3.0-or-later

S1 = ../../Source/Devices/S1

s1test: S1Test.cpp $(S1)/DeviceS1Frame.cpp $(S1)/DeviceS1Frame.h $(S1)/DeviceS1Rate.cpp $(S1)/DeviceS1Rate.h
	g++ -O2 -Wall -std=c++17 -fsingle-precision-constant -I $(S1) -o $@ S1Test.cpp $(S1)/DeviceS1Frame.cpp $(S1)/DeviceS1Rate.cpp -lm

test: s1test
	./s1test

clean:
	rm -f s1test

.PHONY: test clean
