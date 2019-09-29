<template>
  <div class="panel-other">
    <div class="panel panel-heading">By week day<div class="float-right">{{ Units }}</div></div>
    <div class="panel-body">
      <vuetable ref="table" :fields="Columns" :api-mode="false" class="table small table-striped table-responsive"></vuetable>
    </div>
  </div>
</template>

function UpdateWeeks(visibleSeries) {
  var WeekDay = [], WeekEnd = [];
  if(WeekTable !== null) {
    WeekTable.clear();
    WeekTable.destroy();
  }
  for(i = 0; i < EnergyData.length; i++) {
    var Week      = GetWeekOfYear(EnergyData[i][0]);
    var DayOfWeek = new Date(EnergyData[i][0]).getDay();
    if(WeekDay[Week] === undefined) WeekDay[Week] = 0.0;
    if(WeekEnd[Week] === undefined) WeekEnd[Week] = 0.0;
    if((DayOfWeek === 0) || (DayOfWeek === 6)) {
      WeekEnd[Week] = WeekEnd[Week] + SumPhases(EnergyData[i], visibleSeries);
    } else {
      WeekDay[Week] = WeekDay[Week] + SumPhases(EnergyData[i], visibleSeries);
    }
  }
  var Max = -Infinity, Min =  Infinity;
  for(i in WeekDay) {
    Max = Math.max(Max, WeekDay[i] + WeekEnd[i]);
    Min = Math.min(Min, WeekDay[i] + WeekEnd[i]);
  }
  var Units = GetBestUnits(Math.max(Math.abs(Min), Math.abs(Max)));
  $('#WeekUnits').html('<em>' + Units + 'Wh</em>');
  var Contents = $('#WeekTable tbody');
  for(i = 1; ;i++) {
    var startDate = GetDateOfISOWeek(i, SelectedYear);
    var endDate = new Date(startDate.getFullYear(), startDate.getMonth(), startDate.getDate() + 6);
    if(endDate.getFullYear() !== SelectedYear) break;
    var E0 = (WeekDay[i] !== undefined) ? WeekDay[i] : 0, E1 = (WeekEnd[i] !== undefined) ? WeekEnd[i] : 0, E2 = E0 + E1;
    Contents.append('<tr>' +
      '<td data-toogle="tooltip" title="From ' + startDate.toLocaleDateString() + ' to ' + endDate.toLocaleDateString() + '">' + i + '</td>' + 
      '<td><div class="progress" style="margin-bottom:0px; position:relative; height: 16px">' +
      '<div class="progress-bar" role="progressbar" style="width:' + (100 * E0 / Max) + '%"></div>' +
      '<div class="progress-bar" role="progressbar" style="background-color: #7eb0db; width:' + (100 * E1 / Max) + '%"></div>' +
      '</div></td>' +
      '<td data-sort="' + E0 + '">' + ((WeekDay[i] !== undefined) ? Round2(ScaleToUnits(E0, Units)) : '-') + '</td>' +
      '<td data-sort="' + E1 + '">' + ((WeekDay[i] !== undefined) ? Round2(ScaleToUnits(E1, Units)) : '-') + '</td>' +
      '<td data-sort="' + E2 + '">' + ((WeekDay[i] !== undefined) ? Round2(ScaleToUnits(E2, Units)) : '-') + '</td>' +
      '</tr>');
  }
  WeekTable = $('#WeekTable').DataTable({ responsive: true, paging: false, info: false, searching: false,
                                          columnDefs: [ { targets: [1], sortable: false }, { targets: [0, 2, 3, 4], sortable: true, className: "text-center" } ],
                                          order: [0, 'asc']
  });
}

