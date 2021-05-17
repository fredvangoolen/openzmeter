EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
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
P 4575 1050
F 0 "U17" H 4000 625 50  0000 C CNN
F 1 "8205S" H 4450 625 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-23-6" H 4575 1050 50  0001 C CNN
F 3 "" H 4575 1050 50  0001 C CNN
	1    4575 1050
	-1   0    0    1   
$EndComp
$Comp
L 3Phase-rescue:R-3Phase-rescue R69
U 1 1 6043BE3B
P 4500 1225
F 0 "R69" V 4580 1225 50  0000 C CNN
F 1 "100k" H 4500 1225 50  0001 C CNN
F 2 "Footprints:R_0603_1608Metric" V 4430 1225 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Chip-Resistor-Surface-Mount_100KR-104-5_C100048.html" H 4500 1225 50  0001 C CNN
	1    4500 1225
	1    0    0    -1  
$EndComp
Wire Wire Line
	4500 1775 4500 1375
Wire Wire Line
	4500 775  4500 1075
$Comp
L 3Phase-rescue:R-3Phase-rescue R68
U 1 1 6043CE8A
P 5650 1725
F 0 "R68" V 5730 1725 50  0000 C CNN
F 1 "100k" H 5650 1725 50  0001 C CNN
F 2 "Footprints:R_0603_1608Metric" V 5580 1725 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Chip-Resistor-Surface-Mount_100KR-104-5_C100048.html" H 5650 1725 50  0001 C CNN
	1    5650 1725
	-1   0    0    1   
$EndComp
Wire Wire Line
	4500 775  4175 775 
$Comp
L power:GND #PWR0101
U 1 1 5FF0D2D3
P 4025 725
F 0 "#PWR0101" H 4025 475 50  0001 C CNN
F 1 "GND" H 4030 552 50  0000 C CNN
F 2 "" H 4025 725 50  0001 C CNN
F 3 "" H 4025 725 50  0001 C CNN
	1    4025 725 
	-1   0    0    1   
$EndComp
Wire Wire Line
	4025 725  4025 775 
$Comp
L Components:MC34063AD U4
U 1 1 6072003E
P 3100 1500
F 0 "U4" H 2825 1600 50  0000 C CNN
F 1 "MC34063AD" H 3100 1600 50  0000 C CNN
F 2 "Package_SO:SOIC-8_3.9x4.9mm_P1.27mm" H 3150 1050 50  0001 L CNN
F 3 "http://www.onsemi.com/pub_link/Collateral/MC34063A-D.PDF" H 3600 1400 50  0001 C CNN
	1    3100 1500
	1    0    0    1   
$EndComp
$Comp
L Components:HY2120CB U3
U 1 1 60723F31
P 4975 1950
F 0 "U3" H 5050 2217 50  0000 C CNN
F 1 "HY2120CB" H 5050 2126 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-23-6" H 4975 1950 50  0001 C CNN
F 3 "" H 4975 1950 50  0001 C CNN
	1    4975 1950
	-1   0    0    1   
$EndComp
$Comp
L Components:HY2213-BB3A U1
U 1 1 60724CA2
P 6750 1975
F 0 "U1" H 6825 2242 50  0000 C CNN
F 1 "HY2213-BB3A" H 6875 2025 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-23-6" H 6750 1975 50  0001 C CNN
F 3 "" H 6750 1975 50  0001 C CNN
	1    6750 1975
	-1   0    0    1   
$EndComp
$Comp
L Components:HY2213-BB3A U2
U 1 1 607257E8
P 6750 1225
F 0 "U2" H 6825 1492 50  0000 C CNN
F 1 "HY2213-BB3A" H 6875 1275 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-23-6" H 6750 1225 50  0001 C CNN
F 3 "" H 6750 1225 50  0001 C CNN
	1    6750 1225
	-1   0    0    1   
$EndComp
$Comp
L 3Phase-rescue:Buzzer-3Phase-rescue BZ2
U 1 1 628A53C3
P 6075 2550
F 0 "BZ2" V 5950 2475 50  0000 L CNN
F 1 "QMB-09B-05" H 6225 2500 50  0001 L CNN
F 2 "Footprints:Buzzer_CUI_CPT-9019S-SMT" V 6050 2650 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Buzzers_Jiangsu-Huaneng-Elec-QMB-09B-05_C161819.html" V 6050 2650 50  0001 C CNN
	1    6075 2550
	0    1    -1   0   
$EndComp
$Comp
L Components:Battery_Cell BT1
U 1 1 607351E4
P 5850 1400
F 0 "BT1" H 5706 1609 50  0000 R CNN
F 1 "Battery_Cell" H 5706 1518 50  0000 R CNN
F 2 "Footprints:BatteryHolder_Keystone_1042_1x18650" V 5850 1460 50  0001 C CNN
F 3 "~" V 5850 1460 50  0001 C CNN
	1    5850 1400
	1    0    0    1   
$EndComp
$Comp
L Transistor_FET:DMG2302U Q1
U 1 1 60818296
P 6175 1800
F 0 "Q1" H 6100 1650 50  0000 L CNN
F 1 "DMG2302U" H 6379 1755 50  0001 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 6375 1725 50  0001 L CIN
F 3 "http://www.diodes.com/assets/Datasheets/DMG2302U.pdf" H 6175 1800 50  0001 L CNN
	1    6175 1800
	-1   0    0    1   
$EndComp
$Comp
L Transistor_FET:DMG2302U Q2
U 1 1 60822CF3
P 6175 1050
F 0 "Q2" H 6075 925 50  0000 L CNN
F 1 "DMG2302U" H 6379 1005 50  0001 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 6375 975 50  0001 L CIN
F 3 "http://www.diodes.com/assets/Datasheets/DMG2302U.pdf" H 6175 1050 50  0001 L CNN
	1    6175 1050
	-1   0    0    1   
