<template>
  <div class="calendar-container">
    <table class="stripe-table" v-for="(month, idx) in YearTable" :key="idx">
        <tr><td colspan="7">{{ month.Name }}</td></tr>
        <tr>
          <td>Mo</td>
          <td>Tu</td>
          <td>We</td>
          <td>Th</td>
          <td>Fr</td>
          <td>Sa</td>
          <td>Su</td>
        </tr>
        <tr v-for="(week, idx) in month.Weeks" :key="idx">
          <td v-for="(s, idx) in week" :key="idx" :class="{ empty: s.Number==null }" :style="{ background: colors[value[s.Index]]}" data-toggle="tooltip" :title="names[value[s.Index]]" @click="SetDay(s.Index)" @mousemove="SetDay(s.Index, $event)">{{ s.Number }}</td>
        </tr>
    </table>
  </div>
</template>

<script>
  import moment from 'moment';
  export default {
    props: { value: Array, colors: Array, names: Array, year: Number, selected: Number },
    data: function() {
      return { Selected: 0};
    },
    computed: {
      YearTable: function() {
        var Months = [];
        for(var month = 0; month < 12; month++) Months.push(this.BuildMonth(month));
        return Months;
      }
    },
    methods: {
      SetDay: function(idx, evt) {
        if((evt != null) && (evt.buttons == 0)) return;
        var Periods = this.value.slice(0);
        Periods[idx] = this.selected;
        this.$emit('input', Periods);
      },
      BuildMonth: function(month) {
        var MonthNames = ["January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"];
        var Month = { Name: MonthNames[month], Weeks: [] };
        for(var n = 0; n < 6; n++) {
          Month.Weeks.push([]);
          for(var i = 0; i < 7; i++) Month.Weeks[n][i] = { Number: null };
        }
        var CurrentWeek = 0;
        var PrevWeekDay = 0;
        for(var day = 0; day < moment([this.year, month]).daysInMonth(); day++) {
          var WeekDay = moment([this.year, month, day + 1]).isoWeekday();
          if(WeekDay < PrevWeekDay) CurrentWeek++;
          Month.Weeks[CurrentWeek][WeekDay - 1].Number = String(day + 1);
          Month.Weeks[CurrentWeek][WeekDay - 1].Index = moment([this.year, month, day + 1]).dayOfYear() - 1;
          PrevWeekDay = WeekDay;
        }
        return Month;
      }
    }
  }
</script>

<style scoped>
  .calendar-container {
    display: flex;
    flex-wrap: wrap;
    align-content: center;
    padding: 4px 0px 0px 5px;
  }
  .stripe-table {
    width: 136px;
    table-layout: fixed;
    font-size: 11px;
    border: 1px solid #ccc;
    margin-right: 4px;
    margin-bottom: 4px;
  }
  .stripe-table tr td {
    height: 16px;
    line-height: 14px;
    text-align: center;
    border: 1px solid #ccc;
    cursor: pointer;
    user-select: none;
  }
  .stripe-table tr:first-child td {
    border-bottom: none;
    line-height: 12px;
    font-weight: bold;
    cursor: auto;
    background: #EEE;
  }
  .stripe-table tr:nth-child(2) td {
    border: none;
    line-height: 12px;
    font-weight: bold;
    cursor: auto;
    background: #EEE;
  }
  .stripe-table tr td.empty {
    background: url('data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAQAAAAECAIAAAAmkwkpAAAAJElEQVQImT2JsQ0AMAyDyP832q+QIWqZEJDEx6gA0JZr9/mmLvBPKENuQmcdAAAAAElFTkSuQmCC') !important;
    cursor: unset;
  }
</style>