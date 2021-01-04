EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr User 5906 4724
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Components:8205S U17
U 1 1 5FF00CA8
P 2950 2000
F 0 "U17" H 2625 1552 50  0000 C CNN
F 1 "8205S" H 2625 1461 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-23-6" H 2950 2000 50  0001 C CNN
F 3 "" H 2950 2000 50  0001 C CNN
	1    2950 2000
	1    0    0    -1  
$EndComp
$Comp
L Components:DW01A U14
U 1 1 5FF0A653
P 2550 1525
F 0 "U14" H 2537 1692 50  0000 C CNN
F 1 "DW01A" H 2537 1601 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-23-6" H 2550 1525 50  0001 C CNN
F 3 "" H 2550 1525 50  0001 C CNN
	1    2550 1525
	1    0    0    -1  
$EndComp
$Comp
L 3Phase-rescue:Thermistor_NTC-3Phase-rescue TH2
U 1 1 5FF12BB3
P 3325 1900
F 0 "TH2" H 3422 1946 50  0000 L CNN
F 1 "Thermistor_NTC-3Phase-rescue" H 3422 1855 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric" H 3325 1950 50  0001 C CNN
F 3 "" H 3325 1950 50  0001 C CNN
	1    3325 1900
	1    0    0    -1  
$EndComp
$Comp
L Components:BQ21040 U16
U 1 1 5FF180D9
P 3875 1700
F 0 "U16" H 4613 1696 50  0000 L CNN
F 1 "BQ21040" H 4613 1605 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23-6" H 3875 1200 50  0001 C CNN
F 3 "https://www.analog.com/media/en/technical-documentation/data-sheets/405442xf.pdf" H 3875 1600 50  0001 C CNN
	1    3875 1700
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x06_Male J8
U 1 1 604239FD
P 4925 1600
F 0 "J8" H 4897 1482 50  0000 R CNN
F 1 "Conn_01x06_Male" H 4897 1573 50  0000 R CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x06_P2.54mm_Vertical" H 4925 1600 50  0001 C CNN
F 3 "~" H 4925 1600 50  0001 C CNN
	1    4925 1600
	-1   0    0    1   
$EndComp
$Comp
L 3Phase-rescue:R-3Phase-rescue R69
U 1 1 6043BE3B
P 3125 1900
F 0 "R69" V 3205 1900 50  0000 C CNN
F 1 "100k" H 3125 1900 50  0001 C CNN
F 2 "Footprints:R_0603_1608Metric" V 3055 1900 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Chip-Resistor-Surface-Mount_100KR-104-5_C100048.html" H 3125 1900 50  0001 C CNN
	1    3125 1900
	-1   0    0    1   
$EndComp
Wire Wire Line
	2575 1975 2575 1950
Wire Wire Line
	2575 1950 2675 1950
Wire Wire Line
	2675 1950 2675 1975
Wire Wire Line
	2425 1975 2425 1950
Wire Wire Line
	2425 1950 2300 1950
Wire Wire Line
	2300 1950 2300 1800
Wire Wire Line
	2300 1800 2325 1800
Wire Wire Line
	2825 1975 2825 1950
Wire Wire Line
	2825 1950 2950 1950
Wire Wire Line
	2950 1950 2950 1800
Wire Wire Line
	2950 1800 2925 1800
Wire Wire Line
	2225 2275 2200 2275
Wire Wire Line
	2925 1700 3125 1700
Wire Wire Line
	3125 1700 3125 1750
Wire Wire Line
	3025 2275 3125 2275
Wire Wire Line
	3125 2275 3125 2050
Connection ~ 3125 2275
$Comp
L 3Phase-rescue:R-3Phase-rescue R68
U 1 1 6043CE8A
P 3100 1400
F 0 "R68" V 3180 1400 50  0000 C CNN
F 1 "100k" H 3100 1400 50  0001 C CNN
F 2 "Footprints:R_0603_1608Metric" V 3030 1400 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Chip-Resistor-Surface-Mount_100KR-104-5_C100048.html" H 3100 1400 50  0001 C CNN
	1    3100 1400
	0    1    1    0   