$EndComp
$Comp
L Device:C_Small C2
U 1 1 60838577
P 7075 1050
F 0 "C2" H 7167 1096 50  0000 L CNN
F 1 "C_Small" H 7167 1005 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 7075 1050 50  0001 C CNN
F 3 "~" H 7075 1050 50  0001 C CNN
	1    7075 1050
	-1   0    0    1   
$EndComp
$Comp
L Device:C_Small C1
U 1 1 60839267
P 7075 1800
F 0 "C1" H 7167 1846 50  0000 L CNN
F 1 "C_Small" H 7167 1755 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 7075 1800 50  0001 C CNN
F 3 "~" H 7075 1800 50  0001 C CNN
	1    7075 1800
	-1   0    0    1   
$EndComp
Wire Wire Line
	6975 1150 7075 1150
Wire Wire Line
	7075 950  6975 950 
Wire Wire Line
	7075 950  7075 775 
Wire Wire Line
	7075 775  6075 775 
Wire Wire Line
	6075 775  6075 850 
Connection ~ 7075 950 
Wire Wire Line
	6975 1700 7075 1700
Wire Wire Line
	7075 1900 6975 1900
$Comp
L 3Phase-rescue:R-3Phase-rescue R2
U 1 1 6087B6A9
P 7075 2050
F 0 "R2" V 7155 2050 50  0000 C CNN
F 1 "100k" H 7075 2050 50  0001 C CNN
F 2 "Footprints:R_0603_1608Metric" V 7005 2050 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Chip-Resistor-Surface-Mount_100KR-104-5_C100048.html" H 7075 2050 50  0001 C CNN
	1    7075 2050
	1    0    0    -1  
$EndComp
Connection ~ 7075 1900
Wire Wire Line
	5850 1300 5850 775 
Wire Wire Line
	5850 775  6075 775 
Connection ~ 6075 775 
$Comp
L 3Phase-rescue:R-3Phase-rescue R3
U 1 1 608D8CBE
P 7075 1325
F 0 "R3" V 7155 1325 50  0000 C CNN
F 1 "100k" H 7075 1325 50  0001 C CNN
F 2 "Footprints:R_0603_1608Metric" V 7005 1325 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Chip-Resistor-Surface-Mount_100KR-104-5_C100048.html" H 7075 1325 50  0001 C CNN
	1    7075 1325
	1    0    0    -1  
$EndComp
Wire Wire Line
	7075 1175 7075 1150
Connection ~ 7075 1150
Wire Wire Line
	7075 1475 7075 1700
Connection ~ 7075 1700
Wire Wire Line
	6075 1600 6075 1575
Connection ~ 6075 1575
Wire Wire Line
	6075 1575 6075 1550
Wire Wire Line
	6075 2300 7075 2300
Wire Wire Line
	7075 2300 7075 2200
Wire Wire Line
	6075 2300 5850 2300
Wire Wire Line
	5850 2300 5850 1825
Connection ~ 6075 2300
Wire Wire Line
	6075 1575 5950 1575
$Comp
L 3Phase-rescue:R-3Phase-rescue R6
U 1 1 6099A70D
P 5650 2125
F 0 "R6" V 5730 2125 50  0000 C CNN
F 1 "100k" H 5650 2125 50  0001 C CNN
F 2 "Footprints:R_0603_1608Metric" V 5580 2125 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Chip-Resistor-Surface-Mount_100KR-104-5_C100048.html" H 5650 2125 50  0001 C CNN
	1    5650 2125
	-1   0    0    1   
$EndComp
$Comp
L 3Phase-rescue:C_Small-3Phase-rescue C38
U 1 1 60441A0A
P 5525 1400
F 0 "C38" H 5535 1470 50  0000 L CNN
F 1 "100nF 25v" H 5535 1320 50  0001 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 5525 1400 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Multilayer-Ceramic-Capacitors-MLCC-SMD-SMT_SAMSUNG_CL10B104KA8NNNC_100nF-104-10-25V_C1590.html" H 5525 1400 50  0001 C CNN
	1    5525 1400
	-1   0    0    -1  
$EndComp
$Comp
L 3Phase-rescue:C_Small-3Phase-rescue C3
U 1 1 609B71CB
P 5425 1650
F 0 "C3" H 5435 1720 50  0000 L CNN
F 1 "100nF 25v" H 5435 1570 50  0001 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 5425 1650 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Multilayer-Ceramic-Capacitors-MLCC-SMD-SMT_SAMSUNG_CL10B104KA8NNNC_100nF-104-10-25V_C1590.html" H 5425 1650 50  0001 C CNN
	1    5425 1650
	-1   0    0    -1  
$EndComp
Wire Wire Line
	5300 775  5425 775 
Connection ~ 5850 775 
Wire Wire Line
	5650 1875 5525 1875
Wire Wire Line
	5525 1500 5525 1875
Connection ~ 5525 1875
Wire Wire Line
	5525 1875 5200 1875
Wire Wire Line
	5525 1300 5525 775 
Connection ~ 5525 775 
Wire Wire Line
	5525 775  5850 775 
Wire Wire Line
	5200 1975 5425 1975
Wire Wire Line
	5425 1975 5425 1750
Wire Wire Line
	5425 1975 5650 1975
Connection ~ 5425 1975
Wire Wire Line
	5425 1550 5425 775 
Connection ~ 5425 775 
Wire Wire Line
	5425 775  5525 775 
Wire Wire Line
	5300 1775 5300 775 
Wire Wire Line
	5300 1775 5200 1775
Wire Wire Line
	5850 2300 5650 2300
Connection ~ 5850 2300
Wire Wire Line
	5650 2275 5650 2300
Connection ~ 5650 2300
Connection ~ 5300 775 
Wire Wire Line
	5100 1075 5100 1425
Wire Wire Line
	5100 1425 5200 1425
Wire Wire Line
	5200 1425 5200 1675
Wire Wire Line
	4700 1075 4700 1425
Wire Wire Line
	4700 1425 4600 1425
Wire Wire Line
	4600 1425 4600 1675
