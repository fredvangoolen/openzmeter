<template>
  <div class="panel panel-other">
                    <div class="panel-heading">
                      <i class="fa fa-ellipsis-h" aria-hidden="true"></i> By week day
                      <div style="float: right" id="WeekDayUnits"></div>
                    </div>
                    <div class="panel-body">
                      <table id="WeekDayTable" class="table small table-striped table-responsive">
                        <thead>
                          <tr>
                            <th style="width: 30px">Day</th><th style="width: 100%"></th><th>Average</th><th>Accumulated</th>
                          </tr>
                        </thead>
                        <tbody>
                        </tbody>
                      </table>
                    </div>
                  </div>
  
</template>

function UpdateWeekDays(visibleSeries) {
  var WeekDay = Array(7).fill(0), WeekDayCount = Array(7).fill(0);
  if(WeekDayTable !== null) {
    WeekDayTable.clear();
    WeekDayTable.destroy();
  }
  for(i = 0; i < EnergyData.length; i++) {
    var DayOfWeek = new Date(EnergyData[i][0]).getDay();
    WeekDay[DayOfWeek] = WeekDay[DayOfWeek] + SumPhases(EnergyData[i], visibleSeries);
    WeekDayCount[DayOfWeek] = WeekDayCount[DayOfWeek] + 1;
  }
  var Max = -Infinity;
  var Min =  Infinity;
  for(i in WeekDay) {
    if(WeekDayCount[i] === 0) continue;
    Max = Math.max(Max, WeekDay[i] / WeekDayCount[i]);
    Min = Math.min(Min, WeekDay[i] / WeekDayCount[i]);
  }
  var Units = GetBestUnits(Math.max(Math.abs(Min), Math.abs(Max)));
  $('#WeekDayUnits').html('<em>' + Units + 'Wh</em>');
  var Contents = $('#WeekDayTable tbody');
  for(i = 0; i < 7; i++) {
    var Energy = WeekDay[i];
    var Count  = WeekDayCount[i];
    Contents.append('<tr>' +
      '<td data-sort="' + ((i === 0) ? 7 : i) + '">' + $.fn.calendar.dates['custom']['daysShort'][i] + '</td>' + 
      '<td data-sort="' + Energy + '"><div class="progress" style="margin-bottom:0px; position:relative; height: 16px">' +
      '<div class="progress-bar" role="progressbar" style="width:' + (100 * (Energy / Count) / Max) + '%"></div>' +
      '</div></td>' + 
      '<td data-sort="' + (Energy / Count) + '">' + ((WeekDayCount[i] === 0) ? '-' : Round2(ScaleToUnits(Energy / Count, Units))) + '</td>' +
      '<td data-sort="' + Energy + '">' + ((WeekDayCount[i] === 0) ? '-' : Round2(ScaleToUnits(Energy, Units))) + '</td>' +
      '</tr>');
  }
  WeekDayTable = $('#WeekDayTable').DataTable({ responsive: true, paging: false, info: false, searching: false,
                                                columnDefs: [ { targets: [1], sortable: false }, { targets: [2, 3], sortable: true, className: "text-right" }, { targets: [0], sortable: true, className: "text-center" } ],
                                                order: [0, 'asc']
  });
}
