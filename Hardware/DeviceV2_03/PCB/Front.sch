EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr User 7874 4724
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
P 3875 1275
F 0 "U17" H 3550 827 50  0000 C CNN
F 1 "8205S" H 3550 736 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-23-6" H 3875 1275 50  0001 C CNN
F 3 "" H 3875 1275 50  0001 C CNN
	1    3875 1275
	-1   0    0    1   
$EndComp
$Comp
L 3Phase-rescue:R-3Phase-rescue R69
U 1 1 6043BE3B
P 3800 1575
F 0 "R69" V 3880 1575 50  0000 C CNN
F 1 "100k" H 3800 1575 50  0001 C CNN
F 2 "Footprints:R_0603_1608Metric" V 3730 1575 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Chip-Resistor-Surface-Mount_100KR-104-5_C100048.html" H 3800 1575 50  0001 C CNN
	1    3800 1575
	1    0    0    -1  
$EndComp
Wire Wire Line
	3800 2000 3800 1725
Wire Wire Line
	3800 1000 3800 1425
$Comp
L 3Phase-rescue:R-3Phase-rescue R68
U 1 1 6043CE8A
P 4950 1950
F 0 "R68" V 5030 1950 50  0000 C CNN
F 1 "100k" H 4950 1950 50  0001 C CNN
F 2 "Footprints:R_0603_1608Metric" V 4880 1950 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Chip-Resistor-Surface-Mount_100KR-104-5_C100048.html" H 4950 1950 50  0001 C CNN
	1    4950 1950
	-1   0    0    1   
$EndComp
Wire Wire Line
	3800 1000 3675 1000
$Comp
L Transistor_FET:2N7002 Q4
U 1 1 628A337B
P 2325 2775
F 0 "Q4" H 2475 2900 50  0000 L CNN
F 1 "2N7002" V 2550 2625 50  0001 L CNN
F 2 "Footprints:SOT-23" H 2525 2700 50  0001 L CIN
F 3 "https://lcsc.com/product-detail/MOSFET_Guangdong-Hottech-2N7002_C181082.html" H 2325 2775 50  0001 L CNN
	1    2325 2775
	1    0    0    1   
$EndComp
$Comp
L Connector:Conn_01x08_Male J7
U 1 1 62932ADC
P 1425 2625
F 0 "J7" H 1533 3006 50  0000 C CNN
F 1 "Conn_01x06_Male" H 1533 2915 50  0001 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x07_P2.54mm_Vertical" H 1425 2625 50  0001 C CNN
F 3 "~" H 1425 2625 50  0001 C CNN
	1    1425 2625
	1    0    0    1   
$EndComp
$Comp
L 3Phase-rescue:LED_ALT-3Phase-rescue D16
U 1 1 62941D3A
P 2250 2425
F 0 "D16" H 2250 2525 50  0000 C CNN
F 1 "3mm blue LED" H 2250 2325 50  0001 C CNN
F 2 "LED_SMD:LED_0603_1608Metric" H 2250 2425 50  0001 C CNN
F 3 "https://es.aliexpress.com/store/product/100PCS-x-Long-Leg-3mm-5mm-White-Diffused-Jade-Green-Yellow-Red-Blue-Ultra-Bright-LED/102034_32887271791.html" H 2250 2425 50  0001 C CNN
	1    2250 2425
	-1   0    0    -1  
$EndComp
$Comp
L 3Phase-rescue:R-3Phase-rescue R67
U 1 1 62A0039E
P 1900 2325
F 0 "R67" V 1980 2325 50  0000 C CNN
F 1 "100k" H 1900 2325 50  0001 C CNN
F 2 "Footprints:R_0603_1608Metric" V 1830 2325 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Chip-Resistor-Surface-Mount_100KR-104-5_C100048.html" H 1900 2325 50  0001 C CNN
	1    1900 2325
	0    -1   1    0   
$EndComp
$Comp
L 3Phase-rescue:R-3Phase-rescue R61
U 1 1 62E59093
P 1900 2425
F 0 "R61" V 1980 2425 50  0000 C CNN
F 1 "100k" H 1900 2425 50  0001 C CNN
F 2 "Footprints:R_0603_1608Metric" V 1830 2425 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Chip-Resistor-Surface-Mount_100KR-104-5_C100048.html" H 1900 2425 50  0001 C CNN
	1    1900 2425
	0    1    -1   0   
