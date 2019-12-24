<template>
  <div>
    <div class="calendar" ref="calendar"></div>
    <div class="clearfix"></div>
  </div>
</template>

<script>
  import calendar from 'JS/bootstrap-year-calendar.js';
  import moment from 'moment';
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
    props: { data: Object, year: Number },
    data: function() {
      return { };
    },
    mounted: function() {
      this.Calendar = $(this.$refs.calendar).calendar({ language: "custom" });
      if(this.year != null) this.Calendar.setYear(this.year);
      if(this.data != null) this.SetData();
    },
    watch: {
      year: function(year) {
        this.Calendar.setYear(year);
      },
      data: function() {
        this.SetData();
      }
    }, 
    methods: {
      GetData: function(data) {
        data.push(['Day', 'Energy']);
        for(var day in this.Days) data.push([moment().year(this.Year).dayOfYear(day).format('DD/MM/YYYY'), this.Days[day]]);
        return data;
      },
      SetData: function() {
        this.Days = [];
        for(var i in this.data['Active_Power']) {
          var Day = moment(this.data.Sampletime[i]).dayOfYear();
          if(this.data.Active_Power[i] != null) this.Days[Day] = (this.Days[Day] === undefined) ? this.data.Active_Power[i] / 4.0 : this.Days[Day] + this.data.Active_Power[i] / 4.0;
        }
        var ColorScale = chroma.bezier(['#00429d', '#96ffea', '#ffffe0', '#ff005e', '#93003a']);
        var Max = -Infinity;
        var Min =  Infinity;
        for(var Day in this.Days) {
          if(this.Days[Day] == undefined) continue;
          Max = Math.max(Max, this.Days[Day]);
          Min = Math.min(Min, this.Days[Day]);
        }
        this.$emit('range', [Min, Max]);
        this.Calendar.setCustomDayRenderer(function(element, date) {
          var Day = moment(date).dayOfYear();
          if((this.Days[Day] == undefined) || (this.Days[Day] == 0)) {
            var Color = "url('data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAQAAAAECAIAAAAmkwkpAAAAJElEQVQImT2JsQ0AMAyDyP832q+QIWqZEJDEx6gA0JZr9/mmLvBPKENuQmcdAAAAAElFTkSuQmCC')";
            $(element).css({ color: '#BBB',  border: '1px solid #EEE', background: Color });
            $(element).prop({'data-toggle': 'tooltip', 'title': 'No data'} );
          } else {
            var Color = ColorScale((this.Days[Day] - Min) / (Max - Min));
            var Units = oZm.GetBestUnits(this.Days[Day]);
            $(element).css({ background: Color, color: chroma(Color).luminance() > 0.5 ? '#000' : '#FFF', border: '1px solid ' + chroma(Color).darker(0.1)});
            $(element).prop({'data-toggle': 'tooltip', 'title': oZm.RoundPlain(oZm.ScaleToUnits(this.Days[Day], Units), 2) + ' ' + Units + 'Wh'} );
          }
        }.bind(this));
      }
    }
  }
</script>

<style scoped>
  .calendar >>> table {
    border-collapse: separate !important;
    border-spacing: 2px !important;
  }
  .calendar >>> .day-content {
    font-size: 12px;
    text-align: center;
    width: 32px;
    height: 26px;
    padding: 3px !important;
    border-radius: 3px;
  }
  .calendar >>> .month-title, .calendar >>> .day-header {
    text-align: center;
  }
  .calendar >>> .month thead {
    background: #EEE;
    border: 1px solid #DDD;
  }
</style>