$EndComp
Wire Wire Line
	2325 1600 2300 1600
Wire Wire Line
	2300 1600 2300 1400
Wire Wire Line
	2300 1400 2200 1400
Wire Wire Line
	2025 1700 2025 1675
Connection ~ 2200 1700
Wire Wire Line
	2325 1700 2200 1700
Wire Wire Line
	2200 1700 2025 1700
Wire Wire Line
	2200 2275 2200 1700
Wire Wire Line
	2200 1400 2200 1425
Wire Wire Line
	2200 1700 2200 1625
$Comp
L 3Phase-rescue:C_Small-3Phase-rescue C38
U 1 1 60441A0A
P 2200 1525
F 0 "C38" H 2210 1595 50  0000 L CNN
F 1 "100nF 25v" H 2210 1445 50  0001 L CNN
F 2 "Footprints:R_0603_1608Metric" H 2200 1525 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Multilayer-Ceramic-Capacitors-MLCC-SMD-SMT_SAMSUNG_CL10B104KA8NNNC_100nF-104-10-25V_C1590.html" H 2200 1525 50  0001 C CNN
	1    2200 1525
	1    0    0    1   
$EndComp
Wire Wire Line
	2950 1400 2300 1400
Connection ~ 2300 1400
Wire Wire Line
	2025 1375 2025 1300
$Comp
L Device:Battery_Cell BT3
U 1 1 5FF0F7E5
P 2025 1575
F 0 "BT3" H 2143 1671 50  0000 L CNN
F 1 "Battery_Cell" H 2143 1580 50  0000 L CNN
F 2 "Footprints:BatteryHolder_Keystone_1042_1x18650" V 2025 1635 50  0001 C CNN
F 3 "~" V 2025 1635 50  0001 C CNN
	1    2025 1575
	1    0    0    -1  
$EndComp
Wire Wire Line
	3475 1700 3325 1700
Wire Wire Line
	3325 1700 3325 1750
Wire Wire Line
	3475 1800 3450 1800
Wire Wire Line
	3450 1800 3450 2275
Wire Wire Line
	3475 1600 3450 1600
Wire Wire Line
	3450 1600 3450 1400
Wire Wire Line
	2025 1300 3450 1300
Wire Wire Line
	3250 1400 3450 1400
Connection ~ 3450 1400
Wire Wire Line
	3450 1400 3450 1300
Wire Wire Line
	3325 2050 3325 2275
Wire Wire Line
	3125 2275 3325 2275
Connection ~ 3325 2275
Wire Wire Line
	3325 2275 3450 2275
$Comp
L 3Phase-rescue:R-3Phase-rescue R70
U 1 1 618133A7
P 4125 2000
F 0 "R70" V 4205 2000 50  0000 C CNN
F 1 "100k" H 4125 2000 50  0001 C CNN
F 2 "Footprints:R_0603_1608Metric" V 4055 2000 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Chip-Resistor-Surface-Mount_100KR-104-5_C100048.html" H 4125 2000 50  0001 C CNN
	1    4125 2000
	-1   0    0    1   
$EndComp
Wire Wire Line
	4075 1800 4125 1800
Wire Wire Line
	4125 1800 4125 1850
Wire Wire Line
	3450 2275 4125 2275
Wire Wire Line
	4125 2275 4125 2150
Connection ~ 3450 2275
$Comp
L 3Phase-rescue:LED_ALT-3Phase-rescue D17
U 1 1 61910C98
P 4500 2000
F 0 "D17" H 4500 2100 50  0000 C CNN
F 1 "3mm blue LED" H 4500 1900 50  0001 C CNN
F 2 "LED_SMD:LED_0603_1608Metric" H 4500 2000 50  0001 C CNN
F 3 "https://es.aliexpress.com/store/product/100PCS-x-Long-Leg-3mm-5mm-White-Diffused-Jade-Green-Yellow-Red-Blue-Ultra-Bright-LED/102034_32887271791.html" H 4500 2000 50  0001 C CNN
	1    4500 2000
	0    -1   -1   0   