Connection ~ 4500 775 
Wire Wire Line
	4600 1775 4500 1775
Wire Wire Line
	4950 1075 4850 1075
Wire Wire Line
	5750 1575 5650 1575
$Comp
L 3Phase-rescue:R-3Phase-rescue R7
U 1 1 6073C035
P 2425 1375
F 0 "R7" V 2505 1375 50  0000 C CNN
F 1 "100k" H 2425 1375 50  0001 C CNN
F 2 "Footprints:R_1206_3216Metric" V 2355 1375 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Chip-Resistor-Surface-Mount_100KR-104-5_C100048.html" H 2425 1375 50  0001 C CNN
	1    2425 1375
	0    1    1    0   
$EndComp
$Comp
L 3Phase-rescue:D_Schottky_Small_ALT-3Phase-rescue D2
U 1 1 6073F459
P 3750 1100
F 0 "D2" H 3650 1050 50  0000 C CNN
F 1 "SS14" H 3750 1304 50  0001 C CNN
F 2 "Diode_SMD:D_SMA" H 3750 1213 50  0001 C CNN
F 3 "" V 3750 1100 50  0000 C CNN
	1    3750 1100
	0    -1   -1   0   
$EndComp
$Comp
L 3Phase-rescue:C_Small-3Phase-rescue C6
U 1 1 60740031
P 3625 925
F 0 "C6" H 3635 995 50  0000 L CNN
F 1 "100nF 25v" H 3635 845 50  0001 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 3625 925 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Multilayer-Ceramic-Capacitors-MLCC-SMD-SMT_SAMSUNG_CL10B104KA8NNNC_100nF-104-10-25V_C1590.html" H 3625 925 50  0001 C CNN
	1    3625 925 
	-1   0    0    -1  
$EndComp
$Comp
L 3Phase-rescue:R-3Phase-rescue R9
U 1 1 60741A0F
P 3875 1550
F 0 "R9" V 3955 1550 50  0000 C CNN
F 1 "100k" H 3875 1550 50  0001 C CNN
F 2 "Footprints:R_0603_1608Metric" V 3805 1550 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Chip-Resistor-Surface-Mount_100KR-104-5_C100048.html" H 3875 1550 50  0001 C CNN
	1    3875 1550
	1    0    0    -1  
$EndComp
$Comp
L Device:CP_Small C7
U 1 1 60743158
P 4025 1200
F 0 "C7" H 4050 1300 50  0000 L CNN
F 1 "CP_Small" H 4113 1155 50  0001 L CNN
F 2 "Footprints:CP_Radial_D6.3mm_P2.50mm" H 4025 1200 50  0001 C CNN
F 3 "~" H 4025 1200 50  0001 C CNN
	1    4025 1200
	-1   0    0    1   
$EndComp
$Comp
L 3Phase-rescue:R-3Phase-rescue R8
U 1 1 60740D24
P 3875 950
F 0 "R8" V 3955 950 50  0000 C CNN
F 1 "100k" H 3875 950 50  0001 C CNN
F 2 "Footprints:R_0603_1608Metric" V 3805 950 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Chip-Resistor-Surface-Mount_100KR-104-5_C100048.html" H 3875 950 50  0001 C CNN
	1    3875 950 
	1    0    0    -1  
$EndComp
Wire Wire Line
	4025 1100 4025 775 
Wire Wire Line
	5650 2300 4175 2300
$Comp
L Device:L_Small L1
U 1 1 6073E06C
P 3750 1625
F 0 "L1" H 3798 1671 50  0000 L CNN
F 1 "L_Small" H 3798 1580 50  0001 L CNN
F 2 "Footprints:L_Radial_D6.0mm_P3.00mm" H 3750 1625 50  0001 C CNN
F 3 "~" H 3750 1625 50  0001 C CNN
	1    3750 1625
	-1   0    0    1   
$EndComp
Wire Wire Line
	3750 1525 3750 1475
Wire Wire Line
	3750 1475 3750 1200
Wire Wire Line
	3750 1475 3500 1475
Connection ~ 3750 1475
Wire Wire Line
	3875 1375 3875 1100
Wire Wire Line
	3625 1025 3625 1275
Wire Wire Line
	3625 1275 3500 1275
Wire Wire Line
	3500 1175 3500 775 
Wire Wire Line
	3500 775  3625 775 
Connection ~ 4025 775 
Wire Wire Line
	3625 825  3625 775 
Connection ~ 3625 775 
Wire Wire Line
	3625 775  3750 775 
Wire Wire Line
	3750 1000 3750 775 
Connection ~ 3750 775 
$Comp
L Device:CP_Small C5
U 1 1 607F0664
P 2600 925
F 0 "C5" H 2625 1000 50  0000 L CNN
F 1 "CP_Small" H 2688 880 50  0001 L CNN
F 2 "Footprints:CP_Radial_D6.3mm_P2.50mm" H 2600 925 50  0001 C CNN
F 3 "~" H 2600 925 50  0001 C CNN
	1    2600 925 
	-1   0    0    1   
$EndComp
Wire Wire Line
	2700 1175 2600 1175
Wire Wire Line
	2600 1175 2600 1025
Wire Wire Line
	2700 1275 2600 1275
Wire Wire Line
	2600 1275 2600 1175
Connection ~ 2600 1175
Wire Wire Line
	2700 1375 2600 1375
Wire Wire Line
	2600 1275 2600 1375
Connection ~ 2600 1275
Connection ~ 2600 1375
Wire Wire Line
	2600 1375 2575 1375
Wire Wire Line
	2600 825  2600 775 
Wire Wire Line
	2600 775  3500 775 
Connection ~ 3500 775 
Wire Wire Line
	4175 2300 4175 1775
Wire Wire Line
	3875 1775 3875 1700
Connection ~ 3875 1775
Wire Wire Line
	3875 1775 4025 1775
Wire Wire Line
	3875 1375 3500 1375
Connection ~ 2600 775 
Wire Wire Line
	2275 1375 2250 1375