$EndComp
$Comp
L 3Phase-rescue:R-3Phase-rescue R60
U 1 1 62E59B6F
P 1900 2525
F 0 "R60" V 1980 2525 50  0000 C CNN
F 1 "100k" H 1900 2525 50  0001 C CNN
F 2 "Footprints:R_0603_1608Metric" V 1830 2525 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Chip-Resistor-Surface-Mount_100KR-104-5_C100048.html" H 1900 2525 50  0001 C CNN
	1    1900 2525
	0    1    -1   0   
$EndComp
Wire Wire Line
	2050 2525 2100 2525
Wire Wire Line
	2050 2425 2100 2425
Wire Wire Line
	1625 2425 1750 2425
$Comp
L 3Phase-rescue:LED_ALT-3Phase-rescue D18
U 1 1 645CE090
P 2250 2325
F 0 "D18" H 2250 2425 50  0000 C CNN
F 1 "3mm blue LED" H 2250 2225 50  0001 C CNN
F 2 "LED_SMD:LED_0603_1608Metric" H 2250 2325 50  0001 C CNN
F 3 "https://es.aliexpress.com/store/product/100PCS-x-Long-Leg-3mm-5mm-White-Diffused-Jade-Green-Yellow-Red-Blue-Ultra-Bright-LED/102034_32887271791.html" H 2250 2325 50  0001 C CNN
	1    2250 2325
	-1   0    0    -1  
$EndComp
Wire Wire Line
	1625 2225 2425 2225
$Comp
L 3Phase-rescue:LED_ALT-3Phase-rescue D7
U 1 1 62943BE2
P 2250 2525
F 0 "D7" H 2250 2625 50  0000 C CNN
F 1 "3mm blue LED" H 2250 2425 50  0001 C CNN
F 2 "LED_SMD:LED_0603_1608Metric" H 2250 2525 50  0001 C CNN
F 3 "https://es.aliexpress.com/store/product/100PCS-x-Long-Leg-3mm-5mm-White-Diffused-Jade-Green-Yellow-Red-Blue-Ultra-Bright-LED/102034_32887271791.html" H 2250 2525 50  0001 C CNN
	1    2250 2525
	-1   0    0    -1  
$EndComp
Wire Wire Line
	2400 2525 2425 2525
Wire Wire Line
	2425 2525 2425 2425
Wire Wire Line
	2400 2425 2425 2425
Connection ~ 2425 2425
Wire Wire Line
	2425 2425 2425 2325
Wire Wire Line
	2400 2325 2425 2325
Connection ~ 2425 2325
Wire Wire Line
	2425 2325 2425 2225
Wire Wire Line
	1750 2525 1625 2525
Wire Wire Line
	1625 2325 1750 2325
Wire Wire Line
	2050 2325 2100 2325
Wire Wire Line
	2300 2975 2425 2975
$Comp
L power:GND #PWR0101
U 1 1 5FF0D2D3
P 3625 950
F 0 "#PWR0101" H 3625 700 50  0001 C CNN
F 1 "GND" H 3630 777 50  0000 C CNN
F 2 "" H 3625 950 50  0001 C CNN
F 3 "" H 3625 950 50  0001 C CNN
	1    3625 950 
	-1   0    0    1   
$EndComp
Wire Wire Line
	3625 950  3625 1000
$Comp
L 3Phase-rescue:SW_Push-3Phase-rescue SW1
U 1 1 5FE4F8B5
P 1725 3200
F 0 "SW1" V 1816 3152 50  0000 R CNN
F 1 "SW_Push-3Phase-rescue" V 1725 3152 50  0001 R CNN
F 2 "Button_Switch_SMD:SW_Push_1P1T_NO_6x6mm_H9.5mm" V 1634 3152 50  0001 R CNN
F 3 "" H 1725 3400 50  0000 C CNN
	1    1725 3200
	0    -1   1    0   
$EndComp
Wire Wire Line
	1875 3425 1725 3425
Wire Wire Line
	1725 3425 1725 3400
$Comp
L 3Phase-rescue:R-3Phase-rescue R1
U 1 1 5FE634D2
P 1875 3025
F 0 "R1" V 1955 3025 50  0000 C CNN
F 1 "100k" H 1875 3025 50  0001 C CNN
F 2 "Footprints:R_0603_1608Metric" V 1805 3025 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Chip-Resistor-Surface-Mount_100KR-104-5_C100048.html" H 1875 3025 50  0001 C CNN
	1    1875 3025
	-1   0    0    -1  