$EndComp
$Comp
L 3Phase-rescue:R-3Phase-rescue R71
U 1 1 61914591
P 4300 2000
F 0 "R71" V 4380 2000 50  0000 C CNN
F 1 "100k" H 4300 2000 50  0001 C CNN
F 2 "Footprints:R_0603_1608Metric" V 4230 2000 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Chip-Resistor-Surface-Mount_100KR-104-5_C100048.html" H 4300 2000 50  0001 C CNN
	1    4300 2000
	-1   0    0    1   
$EndComp
Wire Wire Line
	4500 2150 4500 2200
Wire Wire Line
	4500 2200 4300 2200
Wire Wire Line
	4300 2200 4300 2150
Wire Wire Line
	4300 1850 4300 1700
Wire Wire Line
	4300 1700 4075 1700
Wire Wire Line
	4075 1600 4500 1600
Wire Wire Line
	4500 1850 4500 1800
Connection ~ 3450 1300
Wire Wire Line
	3450 1300 4700 1300
Wire Wire Line
	4725 1400 4700 1400
Wire Wire Line
	4700 1400 4700 1300
Connection ~ 4700 1300
Wire Wire Line
	4700 1300 4725 1300
$Comp
L Transistor_FET:2N7002 Q4
U 1 1 628A337B
P 1950 1975
F 0 "Q4" H 2155 2021 50  0000 L CNN
F 1 "2N7002" H 2155 1930 50  0000 L CNN
F 2 "Footprints:SOT-23" H 2150 1900 50  0001 L CIN
F 3 "https://lcsc.com/product-detail/MOSFET_Guangdong-Hottech-2N7002_C181082.html" H 1950 1975 50  0001 L CNN
	1    1950 1975
	1    0    0    -1  
$EndComp
$Comp
L 3Phase-rescue:Buzzer-3Phase-rescue BZ2
U 1 1 628A53C3
P 1550 1625
F 0 "BZ2" V 1425 1550 50  0000 L CNN
F 1 "QMB-09B-05" H 1700 1575 50  0001 L CNN
F 2 "Buzzer_Beeper:MagneticBuzzer_Kobitone_254-EMB84Q-RO" V 1525 1725 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Buzzers_Jiangsu-Huaneng-Elec-QMB-09B-05_C161819.html" V 1525 1725 50  0001 C CNN
	1    1550 1625
	0    -1   -1   0   
$EndComp
$Comp
L Connector:Conn_01x07_Male J7
U 1 1 62932ADC
P 850 2075
F 0 "J7" H 958 2456 50  0000 C CNN
F 1 "Conn_01x06_Male" H 958 2365 50  0001 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x07_P2.54mm_Vertical" H 850 2075 50  0001 C CNN
F 3 "~" H 850 2075 50  0001 C CNN
	1    850  2075
	1    0    0    -1  
$EndComp
$Comp
L 3Phase-rescue:LED_ALT-3Phase-rescue D16
U 1 1 62941D3A
P 1675 2175
F 0 "D16" H 1675 2275 50  0000 C CNN
F 1 "3mm blue LED" H 1675 2075 50  0001 C CNN
F 2 "LED_SMD:LED_0603_1608Metric" H 1675 2175 50  0001 C CNN
F 3 "https://es.aliexpress.com/store/product/100PCS-x-Long-Leg-3mm-5mm-White-Diffused-Jade-Green-Yellow-Red-Blue-Ultra-Bright-LED/102034_32887271791.html" H 1675 2175 50  0001 C CNN
	1    1675 2175
	-1   0    0    1   
$EndComp
$Comp
L 3Phase-rescue:R-3Phase-rescue R67
U 1 1 62A0039E
P 1325 2275
F 0 "R67" V 1405 2275 50  0000 C CNN
F 1 "100k" H 1325 2275 50  0001 C CNN
F 2 "Footprints:R_0603_1608Metric" V 1255 2275 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Chip-Resistor-Surface-Mount_100KR-104-5_C100048.html" H 1325 2275 50  0001 C CNN
	1    1325 2275
	0    -1   -1   0   
