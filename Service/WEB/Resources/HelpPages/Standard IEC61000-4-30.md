This part of the Standard IEC 61000-4 defines the measurement and evaluation of the quality of the power supplied with a declared fundamental frequency of 50 Hz or 60Hz.

The power quality parameters considered in this standard are: 

* power frequency	
* magnitude of the supply voltage
* flicker
* supply voltage dips and swells, voltage interruptions and transient voltages
* supply voltage unbalance
* voltage and current harmonics and interharmonics 
* mains signalling on the supply voltage and rapid voltage changes.
* current measurements
* emissions in the range of 2 kHz to 150 kHz

From the above list, flicker detection is not implemented because it is considered that its main effect (flicker in lighting) tends to disappear due to the massive implementation of LED lighting. Neither the emissions above 2500 Hz nor the voltage imbalance in the single-phase hardware versions are analyzed.

## General considerations 

The IEC 61000-4-30 standard defines two measurement methods, according to their compliance with the standard: Class A and class S. oZm implements the methods of the Class S, although in many cases it also complies with those of Class A.
In general, class A will be used when required:

* very precise measures for contractual matters
* verification of compliance with standards
* to resolve disputes.

On the other hand, class S will be used when required:

* quality statistical samplings
* diagnosis of problems with low uncertainty.

### Electrical values

For the single-phase version of oZm, only the existing voltage channel, which is called phase-neutral is used as the basis for the measurements. The current channel is also measured, although no electrical quality study is carried out on it, beyond a spectral analysis.

### Aggregation

The idea of aggregation is based on the need to offer a certain level of zoom or detail in the electrical measurements depending on the selected time horizon. It is not the same to analyze the information of complete months, where normally it is sufficient to have hourly or daily values, than to analyze slots allocations where a level of detail of the order of minutes or seconds is needed.

The norm also establishes that the measurement of 10/12 periods must be resynchronized in each UTC (coordinated universal time) of 10 min. To guarantee this measure, the system incorporates a constant synchronization by NTP protocol.

The service is designed to work continuously, so that some optimizations are made to reduce the use of resources and improve the speed of operation:
- When the service starts, it is verified that there are no pending aggregations in the database. A pending aggregation is one that has not been generated, but has expired the term in which it should have been generated. 
For example:
  - The service closes at 10:58:10 and opens again at 11:10:00. In this case, the aggregation of the time change, the 10 minutes and the minute change would be missing since the deadline to generate them has been exceeded.
  - The service closes at 10:58:10 and opens again at 10:59:10. In this case the aggregation of 1 minute is missing, but not the time change.

To maintain a reasonable size of tables, the old records are eliminated depending on the configuration, allowing to limit the number of samples in each level of aggregation. This system allows that the tables that store data do not change their size over time and the new samples replace the older ones. By default, 72000 samples are stored per level, equivalent to:
* In the aggregation of 10 cycles: 4 hours. (The sample expires 4 hours after its generation, the duration of the samples can be varied in this aggregation period. The total number of samples does not have to be exactly the same).
* In the aggregation of 3 seconds: 60 hours. (Same conditions as the previous one).
* In the aggregation of 1 minute: 50 days. (This aggregation does contain periods of equal length, so that the number of samples stored is exactly as configured, provided that the service runs continuously).
* In the 10 minute aggregation: 500 days. (Same conditions as the previous one).
* In the aggregation of 1 hour: 3000 days. (Same conditions as the previous one).

#### 10 cycles

It is the base period in which starting calculations are made. In addition, it is the one that offers the highest level of detail for the measured parameters and electrical variables. It should be noted that the oZm must be able to measure in the range of 50Hz ± 15% (Europe) or 60 ± 15% (America), so the duration of the capture of 10 cycles can vary according to the following:
- 50Hz - Nominal window of 200ms, although it can vary between 173.9ms and 236ms.
- 60Hz - Nominal window of 166.66ms, although it can vary between 144.9ms and 196ms.
- In case of DC, the duration will be constant 200ms.

As the sampling period is very short, the data obtained at this level may not be accurate and may vary from one period to the next.

### 150/180 cycles (3 seconds).
This aggregation contains the last 3 seconds of samples. The implementation of the Class A method is chosen, that is, 15 consecutive intervals (without separation) of 10 cycles with re-synchronization every 10 minutes UTC. The measurements of voltage and current, as well as harmonics and harmonic distortion follow the pattern dictated by the standard.

Other variables, not included in the IEC 61000-4-30 standard, are also added according to the 150/180 standard as:
- Powers and power factor: The arithmetic mean is calculated.
- Active energy and reactive energy: The values of the samples are added.