$EndComp
Wire Wire Line
	1875 3175 1875 3425
$Comp
L Components:MC34063AD U4
U 1 1 6072003E
P 2750 1625
F 0 "U4" H 2750 2092 50  0000 C CNN
F 1 "MC34063AD" H 2750 2001 50  0000 C CNN
F 2 "Package_SO:SOIC-8_3.9x4.9mm_P1.27mm" H 2800 1175 50  0001 L CNN
F 3 "http://www.onsemi.com/pub_link/Collateral/MC34063A-D.PDF" H 3250 1525 50  0001 C CNN
	1    2750 1625
	1    0    0    1   
$EndComp
$Comp
L Components:HY2120CB U3
U 1 1 60723F31
P 4275 2175
F 0 "U3" H 4350 2442 50  0000 C CNN
F 1 "HY2120CB" H 4350 2351 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-23-6" H 4275 2175 50  0001 C CNN
F 3 "" H 4275 2175 50  0001 C CNN
	1    4275 2175
	-1   0    0    1   
$EndComp
$Comp
L Components:HY2213-BB3A U1
U 1 1 60724CA2
P 6050 2200
F 0 "U1" H 6125 2467 50  0000 C CNN
F 1 "HY2213-BB3A" H 6175 2250 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-23-6" H 6050 2200 50  0001 C CNN
F 3 "" H 6050 2200 50  0001 C CNN
	1    6050 2200
	-1   0    0    1   
$EndComp
$Comp
L Components:HY2213-BB3A U2
U 1 1 607257E8
P 6050 1450
F 0 "U2" H 6125 1717 50  0000 C CNN
F 1 "HY2213-BB3A" H 6175 1500 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-23-6" H 6050 1450 50  0001 C CNN
F 3 "" H 6050 1450 50  0001 C CNN
	1    6050 1450
	-1   0    0    1   
$EndComp
Wire Wire Line
	2425 3100 2425 2975
Wire Wire Line
	1625 2725 2025 2725
Wire Wire Line
	1625 2925 1725 2925
Wire Wire Line
	1725 2925 1725 3000
Wire Wire Line
	1625 2825 1875 2825
Wire Wire Line
	1875 2825 1875 2875
Wire Wire Line
	2225 3175 2225 3100
Wire Wire Line
	2025 2725 2025 2975
$Comp
L 3Phase-rescue:Buzzer-3Phase-rescue BZ2
U 1 1 628A53C3
P 2125 3275
F 0 "BZ2" V 2000 3200 50  0000 L CNN
F 1 "QMB-09B-05" H 2275 3225 50  0001 L CNN
F 2 "Footprints:Buzzer_9x5.5RM4" V 2100 3375 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Buzzers_Jiangsu-Huaneng-Elec-QMB-09B-05_C161819.html" V 2100 3375 50  0001 C CNN
	1    2125 3275
	0    -1   1    0   
$EndComp
Wire Wire Line
	2425 2575 2425 2525
Connection ~ 2425 2525
Wire Wire Line
	1625 2625 2100 2625
Wire Wire Line
	2100 2625 2100 2775
Wire Wire Line
	2100 2775 2125 2775
Wire Wire Line
	2225 3100 2425 3100
Connection ~ 2425 2975
$Comp
L 3Phase-rescue:D_Small_ALT-3Phase-rescue D15
U 1 1 6569F29D
P 2200 2975
F 0 "D15" H 2150 3055 50  0000 L CNN
F 1 "ES1J" H 2050 2895 50  0001 L CNN
F 2 "Footprints:D_SOD-323" V 2200 2975 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Diodes-General-Purpose_Bourne-Semicon-Shenzhen-ES1J_C266561.html" V 2200 2975 50  0001 C CNN
	1    2200 2975
	1    0    0    1   
$EndComp
Wire Wire Line
	2100 2975 2025 2975
Connection ~ 2025 2975
Wire Wire Line
	2025 2975 2025 3175
$Comp
L Components:Battery_Cell BT1
U 1 1 607351E4
P 5150 1625
F 0 "BT1" H 5006 1834 50  0000 R CNN
F 1 "Battery_Cell" H 5006 1743 50  0000 R CNN
F 2 "Footprints:BatteryHolder_Keystone_1042_1x18650" V 5150 1685 50  0001 C CNN
F 3 "~" V 5150 1685 50  0001 C CNN
	1    5150 1625
	1    0    0    1   