$EndComp
$Comp
L 3Phase-rescue:R-3Phase-rescue R61
U 1 1 62E59093
P 1325 2175
F 0 "R61" V 1405 2175 50  0000 C CNN
F 1 "100k" H 1325 2175 50  0001 C CNN
F 2 "Footprints:R_0603_1608Metric" V 1255 2175 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Chip-Resistor-Surface-Mount_100KR-104-5_C100048.html" H 1325 2175 50  0001 C CNN
	1    1325 2175
	0    1    1    0   
$EndComp
$Comp
L 3Phase-rescue:R-3Phase-rescue R60
U 1 1 62E59B6F
P 1325 2075
F 0 "R60" V 1405 2075 50  0000 C CNN
F 1 "100k" H 1325 2075 50  0001 C CNN
F 2 "Footprints:R_0603_1608Metric" V 1255 2075 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Chip-Resistor-Surface-Mount_100KR-104-5_C100048.html" H 1325 2075 50  0001 C CNN
	1    1325 2075
	0    1    1    0   
$EndComp
Wire Wire Line
	1475 2075 1525 2075
Wire Wire Line
	1475 2175 1525 2175
Wire Wire Line
	1050 2175 1175 2175
$Comp
L 3Phase-rescue:LED_ALT-3Phase-rescue D18
U 1 1 645CE090
P 1675 2275
F 0 "D18" H 1675 2375 50  0000 C CNN
F 1 "3mm blue LED" H 1675 2175 50  0001 C CNN
F 2 "LED_SMD:LED_0603_1608Metric" H 1675 2275 50  0001 C CNN
F 3 "https://es.aliexpress.com/store/product/100PCS-x-Long-Leg-3mm-5mm-White-Diffused-Jade-Green-Yellow-Red-Blue-Ultra-Bright-LED/102034_32887271791.html" H 1675 2275 50  0001 C CNN
	1    1675 2275
	-1   0    0    1   
$EndComp
Wire Wire Line
	1050 2375 1850 2375
Wire Wire Line
	2050 2175 2050 2375
$Comp
L 3Phase-rescue:LED_ALT-3Phase-rescue D7
U 1 1 62943BE2
P 1675 2075
F 0 "D7" H 1675 2175 50  0000 C CNN
F 1 "3mm blue LED" H 1675 1975 50  0001 C CNN
F 2 "LED_SMD:LED_0603_1608Metric" H 1675 2075 50  0001 C CNN
F 3 "https://es.aliexpress.com/store/product/100PCS-x-Long-Leg-3mm-5mm-White-Diffused-Jade-Green-Yellow-Red-Blue-Ultra-Bright-LED/102034_32887271791.html" H 1675 2075 50  0001 C CNN
	1    1675 2075
	-1   0    0    1   
$EndComp
Wire Wire Line
	1825 2075 1850 2075
Wire Wire Line
	1850 2075 1850 2175
Connection ~ 1850 2375
Wire Wire Line
	1850 2375 2050 2375
Wire Wire Line
	1825 2175 1850 2175
Connection ~ 1850 2175
Wire Wire Line
	1850 2175 1850 2275
Wire Wire Line
	1825 2275 1850 2275
Connection ~ 1850 2275
Wire Wire Line
	1850 2275 1850 2375
Wire Wire Line
	1175 2075 1050 2075
Wire Wire Line
	1050 2275 1175 2275
Wire Wire Line
	1650 1725 1650 1750
Wire Wire Line
	1650 1750 1775 1750
Wire Wire Line
	2050 1750 2050 1775
Wire Wire Line
	1450 1875 1450 1725
Wire Wire Line
	1475 2275 1525 2275