Connection ~ 2250 1375
Wire Wire Line
	1000 1025 1000 1125
Wire Wire Line
	1000 925  1000 1025
Connection ~ 1000 1025
$Comp
L Connector:Conn_01x10_Male J7
U 1 1 62932ADC
P 800 2125
F 0 "J7" H 908 2506 50  0000 C CNN
F 1 "Conn_01x06_Male" H 908 2415 50  0001 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x10_P2.54mm_Vertical" H 800 2125 50  0001 C CNN
F 3 "~" H 800 2125 50  0001 C CNN
	1    800  2125
	1    0    0    -1  
$EndComp
$Comp
L 3Phase-rescue:C_Small-3Phase-rescue C4
U 1 1 6095FE52
P 2250 925
F 0 "C4" H 2125 850 50  0000 L CNN
F 1 "100nF 25v" H 2260 845 50  0001 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 2250 925 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Multilayer-Ceramic-Capacitors-MLCC-SMD-SMT_SAMSUNG_CL10B104KA8NNNC_100nF-104-10-25V_C1590.html" H 2250 925 50  0001 C CNN
	1    2250 925 
	-1   0    0    -1  
$EndComp
Wire Wire Line
	2250 825  2250 775 
Wire Wire Line
	2250 775  2600 775 
Wire Wire Line
	2250 1375 2250 1025
Text GLabel 1050 1025 2    50   Input ~ 0
+12v
Text GLabel 4375 1925 3    50   Input ~ 0
Vbat
$Comp
L 3Phase-rescue:R-3Phase-rescue R13
U 1 1 60B921EF
P 4175 1550
F 0 "R13" V 4255 1550 50  0000 C CNN
F 1 "100k" H 4175 1550 50  0001 C CNN
F 2 "Footprints:R_0603_1608Metric" V 4105 1550 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Chip-Resistor-Surface-Mount_100KR-104-5_C100048.html" H 4175 1550 50  0001 C CNN
	1    4175 1550
	-1   0    0    1   
$EndComp
$Comp
L 3Phase-rescue:R-3Phase-rescue R14
U 1 1 60B93067
P 4175 950
F 0 "R14" V 4255 950 50  0000 C CNN
F 1 "100k" H 4175 950 50  0001 C CNN
F 2 "Footprints:R_0603_1608Metric" V 4105 950 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Chip-Resistor-Surface-Mount_100KR-104-5_C100048.html" H 4175 950 50  0001 C CNN
	1    4175 950 
	-1   0    0    1   
$EndComp
Wire Wire Line
	3750 1725 3750 1775
Connection ~ 3750 1775
Wire Wire Line
	3750 1775 3875 1775
Text GLabel 1050 2125 2    50   Input ~ 0
ENABLE
$Comp
L 3Phase-rescue:R-3Phase-rescue R11
U 1 1 60AD6151
P 2075 1300
F 0 "R11" V 1975 1300 50  0000 C CNN
F 1 "100k" H 2075 1300 50  0001 C CNN
F 2 "Footprints:R_0603_1608Metric" V 2005 1300 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Chip-Resistor-Surface-Mount_100KR-104-5_C100048.html" H 2075 1300 50  0001 C CNN
	1    2075 1300
	1    0    0    -1  
$EndComp
Text GLabel 2000 1475 0    50   Input ~ 0
+12v
Wire Wire Line
	1000 1025 1050 1025
$Comp
L 3Phase-rescue:R-3Phase-rescue R10
U 1 1 60B120AD
P 2075 950
F 0 "R10" V 1975 950 50  0000 C CNN
F 1 "100k" H 2075 950 50  0001 C CNN
F 2 "Footprints:R_0603_1608Metric" V 2005 950 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Chip-Resistor-Surface-Mount_100KR-104-5_C100048.html" H 2075 950 50  0001 C CNN
	1    2075 950 
	1    0    0    -1  
$EndComp
Connection ~ 2250 1475
Wire Wire Line
	2075 1450 2075 1475
Wire Wire Line
	2075 1475 2250 1475
Wire Wire Line
	2075 1100 2075 1125
Wire Wire Line
	2075 800  2075 775 
Wire Wire Line
	2075 775  2250 775 
Connection ~ 2250 775 
Wire Wire Line
	2250 1775 2400 1775
$Comp
L 3Phase-rescue:D_Schottky_Small_ALT-3Phase-rescue D1
U 1 1 60B3865C
P 2250 1625
F 0 "D1" H 2225 1725 50  0000 C CNN
F 1 "SS14" H 2250 1829 50  0001 C CNN
F 2 "Diode_SMD:D_SMA" H 2250 1738 50  0001 C CNN
F 3 "" V 2250 1625 50  0000 C CNN
	1    2250 1625
	0    -1   -1   0   
$EndComp
Text GLabel 1925 1125 0    50   Input ~ 0
EXT
Wire Wire Line
	1925 1125 2075 1125
Connection ~ 2075 1125
Wire Wire Line
	2075 1125 2075 1150
Wire Wire Line
	3875 1400 3875 1375
Connection ~ 3875 1375
Wire Wire Line
	4025 1300 4025 1775
Connection ~ 4025 1775
Wire Wire Line
	4025 1775 4175 1775
Wire Wire Line
	4175 1700 4175 1775
Connection ~ 4175 1775
Wire Wire Line
	4175 1100 4175 1225
Wire Wire Line
	4175 800  4175 775 
Connection ~ 4175 775 
Wire Wire Line
	4175 775  4025 775 
Wire Wire Line
	3750 775  3875 775 
Wire Wire Line
	3875 800  3875 775 
Connection ~ 3875 775 
Wire Wire Line
	3875 775  4025 775 
Connection ~ 4175 1225
Wire Wire Line
	4175 1225 4175 1400
Wire Wire Line
	4375 1225 4375 1925
Wire Wire Line
	4175 1225 4375 1225
Wire Wire Line
	2000 1475 2075 1475
Connection ~ 2075 1475
Wire Wire Line
	2800 1775 3750 1775
