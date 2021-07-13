## [feb-2021]
  - Fix in events detection and recording
  - Remote sync works

## [sep-2020]
  - Lots of internal changes to reduce memory and CPU usage.
  - API changed to stream data from database without need to load entire dataset.
  - Lots of bugs fixed in frontend:
    - Export buttons.
    - Labels.
    - Units in graphs.
    - Period/bill decoration in graphs.
    - Bar graphs alignament to hours/quarters.
    - X axes labels alignament to hours/quarters.
    - Timezone corrections.
  - Rate comparison function added. Aditionaly to current real rates, other groups of rates may be added to an analyzer. In 'Cost' view a group can be selected.
  - API documentation improved.
  - Ethernet configuration added to board with embedded ethernet (OrangePI).

## [jun-2020]
  - Alarms and Disturbances badged added to WEB menu.
  - Waveform stream function added to WEB.

## [may-2020]
  - Add pagination support in some tables than are spected to have lot of rows
  - Added ESIOS fetcher and other advanced functions to tariffs calculations

## [abr-2020]
  - Added advanced tariffs, most dialogs are rewrited to allow higher flexibility
  - Help view improved, it now allow to update and test API

## [mar-2020]
  - Symmetrical components and unbalance added
  - Implement voltage-current Lissajou plots
  - Added visual editor for alarms

## [feb-2020] - First stable release
  - Most functions works as spected