### 1 minute
This aggregation is not required by the standard, however it has been introduced in oZm. It contains 1 minute of samples. Aggregation occurs before the real minute change. The number of source samples used for the aggregation may vary. The aggregation methods are the same as the previous ones.
### 10 minutes
This aggregation contains 10 minutes of samples (3000 intervals of 10 cycles). Aggregation occurs before the real minute change. The number of source samples used for the aggregation may vary. The aggregation methods are the same as the previous ones.
### 1 hour
This aggregation contains 60 minute samples. Aggregation occurs before the real time change. The number of source samples used for the aggregation may vary. The aggregation methods are the same as the previous ones.
Uncertainty in the time clock
The uncertainty of the time clock is defined in relation to Coordinated Universal Time (UTC). For the Class A method, the uncertainty of the time clock should not exceed ± 20 ms for 50 Hz or ± 16.7 ms for 60 Hz, regardless of the total interval of time, with a maximum tolerance of 1 s every 24 hours in case there is no external connectivity. This can be achieved in various ways, although the preferred methods are the use of NTP synchronization servers or GPS. Since oZm runs on Linux machines, the installation of any NTP daemon allows access to an external source of temporary synchronization. In case of absence of connection, the internal architecture of the system allows not to deviate beyond 1 s every 24 hours. A complete list of NTP daemons can be found here. The implementation chosen in oZm is the one provided by Chrony due to the improvements it incorporates with respect to the official version ntpd.
It should be noted that oZm can optionally support GPS synchronization thanks to the gpsd package.
### Marking concept¿
As part of the standard, a detection of several types of events must be made on each block. In the case of oZm, all the defined types that are categorized as INTERRUPTION, DIVING, SWELL or RVC are detected. The concept of marking is necessary not only to locate the blocks in which an event has occurred, but also to inform that the measures of the period may have been influenced by the event it contains.
The concept of marking extends to the higher aggregations so that the periods of 3 seconds, 1 minute, 10 minutes and 1 hour that contain one or more events are also marked, this marking is visible in the different images of each view.
## Supply  Quality Parameters
### Frecuency
 According to the norm, the indication of frequency must be obtained every 10 seconds, although it also allows the use of shorter times. In the case of oZm, the frequency is calculated from the last 500 cycles (10 seconds for 50Hz networks), updating the measurement in each period of 10 cycles taking into account that the measurement range can be extended by ± 15%. The algorithm used is "step by zero" or _zero crossing_, which uses digital filters to minimize the effect of interharmonics or noise. The final value is the result of counting the time it takes the wave to complete the 500 cycles mentioned. Aggregations of 150 cycles, 1 minute, 10 minutes and 1 hour are made.
The measurement time intervals should not overlap. Individual periods that overlap at the boundary of an interval of 10 s are rejected. Each interval of 10 s must begin in an absolute time of 10 s of clock, with an uncertainty as defined in section 4.6 [it is necessary to verify what is started in clock strokes of 10 s absolute]
### Amplitude of the supply voltage
The measurement is the effective value of the amplitude of the voltage in a time interval of 10 periods for the 50 Hz networks (time interval of 12 periods for the 60 Hz networks). The intervals of 10/12 periods are contiguous. The uncertainty of the measurement does not exceed ± 0.1% of the nominal voltage 230 V, in the range of 10% to 150% of said voltage.

### Flicker
No flicker calculation is performed.
### Voltage dips and overvoltage
The effective half-cycle voltage is used for the characterization of a voltage dip or an overvoltage.
Detection and evaluation of the voltage dip
Talk about how we do it
The rule says:
The dip threshold is a percentage of Udin or the sliding reference voltage Usr (see 5.4.4). The selection of the reference voltage used must be declared by the user.
- On single-phase systems, a voltage dip begins when the Urms voltage falls below the dip threshold, and ends when the Urms voltage is equal to or greater than the dip threshold plus the voltage hysteresis.
The dip threshold and voltage hysteresis are determined by the user according to the application.
A voltage dip is characterized by a pair of data, residual voltage (Ures) or depth and duration:
- the residual voltage of a gap is the lowest value of Urms (1/2) measured in any channel during the gap;

- the depth of a gap is the difference between the reference voltage (Udin or Usr) and the residual voltage. In general, it is expressed as a percentage of the reference voltage;

The start time of a hole must be dated with the time of the beginning of Urms of the channel in which the event began and the end time of the hole must be dated with the time of the end of Urms with which the event was finished, as defined by the threshold plus the hysteresis.

The duration of a voltage gap is the time difference between the start and the end of the voltage gap.