$EndComp
$Comp
L Transistor_FET:DMG2302U Q1
U 1 1 60818296
P 5475 2025
F 0 "Q1" H 5400 1875 50  0000 L CNN
F 1 "DMG2302U" H 5679 1980 50  0001 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 5675 1950 50  0001 L CIN
F 3 "http://www.diodes.com/assets/Datasheets/DMG2302U.pdf" H 5475 2025 50  0001 L CNN
	1    5475 2025
	-1   0    0    1   
$EndComp
$Comp
L Transistor_FET:DMG2302U Q2
U 1 1 60822CF3
P 5475 1275
F 0 "Q2" H 5375 1150 50  0000 L CNN
F 1 "DMG2302U" H 5679 1230 50  0001 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 5675 1200 50  0001 L CIN
F 3 "http://www.diodes.com/assets/Datasheets/DMG2302U.pdf" H 5475 1275 50  0001 L CNN
	1    5475 1275
	-1   0    0    1   
$EndComp
$Comp
L 3Phase-rescue:R-3Phase-rescue R4
U 1 1 6082E83A
P 5375 2375
F 0 "R4" V 5455 2375 50  0000 C CNN
F 1 "100k" H 5375 2375 50  0001 C CNN
F 2 "Footprints:R_0603_1608Metric" V 5305 2375 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Chip-Resistor-Surface-Mount_100KR-104-5_C100048.html" H 5375 2375 50  0001 C CNN
	1    5375 2375
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C2
U 1 1 60838577
P 6375 1275
F 0 "C2" H 6467 1321 50  0000 L CNN
F 1 "C_Small" H 6467 1230 50  0000 L CNN
F 2 "" H 6375 1275 50  0001 C CNN
F 3 "~" H 6375 1275 50  0001 C CNN
	1    6375 1275
	-1   0    0    1   
$EndComp
$Comp
L Device:C_Small C1
U 1 1 60839267
P 6375 2025
F 0 "C1" H 6467 2071 50  0000 L CNN
F 1 "C_Small" H 6467 1980 50  0000 L CNN
F 2 "" H 6375 2025 50  0001 C CNN
F 3 "~" H 6375 2025 50  0001 C CNN
	1    6375 2025
	-1   0    0    1   
$EndComp
Wire Wire Line
	6275 1375 6375 1375
Wire Wire Line
	6375 1175 6275 1175
Wire Wire Line
	6375 1175 6375 1000
Wire Wire Line
	6375 1000 5375 1000
Wire Wire Line
	5375 1000 5375 1075
Connection ~ 6375 1175
Wire Wire Line
	6275 1925 6375 1925
Wire Wire Line
	6375 2125 6275 2125
$Comp
L 3Phase-rescue:R-3Phase-rescue R2
U 1 1 6087B6A9
P 6375 2275
F 0 "R2" V 6455 2275 50  0000 C CNN
F 1 "100k" H 6375 2275 50  0001 C CNN
F 2 "Footprints:R_0603_1608Metric" V 6305 2275 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Chip-Resistor-Surface-Mount_100KR-104-5_C100048.html" H 6375 2275 50  0001 C CNN
	1    6375 2275
	1    0    0    -1  
$EndComp
Connection ~ 6375 2125
Wire Wire Line
	5150 1525 5150 1000
Wire Wire Line
	5150 1000 5375 1000
Connection ~ 5375 1000
$Comp
L 3Phase-rescue:R-3Phase-rescue R3
U 1 1 608D8CBE
P 6375 1550
F 0 "R3" V 6455 1550 50  0000 C CNN
F 1 "100k" H 6375 1550 50  0001 C CNN
F 2 "Footprints:R_0603_1608Metric" V 6305 1550 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Chip-Resistor-Surface-Mount_100KR-104-5_C100048.html" H 6375 1550 50  0001 C CNN
	1    6375 1550
	1    0    0    -1  
$EndComp
$Comp
L 3Phase-rescue:R-3Phase-rescue R5
U 1 1 608E33E2
P 5375 1625
F 0 "R5" V 5455 1625 50  0000 C CNN
F 1 "100k" H 5375 1625 50  0001 C CNN
F 2 "Footprints:R_0603_1608Metric" V 5305 1625 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Chip-Resistor-Surface-Mount_100KR-104-5_C100048.html" H 5375 1625 50  0001 C CNN
	1    5375 1625
	1    0    0    -1  
