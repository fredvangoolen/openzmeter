

function UpdateHours(visibleSeries) {
  var Hours = Array(24).fill(0), HoursCount = Array(24).fill(0);
  if(HourTable !== null) {
    HourTable.clear();
    HourTable.destroy();
  }
  for(i = 0; i < EnergyData.length; i++) {
    var Hour = new Date(EnergyData[i][0]).getHours();
    Hours[Hour] = Hours[Hour] + SumPhases(EnergyData[i], visibleSeries);
    HoursCount[Hour] = HoursCount[Hour] + 1;
  }
  var Max = -Infinity;
  var Min =  Infinity;
  for(i in Hours) {
    if(Hours[i] === 0) continue;
    Max = Math.max(Max, Hours[i] / HoursCount[i]);
    Min = Math.min(Min, Hours[i] / HoursCount[i]);
  }
  var Units = GetBestUnits(Math.max(Math.abs(Min), Math.abs(Max)));
  $('#HourUnits').html('<em>' + Units + 'Wh</em>');
  var Contents = $('#HourTable tbody');
  for(i = 0; i < 24; i++) {
    var Energy = Hours[i];
    var Count  = HoursCount[i];
    Contents.append('<tr>' +
      '<td data-sort="' + i + '">' + i + '</td>' + 
      '<td data-sort="' + Energy + '"><div class="progress" style="margin-bottom:0px; position:relative; height: 16px">' +
      '<div class="progress-bar" role="progressbar" style="width:' + (100 * (Energy / Count) / Max) + '%"></div>' +
      '</div></td>' + 
      '<td data-sort="' + (Energy / Count) + '">' + ((HoursCount[i] === 0) ? '-' : Round2(ScaleToUnits(Energy / Count, Units))) + '</td>' +
      '<td data-sort="' + Energy + '">' + ((HoursCount[i] === 0) ? '-' : Round2(ScaleToUnits(Energy, Units))) + '</td>' +
      '</tr>');
  }
  HourTable = $('#HourTable').DataTable({ responsive: true, paging: false, info: false, searching: false,
                                                columnDefs: [ { targets: [1], sortable: false }, { targets: [2, 3], sortable: true, className: "text-right" }, { targets: [0], sortable: true, className: "text-center" } ],
                                                order: [0, 'asc']
  });
}