Text GLabel 1050 1825 2    50   Input ~ 0
MOSI
Text GLabel 1050 1925 2    50   Input ~ 0
MCLK
Text GLabel 1050 2025 2    50   Input ~ 0
LCD_BL
Connection ~ 1000 725 
Wire Wire Line
	1000 825  1000 725 
Text GLabel 1050 2225 2    50   Input ~ 0
EXT
Text GLabel 1050 2625 2    50   Input ~ 0
Vbat
Text GLabel 1050 1325 2    50   Input ~ 0
+5v
Wire Wire Line
	1000 1225 1000 1325
Wire Wire Line
	1000 1425 1000 1325
Connection ~ 1000 1325
Wire Wire Line
	1000 1325 1050 1325
Wire Wire Line
	1000 725  1050 725 
$Comp
L power:GND #PWR0103
U 1 1 60B2E019
P 1050 725
F 0 "#PWR0103" H 1050 475 50  0001 C CNN
F 1 "GND" H 1055 552 50  0000 C CNN
F 2 "" H 1050 725 50  0001 C CNN
F 3 "" H 1050 725 50  0001 C CNN
	1    1050 725 
	0    -1   -1   0   
$EndComp
Text GLabel 1050 1525 2    50   Input ~ 0
Vrtc
Text GLabel 1050 2325 2    50   Input ~ 0
BTN1
Text GLabel 1050 2425 2    50   Input ~ 0
BTN2
Text GLabel 1050 2525 2    50   Input ~ 0
BEEP
$Comp
L Transistor_FET:2N7002 Q5
U 1 1 60A118A9
P 6075 3025
F 0 "Q5" H 6280 3071 50  0000 L CNN
F 1 "2N7002" H 6280 2980 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 6275 2950 50  0001 L CIN
F 3 "https://www.onsemi.com/pub/Collateral/NDS7002A-D.PDF" H 6075 3025 50  0001 L CNN
	1    6075 3025
	-1   0    0    -1  
$EndComp
Wire Wire Line
	1000 1825 1050 1825
Wire Wire Line
	1050 1925 1000 1925
Wire Wire Line
	1000 2025 1050 2025
Wire Wire Line
	1050 2125 1000 2125
Wire Wire Line
	1000 2225 1050 2225
Wire Wire Line
	1050 2325 1000 2325
Wire Wire Line
	1000 2425 1050 2425
Wire Wire Line
	1000 2525 1050 2525
Wire Wire Line
	1000 2625 1050 2625
$Comp
L Connector:Conn_01x12_Female J1
U 1 1 608DA3B1
P 800 3525
F 0 "J1" H 828 3501 50  0000 L CNN
F 1 "Conn_01x12_Female" H 828 3410 50  0000 L CNN
F 2 "Connector_FFC-FPC:Hirose_FH12-12S-0.5SH_1x12-1MP_P0.50mm_Horizontal" H 800 3525 50  0001 C CNN
F 3 "~" H 800 3525 50  0001 C CNN
	1    800  3525
	-1   0    0    1   
$EndComp
Text GLabel 2150 2300 0    50   Input ~ 0
ENABLE
Wire Wire Line
	2700 2200 2350 2200
Wire Wire Line
	3700 3500 3700 3475
Wire Wire Line
	4000 2325 4000 1925
Connection ~ 3700 3475
Wire Wire Line
	3700 2775 3700 3475
Wire Wire Line
	3750 1925 3750 1975
$Comp
L Device:CP_Small C15
U 1 1 6096E724
P 4000 2425
F 0 "C15" H 3850 2350 50  0000 L CNN
F 1 "CP_Small" H 4088 2380 50  0001 L CNN
F 2 "Footprints:CP_Radial_D6.3mm_P2.50mm" H 4000 2425 50  0001 C CNN
F 3 "~" H 4000 2425 50  0001 C CNN
	1    4000 2425
	-1   0    0    -1  
$EndComp
Wire Wire Line
	2250 2725 2250 3475
$Comp
L 3Phase-rescue:C_Small-3Phase-rescue C12
U 1 1 6093AA66
P 2250 2625
F 0 "C12" H 2260 2695 50  0000 L CNN
F 1 "100nF 25v" H 2260 2545 50  0001 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 2250 2625 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Multilayer-Ceramic-Capacitors-MLCC-SMD-SMT_SAMSUNG_CL10B104KA8NNNC_100nF-104-10-25V_C1590.html" H 2250 2625 50  0001 C CNN
	1    2250 2625
	-1   0    0    1   
$EndComp
Wire Wire Line
	3875 1925 4000 1925
Connection ~ 3875 1925
Wire Wire Line
	3875 1925 3875 1975
Wire Wire Line
	3750 1925 3875 1925
Wire Wire Line
	2475 3075 2475 3475
$Comp
L Device:CP_Small C13
U 1 1 6093AA26
P 2350 3225
F 0 "C13" H 2375 3150 50  0000 L CNN
F 1 "CP_Small" H 2438 3180 50  0001 L CNN
F 2 "Footprints:CP_Radial_D6.3mm_P2.50mm" H 2350 3225 50  0001 C CNN
F 3 "~" H 2350 3225 50  0001 C CNN
	1    2350 3225
	-1   0    0    -1  
$EndComp
Connection ~ 3550 3475
Wire Wire Line
	3550 3050 3550 3475
$Comp
L Device:L_Small L2
U 1 1 6093AA0A
P 3750 2075
F 0 "L2" H 3798 2121 50  0000 L CNN
F 1 "L_Small" H 3798 2030 50  0001 L CNN
F 2 "Inductor_SMD:L_Bourns_SRN6045TA" H 3750 2075 50  0001 C CNN
F 3 "~" H 3750 2075 50  0001 C CNN
	1    3750 2075
	-1   0    0    -1  
