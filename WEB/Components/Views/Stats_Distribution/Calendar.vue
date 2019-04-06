<template>
  <div>
    <div class="calendar" ref="calendar"></div>
    <div class="clearfix"></div>
  </div>
</template>

<script>
  import calendar from 'JS/bootstrap-year-calendar.js';
  import moment from 'JS/moment.js';
  import chroma from 'JS/chroma.js';
  import * as oZm from 'JS/openZmeter.js';
  ;(function($) {
    $.fn.calendar.dates['custom'] = {
      days: ["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"],
      daysShort: ["Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"],
      daysMin: ["Su", "Mo", "Tu", "We", "Th", "Fr", "Sa", "Su"],
      months: ["January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"],
      monthsShort: ["Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"],
      weekShort: 'W',
      weekStart: 1
    };
  } (jQuery));
  export default {
    data: function() {
      return {};
    },
    created: function() {
      this.Calendar = null;
    },
    mounted: function() {
      this.Calendar = $(this.$refs.calendar).calendar({ language: "custom" });
    },
    methods: {
      SetData: function(result) {
        if(result['Active_Power'].length !== 0) this.Calendar.setYear(moment(result.Sampletime[0]).year());
        var Days = [];
        for(var i = 0; i < result['Active_Power'].length; i++) {
          var Day = moment(result.Sampletime[i]).dayOfYear();
          if(Days[Day] === undefined) Days[Day] = null;
          if(result['Active_Power'][i] !== null) Days[Day] = (Days[Day] === null) ? result['Active_Power'][i] : Days[Day] + result['Active_Power'][i];
        }
        var ColorScale = chroma.scale(['#000000', '#0000FF', '#FF0000', '#FFFF00']).domain([0, 1]);
        var Max = -Infinity;
        var Min =  Infinity;
        for(var Day in Days) {
          if(Days[Day] !== null) {
            Max = Math.max(Max, Days[Day]);
            Min = Math.min(Min, Days[Day]);
          }
        }
        var Units = oZm.GetBestUnits(Math.max(Math.abs(Min), Math.abs(Max)));
        this.$emit('RangeChanged', oZm.ScaleToUnits(Min, Units), oZm.ScaleToUnits(Max, Units), Units);
        this.Calendar.setCustomDayRenderer(function(element, date) {
          var Day = moment(date).dayOfYear();
          if(Days[Day] === null) {
            $(element).css({ 'color': '#BBB' });
            $(element).prop({'data-toggle': 'tooltip', 'title': 'No data'} );
          } else {
            var Energy = Days[Day];
            var Color = ColorScale((Energy - Min) / (Max - Min));
            $(element).css({ 'background-color': Color, 'color': chroma(Color).luminance() > 0.5 ? '#000' : '#FFF', 'border': '1px solid #fff'});
            $(element).prop({'data-toggle': 'tooltip', 'title': oZm.Round2Plain(oZm.ScaleToUnits(Energy, Units)) + ' ' + Units + 'Wh'} );
          }
        });
      }
    }
  }
</script>

<style scoped>
  .calendar >>> .day-content {
    font-size: 12px;
    text-align: center;
    width: 35px;
  }
  .calendar >>> .month-title, .calendar >>> .day-header {
    text-align: center;
  }
  .calendar >>> .month thead {
    background: #EEE;
    border: 1px solid #DDD;
    border-radius: 3px;
  }
</style>