$Comp
L 3Phase-rescue:D_Small_ALT-3Phase-rescue D15
U 1 1 6569F29D
P 1600 1875
F 0 "D15" H 1550 1955 50  0000 L CNN
F 1 "ES1J" H 1450 1795 50  0001 L CNN
F 2 "Footprints:D_SOD-323" V 1600 1875 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Diodes-General-Purpose_Bourne-Semicon-Shenzhen-ES1J_C266561.html" V 1600 1875 50  0001 C CNN
	1    1600 1875
	1    0    0    -1  
$EndComp
Wire Wire Line
	1050 1975 1750 1975
Wire Wire Line
	1500 1875 1450 1875
Connection ~ 1450 1875
Wire Wire Line
	1700 1875 1775 1875
Wire Wire Line
	1775 1875 1775 1750
Connection ~ 1775 1750
Wire Wire Line
	1775 1750 2050 1750
Wire Wire Line
	2050 2375 2050 2475
Wire Wire Line
	2050 2475 3125 2475
Wire Wire Line
	3125 2475 3125 2275
Connection ~ 2050 2375
$Comp
L power:GND #PWR0101
U 1 1 5FF0D2D3
P 3325 2375
F 0 "#PWR0101" H 3325 2125 50  0001 C CNN
F 1 "GND" H 3330 2202 50  0000 C CNN
F 2 "" H 3325 2375 50  0001 C CNN
F 3 "" H 3325 2375 50  0001 C CNN
	1    3325 2375
	1    0    0    -1  
$EndComp
Wire Wire Line
	3325 2375 3325 2275
$Comp
L 3Phase-rescue:SW_Push-3Phase-rescue SW1
U 1 1 5FE4F8B5
P 1150 1550
F 0 "SW1" V 1241 1502 50  0000 R CNN
F 1 "SW_Push-3Phase-rescue" V 1150 1502 50  0001 R CNN
F 2 "Button_Switch_SMD:SW_Push_1P1T_NO_6x6mm_H9.5mm" V 1059 1502 50  0001 R CNN
F 3 "" H 1150 1750 50  0000 C CNN
	1    1150 1550
	0    -1   -1   0   
$EndComp
Wire Wire Line
	1300 1300 1150 1300
Wire Wire Line
	1150 1300 1150 1350
$Comp
L 3Phase-rescue:R-3Phase-rescue R1
U 1 1 5FE634D2
P 1300 1575
F 0 "R1" V 1380 1575 50  0000 C CNN
F 1 "100k" H 1300 1575 50  0001 C CNN
F 2 "Footprints:R_0603_1608Metric" V 1230 1575 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Chip-Resistor-Surface-Mount_100KR-104-5_C100048.html" H 1300 1575 50  0001 C CNN
	1    1300 1575
	-1   0    0    1   
$EndComp
Wire Wire Line
	1300 1425 1300 1300
Wire Wire Line
	4725 1700 4500 1700
Connection ~ 4500 1700
Wire Wire Line
	4500 1700 4500 1600
Wire Wire Line
	4725 1800 4500 1800
Connection ~ 4500 1800
Wire Wire Line
	4500 1800 4500 1700
$Comp
L power:GND #PWR0102
U 1 1 5FEB6B51
P 4650 1500
F 0 "#PWR0102" H 4650 1250 50  0001 C CNN
F 1 "GND" H 4655 1327 50  0000 C CNN
F 2 "" H 4650 1500 50  0001 C CNN
F 3 "" H 4650 1500 50  0001 C CNN
	1    4650 1500
	0    1    1    0   
$EndComp
Wire Wire Line
	4650 1500 4700 1500
Wire Wire Line
	4725 1600 4700 1600
Wire Wire Line
	4700 1600 4700 1500
Connection ~ 4700 1500
Wire Wire Line
	4700 1500 4725 1500
Wire Wire Line
	1050 1875 1300 1875
Wire Wire Line
	1150 1775 1150 1750
Wire Wire Line
	1050 1775 1150 1775
Wire Wire Line
	1300 1725 1300 1875
Connection ~ 1300 1875
Wire Wire Line
	1300 1875 1450 1875
$EndSCHEMATC