$EndComp
$Comp
L 3Phase-rescue:R-3Phase-rescue R19
U 1 1 6093AA00
P 3875 2625
F 0 "R19" V 3955 2625 50  0000 C CNN
F 1 "100k" H 3875 2625 50  0001 C CNN
F 2 "Footprints:R_0603_1608Metric" V 3805 2625 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Chip-Resistor-Surface-Mount_100KR-104-5_C100048.html" H 3875 2625 50  0001 C CNN
	1    3875 2625
	1    0    0    1   
$EndComp
$Comp
L 3Phase-rescue:R-3Phase-rescue R18
U 1 1 6093A9F6
P 3875 2125
F 0 "R18" V 3955 2125 50  0000 C CNN
F 1 "100k" H 3875 2125 50  0001 C CNN
F 2 "Footprints:R_0603_1608Metric" V 3805 2125 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Chip-Resistor-Surface-Mount_100KR-104-5_C100048.html" H 3875 2125 50  0001 C CNN
	1    3875 2125
	1    0    0    1   
$EndComp
$Comp
L 3Phase-rescue:C_Small-3Phase-rescue C14
U 1 1 6093A9EC
P 3700 2675
F 0 "C14" V 3650 2750 50  0000 L CNN
F 1 "100nF 25v" H 3710 2595 50  0001 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 3700 2675 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Multilayer-Ceramic-Capacitors-MLCC-SMD-SMT_SAMSUNG_CL10B104KA8NNNC_100nF-104-10-25V_C1590.html" H 3700 2675 50  0001 C CNN
	1    3700 2675
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR0105
U 1 1 6093A4AA
P 3700 3500
F 0 "#PWR0105" H 3700 3250 50  0001 C CNN
F 1 "GND" H 3705 3327 50  0000 C CNN
F 2 "" H 3700 3500 50  0001 C CNN
F 3 "" H 3700 3500 50  0001 C CNN
	1    3700 3500
	-1   0    0    -1  
$EndComp
Text GLabel 3525 1925 0    50   Input ~ 0
+5v
Wire Wire Line
	3525 1925 3750 1925
Connection ~ 3750 1925
$Comp
L Regulator_Linear:LM2936-3.3 U6
U 1 1 610920CB
P 4500 2450
F 0 "U6" H 4500 2692 50  0000 C CNN
F 1 "LM2936-3.3" H 4500 2601 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 4500 2675 50  0001 C CIN
F 3 "http://www.ti.com/lit/ds/symlink/lm2936.pdf" H 4500 2400 50  0001 C CNN
	1    4500 2450
	1    0    0    -1  
$EndComp
$Comp
L 3Phase-rescue:C_Small-3Phase-rescue C16
U 1 1 61094DE8
P 4825 3025
F 0 "C16" H 4835 3095 50  0000 L CNN
F 1 "100nF 25v" H 4835 2945 50  0001 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 4825 3025 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Multilayer-Ceramic-Capacitors-MLCC-SMD-SMT_SAMSUNG_CL10B104KA8NNNC_100nF-104-10-25V_C1590.html" H 4825 3025 50  0001 C CNN
	1    4825 3025
	-1   0    0    1   
$EndComp
Wire Wire Line
	4500 2750 4500 3475
Connection ~ 4500 3475
Wire Wire Line
	4500 3475 4825 3475
Wire Wire Line
	4825 3125 4825 3475
Wire Wire Line
	4800 2450 4825 2450
Wire Wire Line
	4825 2450 4825 2925
Wire Wire Line
	4200 2450 4175 2450
Wire Wire Line
	4175 2450 4175 2300
Connection ~ 4175 2300
$Comp
L 3Phase-rescue:C_Small-3Phase-rescue C9
U 1 1 60A9804E
P 5125 3350
F 0 "C9" H 5135 3420 50  0000 L CNN
F 1 "100nF 25v" H 5135 3270 50  0001 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 5125 3350 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Multilayer-Ceramic-Capacitors-MLCC-SMD-SMT_SAMSUNG_CL10B104KA8NNNC_100nF-104-10-25V_C1590.html" H 5125 3350 50  0001 C CNN
	1    5125 3350
	-1   0    0    -1  
$EndComp
$Comp
L 3Phase-rescue:R-3Phase-rescue R12
U 1 1 60A96CB5
P 5125 3050
F 0 "R12" V 5205 3050 50  0000 C CNN
F 1 "100k" H 5125 3050 50  0001 C CNN
F 2 "Footprints:R_0603_1608Metric" V 5055 3050 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Chip-Resistor-Surface-Mount_100KR-104-5_C100048.html" H 5125 3050 50  0001 C CNN
	1    5125 3050
	1    0    0    1   
$EndComp
$Comp
L 3Phase-rescue:SW_Push-3Phase-rescue SW1
U 1 1 5FE4F8B5
P 5550 2675
F 0 "SW1" V 5641 2627 50  0000 R CNN
F 1 "SW_Push-3Phase-rescue" V 5550 2627 50  0001 R CNN
F 2 "Footprints:SW_Push_6x6mm_H8mm" V 5459 2627 50  0001 R CNN
F 3 "" H 5550 2875 50  0000 C CNN
	1    5550 2675
	0    -1   1    0   
$EndComp
$Comp
L 3Phase-rescue:SW_Push-3Phase-rescue SW2
U 1 1 609B2370
P 5125 2675
F 0 "SW2" V 5216 2627 50  0000 R CNN
F 1 "SW_Push-3Phase-rescue" V 5125 2627 50  0001 R CNN
F 2 "Footprints:SW_Push_6x6mm_H8mm" V 5034 2627 50  0001 R CNN
F 3 "" H 5125 2875 50  0000 C CNN
	1    5125 2675
	0    -1   1    0   
$EndComp
Text GLabel 5625 2450 2    50   Input ~ 0
Vrtc
$Comp
L 3Phase-rescue:C_Small-3Phase-rescue C8
U 1 1 60DFD8CD
P 5550 3350
F 0 "C8" H 5560 3420 50  0000 L CNN
F 1 "100nF 25v" H 5560 3270 50  0001 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 5550 3350 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Multilayer-Ceramic-Capacitors-MLCC-SMD-SMT_SAMSUNG_CL10B104KA8NNNC_100nF-104-10-25V_C1590.html" H 5550 3350 50  0001 C CNN
	1    5550 3350
	-1   0    0    -1  