$EndComp
Wire Wire Line
	6375 1400 6375 1375
Connection ~ 6375 1375
Wire Wire Line
	6375 1700 6375 1925
Connection ~ 6375 1925
Wire Wire Line
	5375 1825 5375 1800
Connection ~ 5375 1800
Wire Wire Line
	5375 1800 5375 1775
Wire Wire Line
	5375 2525 6375 2525
Wire Wire Line
	6375 2525 6375 2425
Wire Wire Line
	5375 2525 5150 2525
Wire Wire Line
	5150 2525 5150 2050
Connection ~ 5375 2525
Wire Wire Line
	5375 1800 5250 1800
$Comp
L 3Phase-rescue:R-3Phase-rescue R6
U 1 1 6099A70D
P 4950 2350
F 0 "R6" V 5030 2350 50  0000 C CNN
F 1 "100k" H 4950 2350 50  0001 C CNN
F 2 "Footprints:R_0603_1608Metric" V 4880 2350 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Chip-Resistor-Surface-Mount_100KR-104-5_C100048.html" H 4950 2350 50  0001 C CNN
	1    4950 2350
	-1   0    0    1   
$EndComp
$Comp
L 3Phase-rescue:C_Small-3Phase-rescue C38
U 1 1 60441A0A
P 4825 1625
F 0 "C38" H 4835 1695 50  0000 L CNN
F 1 "100nF 25v" H 4835 1545 50  0001 L CNN
F 2 "Footprints:R_0603_1608Metric" H 4825 1625 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Multilayer-Ceramic-Capacitors-MLCC-SMD-SMT_SAMSUNG_CL10B104KA8NNNC_100nF-104-10-25V_C1590.html" H 4825 1625 50  0001 C CNN
	1    4825 1625
	-1   0    0    -1  
$EndComp
$Comp
L 3Phase-rescue:C_Small-3Phase-rescue C3
U 1 1 609B71CB
P 4725 1875
F 0 "C3" H 4735 1945 50  0000 L CNN
F 1 "100nF 25v" H 4735 1795 50  0001 L CNN
F 2 "Footprints:R_0603_1608Metric" H 4725 1875 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Multilayer-Ceramic-Capacitors-MLCC-SMD-SMT_SAMSUNG_CL10B104KA8NNNC_100nF-104-10-25V_C1590.html" H 4725 1875 50  0001 C CNN
	1    4725 1875
	-1   0    0    -1  
$EndComp
Wire Wire Line
	4600 1000 4725 1000
Connection ~ 5150 1000
Wire Wire Line
	4950 2100 4825 2100
Wire Wire Line
	4825 1725 4825 2100
Connection ~ 4825 2100
Wire Wire Line
	4825 2100 4500 2100
Wire Wire Line
	4825 1525 4825 1000
Connection ~ 4825 1000
Wire Wire Line
	4825 1000 5150 1000
Wire Wire Line
	4500 2200 4725 2200
Wire Wire Line
	4725 2200 4725 1975
Wire Wire Line
	4725 2200 4950 2200
Connection ~ 4725 2200
Wire Wire Line
	4725 1775 4725 1000
Connection ~ 4725 1000
Wire Wire Line
	4725 1000 4825 1000
Wire Wire Line
	4600 2000 4600 1000
Wire Wire Line
	4600 2000 4500 2000
Wire Wire Line
	5150 2525 4950 2525
Connection ~ 5150 2525
Wire Wire Line
	4950 2500 4950 2525
Connection ~ 4950 2525
Connection ~ 4600 1000
Wire Wire Line
	4400 1300 4400 1650
Wire Wire Line
	4400 1650 4500 1650
Wire Wire Line
	4500 1650 4500 1900
Wire Wire Line
	4000 1300 4000 1650
Wire Wire Line
	4000 1650 3900 1650
Wire Wire Line
	3900 1650 3900 1900
Connection ~ 3800 1000
Wire Wire Line
	3900 2000 3800 2000
Wire Wire Line
	4250 1300 4150 1300
Wire Wire Line
	5050 1800 4950 1800
$Comp
L 3Phase-rescue:D_Schottky_Small_ALT-3Phase-rescue D?
U 1 1 60B3865C
P 1775 1600
F 0 "D?" H 1775 1487 50  0000 C CNN
F 1 "SS14" H 1775 1804 50  0001 C CNN
F 2 "Footprints:D_SMA" H 1775 1713 50  0001 C CNN
F 3 "" V 1775 1600 50  0000 C CNN
	1    1775 1600
	1    0    0    -1  
