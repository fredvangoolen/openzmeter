<template>
  <div class="panel panel-other">
    <div class="panel-heading">By month<div class="pull-right">{{ Units }}Wh</div></div>
    <div class="panel-body">
      <vuetable ref="table" :fields="Columns" :api-mode="false" class="table small table-striped table-responsive">
        <template slot="period" slot-scope="props">
          <div class="period-color">{{props.rowData}}</div>
          
        </template>
      </vuetable>
    </div>
  </div>
</template>

<script>
  
  import vuetable from 'vuetable-2';
  import moment from 'JS/moment.js';
  var Names = ["January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"];
  export default {
    data: function() {
      return { ShowSpinner: false, Units: '', 
        Columns: [
          {name: 'Month', title: 'User Name'},
          {name: '__slot:period', title: '-'},
          {name: '__slot:period', title: 'P1'}, 
          {name: '__slot:period', title: 'P2'}
        ]
      };
    },
    methods: {
      SetData: function(result) {
        var Months = [];
        for(var i = 0; i < result['Active_Power'].length; i++) {
          var Month = moment(result.Sampletime[i]).month();
          if(Months[Month] === undefined) Months[Month] = { };
          var Period = (result.Period_Name[i] === null) ? 'NONE' : result.Period_Name[i];
          if(Months[Month][Period] === undefined) Months[Month][Period] = null;
          if(result['Active_Power'][i] !== null) Months[Month][Period] = (Months[Month][Period] === null) ? result['Active_Power'][i] : Months[Month][Period] + result['Active_Power'][i];
        }
        var Periods = [];
        for(var Month in Months) {
          for(var Period in Months[Month]) {
            if(Periods.includes(Period) === false) Periods.push(Period);
          }
        }
        Periods = Periods.sort();
        this.Columns = [ {name: 'Month', title: 'Month'} ]; 
        for(var Period in Periods) {
          this.Columns.push({name: Period, title: Period});
        }
    
        
        this.$refs.table.setData({ data: Months });
 return;   
    
        for(var i = 0; i < result[EnergyData].length; i++) {
    var Month     = new Date(EnergyData[i][0]).getMonth();
    var DayOfWeek =  new Date(EnergyData[i][0]).getDay();
    var Energy = 0;
    if(WeekDay[Month] === undefined) WeekDay[Month] = 0.0;
    if(WeekEnd[Month] === undefined) WeekEnd[Month] = 0.0;
    if((DayOfWeek === 0) || (DayOfWeek === 6)) {
      WeekEnd[Month] = WeekEnd[Month] + SumPhases(EnergyData[i], visibleSeries);
    } else {
      WeekDay[Month] = WeekDay[Month] + SumPhases(EnergyData[i], visibleSeries);
    }
  }
  var Max = -Infinity, Min =  Infinity;
  for(i in WeekDay) {
    Max = Math.max(Max, WeekDay[i] + WeekEnd[i]);
    Min = Math.min(Min, WeekDay[i] + WeekEnd[i]);
  }
  var Units = GetBestUnits(Math.max(Math.abs(Min), Math.abs(Max)));
  $('#MonthUnits').html('<em>' + Units + 'Wh</em>');
  var Contents = $('#MonthTable tbody');
  for(i = 0; i < 12; i++) {
    var startDate = new Date(SelectedYear, i, 1);
    var endDate = new Date(SelectedYear, i + 1, 0);
    var E0 = (WeekDay[i] !== undefined) ? WeekDay[i] : 0, E1 = (WeekEnd[i] !== undefined) ? WeekEnd[i] : 0, E2 = E0 + E1;
    Contents.append('<tr>' +
      '<td data-sort="' + i + '">' + $.fn.calendar.dates['custom']['monthsShort'][i] + '</td>' + 
      '<td><div class="progress" style="margin-bottom:0px; position:relative; height: 16px">' +
      '<div class="progress-bar" role="progressbar" style="width:' + (100 * E0 / Max) + '%"></div>' +
      '<div class="progress-bar" role="progressbar" style="background-color: #7eb0db; width:' + (100 * E1 / Max) + '%"></div>' +
      '</div></td>' +
      '<td data-sort="' + E0 + '">' + ((WeekDay[i] !== undefined) ? Round2(ScaleToUnits(E0, Units)) : '-') + '</td>' +
      '<td data-sort="' + E1 + '">' + ((WeekDay[i] !== undefined) ? Round2(ScaleToUnits(E1, Units)) : '-') + '</td>' +
      '<td data-sort="' + E2 + '">' + ((WeekDay[i] !== undefined) ? Round2(ScaleToUnits(E2, Units)) : '-') + '</td>' +
      '</tr>');
  }
  MonthTable = $('#MonthTable').DataTable({ responsive: true, paging: false, info: false, searching: false, 
                                            columnDefs: [ { targets: [1], sortable: false }, { targets: [0, 2, 3, 4], sortable: true, className: "text-center" } ],
                                            order: [0, 'asc']
  });
        
      }
    },
    components: { vuetable }
  }
</script>

<style scoped>

</style>