$EndComp
$Comp
L 3Phase-rescue:R-3Phase-rescue R1
U 1 1 5FE634D2
P 5550 3050
F 0 "R1" V 5630 3050 50  0000 C CNN
F 1 "100k" H 5550 3050 50  0001 C CNN
F 2 "Footprints:R_0603_1608Metric" V 5480 3050 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Chip-Resistor-Surface-Mount_100KR-104-5_C100048.html" H 5550 3050 50  0001 C CNN
	1    5550 3050
	1    0    0    1   
$EndComp
Text GLabel 5175 3225 2    50   Input ~ 0
BTN1
Wire Wire Line
	4825 2450 5125 2450
Connection ~ 4825 2450
Wire Wire Line
	5125 2475 5125 2450
Connection ~ 5125 2450
Wire Wire Line
	5125 2450 5550 2450
Wire Wire Line
	5125 2900 5125 2875
Wire Wire Line
	5125 3250 5125 3225
Wire Wire Line
	5175 3225 5125 3225
Connection ~ 5125 3225
Wire Wire Line
	5125 3225 5125 3200
Wire Wire Line
	4825 3475 5125 3475
Wire Wire Line
	5125 3475 5125 3450
Connection ~ 4825 3475
Wire Wire Line
	5125 3475 5550 3475
Wire Wire Line
	5550 3475 5550 3450
Connection ~ 5125 3475
Wire Wire Line
	5550 3250 5550 3225
Wire Wire Line
	5550 2900 5550 2875
Wire Wire Line
	5550 2475 5550 2450
Connection ~ 5550 2450
Wire Wire Line
	5550 2450 5625 2450
Text GLabel 5600 3225 2    50   Input ~ 0
BTN2
Wire Wire Line
	5600 3225 5550 3225
Connection ~ 5550 3225
Wire Wire Line
	5550 3225 5550 3200
Wire Wire Line
	5550 3475 5975 3475
Wire Wire Line
	5975 3475 5975 3225
Connection ~ 5550 3475
Text GLabel 6350 3025 2    50   Input ~ 0
BEEP
Wire Wire Line
	6275 3025 6350 3025
Wire Wire Line
	5975 2650 5975 2825
Text GLabel 6350 2875 2    50   Input ~ 0
+5v
Wire Wire Line
	6350 2875 6175 2875
Wire Wire Line
	6175 2875 6175 2650
$Comp
L 3Phase-rescue:C_Small-3Phase-rescue C10
U 1 1 6097B2D2
P 1375 3000
F 0 "C10" H 1385 3070 50  0000 L CNN
F 1 "100nF 25v" H 1385 2920 50  0001 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 1375 3000 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Multilayer-Ceramic-Capacitors-MLCC-SMD-SMT_SAMSUNG_CL10B104KA8NNNC_100nF-104-10-25V_C1590.html" H 1375 3000 50  0001 C CNN
	1    1375 3000
	-1   0    0    1   
$EndComp
$Comp
L 3Phase-rescue:C_Small-3Phase-rescue C11
U 1 1 6097C461
P 1375 3350
F 0 "C11" H 1385 3420 50  0000 L CNN
F 1 "100nF 25v" H 1385 3270 50  0001 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 1375 3350 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Multilayer-Ceramic-Capacitors-MLCC-SMD-SMT_SAMSUNG_CL10B104KA8NNNC_100nF-104-10-25V_C1590.html" H 1375 3350 50  0001 C CNN
	1    1375 3350
	-1   0    0    1   
$EndComp
Wire Wire Line
	1000 725  1000 625 
Wire Wire Line
	1050 1525 1000 1525
Text GLabel 1050 1725 2    50   Input ~ 0
LCD_EN
Wire Wire Line
	1000 1725 1050 1725
$Comp
L Connector:Conn_01x10_Male J8
U 1 1 604239FD
P 800 1025
F 0 "J8" H 772 907 50  0000 R CNN
F 1 "Conn_01x06_Male" H 772 998 50  0001 R CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x10_P2.54mm_Vertical" H 800 1025 50  0001 C CNN
F 3 "~" H 800 1025 50  0001 C CNN
	1    800  1025
	1    0    0    -1  
$EndComp
$Comp
L 3Phase-rescue:R-3Phase-rescue R4
U 1 1 6082E83A
P 6075 2150
F 0 "R4" V 6155 2150 50  0000 C CNN
F 1 "100k" H 6075 2150 50  0001 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 6005 2150 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Chip-Resistor-Surface-Mount_100KR-104-5_C100048.html" H 6075 2150 50  0001 C CNN
	1    6075 2150
	1    0    0    -1  
$EndComp
$Comp
L 3Phase-rescue:R-3Phase-rescue R5
U 1 1 608E33E2
P 6075 1400
F 0 "R5" V 6155 1400 50  0000 C CNN
F 1 "100k" H 6075 1400 50  0001 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 6005 1400 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Chip-Resistor-Surface-Mount_100KR-104-5_C100048.html" H 6075 1400 50  0001 C CNN
	1    6075 1400
	1    0    0    -1  
$EndComp
$Comp
L Components:AP64350 U5
U 1 1 60C19147
P 3100 2175
F 0 "U5" H 3100 2392 50  0000 C CNN
F 1 "AP64350" H 3100 2301 50  0000 C CNN
F 2 "Package_SO:SOIC-8-1EP_3.9x4.9mm_P1.27mm_EP2.41x3.3mm_ThermalVias" H 3150 1725 50  0001 L CNN
F 3 "http://www.onsemi.com/pub_link/Collateral/MC34063A-D.PDF" H 3600 2075 50  0001 C CNN
	1    3100 2175
	1    0    0    -1  
$EndComp
Wire Wire Line
	2250 2200 2250 2525