$EndComp
$Comp
L Transistor_FET:DMG2302U Q?
U 1 1 60B456A6
P 1900 1800
F 0 "Q?" H 1825 1650 50  0000 L CNN
F 1 "DMG2302U" H 2104 1755 50  0001 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 2100 1725 50  0001 L CIN
F 3 "http://www.diodes.com/assets/Datasheets/DMG2302U.pdf" H 1900 1800 50  0001 L CNN
	1    1900 1800
	0    -1   1    0   
$EndComp
$Comp
L 3Phase-rescue:R-3Phase-rescue R?
U 1 1 6073C035
P 2075 1500
F 0 "R?" V 2155 1500 50  0000 C CNN
F 1 "100k" H 2075 1500 50  0001 C CNN
F 2 "Footprints:R_0603_1608Metric" V 2005 1500 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Chip-Resistor-Surface-Mount_100KR-104-5_C100048.html" H 2075 1500 50  0001 C CNN
	1    2075 1500
	0    1    1    0   
$EndComp
$Comp
L 3Phase-rescue:D_Schottky_Small_ALT-3Phase-rescue D?
U 1 1 6073F459
P 3400 1325
F 0 "D?" H 3400 1212 50  0000 C CNN
F 1 "SS14" H 3400 1529 50  0001 C CNN
F 2 "Footprints:D_SMA" H 3400 1438 50  0001 C CNN
F 3 "" V 3400 1325 50  0000 C CNN
	1    3400 1325
	0    -1   -1   0   
$EndComp
$Comp
L 3Phase-rescue:C_Small-3Phase-rescue C?
U 1 1 60740031
P 3275 1150
F 0 "C?" H 3285 1220 50  0000 L CNN
F 1 "100nF 25v" H 3285 1070 50  0001 L CNN
F 2 "Footprints:R_0603_1608Metric" H 3275 1150 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Multilayer-Ceramic-Capacitors-MLCC-SMD-SMT_SAMSUNG_CL10B104KA8NNNC_100nF-104-10-25V_C1590.html" H 3275 1150 50  0001 C CNN
	1    3275 1150
	-1   0    0    -1  
$EndComp
$Comp
L 3Phase-rescue:R-3Phase-rescue R?
U 1 1 60741A0F
P 3525 1650
F 0 "R?" V 3605 1650 50  0000 C CNN
F 1 "100k" H 3525 1650 50  0001 C CNN
F 2 "Footprints:R_0603_1608Metric" V 3455 1650 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Chip-Resistor-Surface-Mount_100KR-104-5_C100048.html" H 3525 1650 50  0001 C CNN
	1    3525 1650
	1    0    0    -1  
$EndComp
$Comp
L Device:CP_Small C?
U 1 1 60743158
P 3675 1375
F 0 "C?" H 3763 1421 50  0000 L CNN
F 1 "CP_Small" H 3763 1330 50  0000 L CNN
F 2 "" H 3675 1375 50  0001 C CNN
F 3 "~" H 3675 1375 50  0001 C CNN
	1    3675 1375
	-1   0    0    1   
$EndComp
$Comp
L 3Phase-rescue:R-3Phase-rescue R?
U 1 1 60740D24
P 3525 1150
F 0 "R?" V 3605 1150 50  0000 C CNN
F 1 "100k" H 3525 1150 50  0001 C CNN
F 2 "Footprints:R_0603_1608Metric" V 3455 1150 50  0001 C CNN
F 3 "https://lcsc.com/product-detail/Chip-Resistor-Surface-Mount_100KR-104-5_C100048.html" H 3525 1150 50  0001 C CNN
	1    3525 1150
	1    0    0    -1  
$EndComp
Wire Wire Line
	3675 1275 3675 1000
Connection ~ 3675 1000
Wire Wire Line
	3675 1000 3625 1000
Wire Wire Line
	4950 2525 3675 2525
$Comp
L Device:L_Small L?
U 1 1 6073E06C
P 3400 1800
F 0 "L?" H 3448 1846 50  0000 L CNN
F 1 "L_Small" H 3448 1755 50  0000 L CNN
F 2 "" H 3400 1800 50  0001 C CNN
F 3 "~" H 3400 1800 50  0001 C CNN
	1    3400 1800
	-1   0    0    1   
$EndComp
Wire Wire Line
	3400 1700 3400 1600
Wire Wire Line
	3400 1600 3400 1425
Wire Wire Line
	3400 1600 3150 1600
Connection ~ 3400 1600
Connection ~ 3525 1500
Wire Wire Line
	3525 1500 3525 1300
Wire Wire Line
	3275 1250 3275 1400
Wire Wire Line
	3275 1400 3150 1400
Wire Wire Line
	3150 1300 3150 1000
Wire Wire Line
	3150 1000 3275 1000
Connection ~ 3625 1000
Connection ~ 3525 1000
Wire Wire Line
	3525 1000 3625 1000
Wire Wire Line
	3275 1050 3275 1000
Connection ~ 3275 1000
Wire Wire Line
	3275 1000 3400 1000
Wire Wire Line
	3400 1225 3400 1000
Connection ~ 3400 1000
Wire Wire Line
	3400 1000 3525 1000
$Comp
L Device:CP_Small C?
U 1 1 607F0664
P 2250 1150
F 0 "C?" H 2338 1196 50  0000 L CNN
F 1 "CP_Small" H 2338 1105 50  0000 L CNN
F 2 "" H 2250 1150 50  0001 C CNN
F 3 "~" H 2250 1150 50  0001 C CNN
	1    2250 1150
	-1   0    0    1   
$EndComp
Wire Wire Line
	2350 1300 2250 1300
Wire Wire Line
	2250 1300 2250 1250
Wire Wire Line
	2350 1400 2250 1400
Wire Wire Line
	2250 1400 2250 1300
Connection ~ 2250 1300
Wire Wire Line
	2350 1500 2250 1500
Wire Wire Line
	2250 1400 2250 1500
Connection ~ 2250 1400
Connection ~ 2250 1500
Wire Wire Line
	2250 1500 2225 1500
Wire Wire Line
	2250 1050 2250 1000
Wire Wire Line
	2250 1000 3150 1000
Connection ~ 3150 1000
Wire Wire Line
	3675 2525 3675 1900
Wire Wire Line
	3400 1900 3525 1900
Connection ~ 3675 1900
Wire Wire Line
	3675 1900 3675 1475
Wire Wire Line
	3525 1900 3525 1800
Connection ~ 3525 1900
Wire Wire Line
	3525 1900 3675 1900
Wire Wire Line
	3525 1500 3150 1500
Wire Wire Line
	1625 1000 2250 1000
Connection ~ 2250 1000
Wire Wire Line
	1625 1100 1625 1000
Connection ~ 1625 1000
Wire Wire Line
	1625 1200 1625 1100
Connection ~ 1625 1100
Wire Wire Line
	1925 1500 1900 1500
Wire Wire Line
	2350 1600 1900 1600
Wire Wire Line
	1900 1600 1900 1500
Connection ~ 1900 1500
Wire Wire Line
	1900 1500 1625 1500
Wire Wire Line
	1625 1400 1625 1500
Connection ~ 1625 1500
Wire Wire Line
	1625 1300 1625 1400
Connection ~ 1625 1400
Wire Wire Line
	1900 1600 1875 1600
Connection ~ 1900 1600
Wire Wire Line
	1675 1600 1625 1600
Wire Wire Line
	1625 1700 1625 1600
Connection ~ 1625 1600
Connection ~ 1625 1700
Wire Wire Line
	2100 1900 3400 1900
Connection ~ 3400 1900
Wire Wire Line
	1625 1800 1625 1700
Wire Wire Line
	1625 1800 1700 1800
Wire Wire Line
	1700 1800 1700 1900
Connection ~ 1625 1800
$Comp
L Connector:Conn_01x10_Male J8
U 1 1 604239FD
P 1425 1400
F 0 "J8" H 1397 1282 50  0000 R CNN
F 1 "Conn_01x06_Male" H 1397 1373 50  0000 R CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x06_P2.54mm_Vertical" H 1425 1400 50  0001 C CNN
F 3 "~" H 1425 1400 50  0001 C CNN
	1    1425 1400
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 60A6D1F6
P 2425 2325
F 0 "#PWR?" H 2425 2075 50  0001 C CNN
F 1 "GND" H 2430 2152 50  0000 C CNN
F 2 "" H 2425 2325 50  0001 C CNN
F 3 "" H 2425 2325 50  0001 C CNN
	1    2425 2325
	0    -1   -1   0   
$EndComp
$EndSCHEMATC