Wire Wire Line
	2700 2500 2700 3475
$Comp
L 3Phase-rescue:R-3Phase-rescue R15
U 1 1 60C70FF9
P 2575 2625
F 0 "R15" V 2655 2625 50  0000 C CNN
F 1 "100k" H 2575 2625 50  0001 C CNN
F 2 "Footprints:R_0603_1608Metric" V 2505 2625 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Chip-Resistor-Surface-Mount_100KR-104-5_C100048.html" H 2575 2625 50  0001 C CNN
	1    2575 2625
	1    0    0    1   
$EndComp
Wire Wire Line
	2700 2400 2575 2400
Wire Wire Line
	2575 2400 2575 2475
Wire Wire Line
	2700 2300 2475 2300
Wire Wire Line
	2475 2775 2475 2300
Connection ~ 2475 2300
Wire Wire Line
	2475 2300 2150 2300
$Comp
L 3Phase-rescue:C_Small-3Phase-rescue C18
U 1 1 60CE5378
P 3625 2200
F 0 "C18" H 3635 2270 50  0000 L CNN
F 1 "100nF 25v" H 3635 2120 50  0001 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 3625 2200 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Multilayer-Ceramic-Capacitors-MLCC-SMD-SMT_SAMSUNG_CL10B104KA8NNNC_100nF-104-10-25V_C1590.html" H 3625 2200 50  0001 C CNN
	1    3625 2200
	0    -1   -1   0   
$EndComp
Wire Wire Line
	3875 2275 3875 2400
Wire Wire Line
	3550 3475 3700 3475
Wire Wire Line
	3500 2300 3750 2300
Wire Wire Line
	3750 2300 3750 2200
Wire Wire Line
	3500 2400 3875 2400
Connection ~ 3875 2400
Wire Wire Line
	3875 2400 3875 2475
Wire Wire Line
	3725 2200 3750 2200
Connection ~ 3750 2200
Wire Wire Line
	3750 2200 3750 2175
Wire Wire Line
	3525 2200 3500 2200
Wire Wire Line
	2250 1375 2250 1475
Wire Wire Line
	3700 3475 3875 3475
Wire Wire Line
	2250 3475 2350 3475
Wire Wire Line
	2250 1525 2250 1475
Wire Wire Line
	2250 1725 2250 1775
Wire Wire Line
	2250 1775 2250 2200
Connection ~ 2250 1775
Connection ~ 2250 2200
$Comp
L 3Phase-rescue:R-3Phase-rescue R16
U 1 1 60E97FEF
P 3550 2675
F 0 "R16" V 3475 2675 50  0000 C CNN
F 1 "100k" H 3550 2675 50  0001 C CNN
F 2 "Footprints:R_0603_1608Metric" V 3480 2675 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Chip-Resistor-Surface-Mount_100KR-104-5_C100048.html" H 3550 2675 50  0001 C CNN
	1    3550 2675
	1    0    0    1   
$EndComp
$Comp
L 3Phase-rescue:C_Small-3Phase-rescue C17
U 1 1 60E9EF23
P 3550 2950
F 0 "C17" V 3500 3025 50  0000 L CNN
F 1 "100nF 25v" H 3560 2870 50  0001 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 3550 2950 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Multilayer-Ceramic-Capacitors-MLCC-SMD-SMT_SAMSUNG_CL10B104KA8NNNC_100nF-104-10-25V_C1590.html" H 3550 2950 50  0001 C CNN
	1    3550 2950
	-1   0    0    1   
$EndComp
Wire Wire Line
	3875 2775 3875 3475
Connection ~ 3875 3475
Wire Wire Line
	3875 3475 4000 3475
Wire Wire Line
	4000 2525 4000 3475
Connection ~ 4000 3475
Wire Wire Line
	4000 3475 4500 3475
Wire Wire Line
	3700 2575 3700 2500
Wire Wire Line
	3700 2500 3550 2500
Wire Wire Line
	3550 2525 3550 2500
Connection ~ 3550 2500
Wire Wire Line
	3550 2500 3500 2500
Wire Wire Line
	3550 2825 3550 2850
Connection ~ 2700 3475
Wire Wire Line
	2700 3475 3100 3475
Wire Wire Line
	2575 2775 2575 3475
Connection ~ 2575 3475
Wire Wire Line
	2575 3475 2700 3475
Connection ~ 2475 3475
Wire Wire Line
	2475 3475 2575 3475
Wire Wire Line
	2350 3125 2350 2200
Connection ~ 2350 2200
Wire Wire Line
	2350 2200 2250 2200
Wire Wire Line
	2350 3325 2350 3475
Connection ~ 2350 3475
Wire Wire Line
	2350 3475 2475 3475
Wire Wire Line
	3100 2675 3100 3475
Connection ~ 3100 3475
Wire Wire Line
	3100 3475 3550 3475
$Comp
L Transistor_FET:BS250 Q3
U 1 1 610263CF
P 2600 1675
F 0 "Q3" V 2849 1675 50  0000 C CNN
F 1 "NCE40P05Y" V 2940 1675 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 2800 1600 50  0001 L CIN
F 3 "http://www.vishay.com/docs/70209/70209.pdf" H 2600 1675 50  0001 L CNN
	1    2600 1675
	0    1    1    0   
$EndComp
Wire Wire Line
	2250 1475 2600 1475
Connection ~ 2600 1475
Wire Wire Line
	2600 1475 2700 1475
$Comp
L 3Phase-rescue:R-3Phase-rescue R17
U 1 1 61055A28
P 2475 2925
F 0 "R17" V 2555 2925 50  0000 C CNN
F 1 "100k" H 2475 2925 50  0001 C CNN
F 2 "Footprints:R_0603_1608Metric" V 2405 2925 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Chip-Resistor-Surface-Mount_100KR-104-5_C100048.html" H 2475 2925 50  0001 C CNN
	1    2475 2925
	1    0    0    1   
$EndComp
$EndSCHEMATC
