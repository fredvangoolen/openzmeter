<template>
  <div id="wrapper">
    <div class="row">
      <div class="col-lg-12">
        <div class="panel panel-active-energy">
          <div class="panel-heading">
            <span class="header-text">Energy distribution</span>
            <div class="header-buttons clearfix">
              <color-bar :width="140" :height="10" ref="colorbar"></color-bar>
              <toolbar-years @YearChanged="YearChanged" aggreg="1H"></toolbar-years>
              <toolbar-tools ref="tools" :options="['EXPORT']" v-on:ExportClick="ExportClick"></toolbar-tools>
            </div>
          </div>
          <div class="panel-body">
            <spinner v-if="ShowSpinner"></spinner>
            <calendar ref="calendar" v-on:RangeChanged="RangeChanged"></calendar>
          </div>
        </div>
      </div>
      <div class="col-md-12 col-lg-6"><months ref="months"></months></div>
      <div class="col-md-12 col-lg-6"><day ref="day"></day></div>
    </div>
  </div>
</template>

<script>
  import Calendar from './Calendar.vue';
  import Months from './Months.vue';
  import Day from './Day.vue';
  import ToolbarYears from 'Components/ToolbarYears.vue';
  import ToolbarTools from 'Components/ToolbarTools.vue';
  import ColorBar from 'Components/ColorBar.vue';
  import Spinner from 'Components/Spinner.vue';
  import moment from 'JS/moment.js';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    data: function() {
      return { ShowSpinner: true };
    },
    methods: {
      YearChanged: function(year) {
        this.ShowSpinner = true;
        if(this.$root.SelectedAnalyzer === null) return;
        var Analyzer = this.$root.SelectedAnalyzer;
        var Year = moment().year(year);
        $.ajax({ url: "getPrices", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify( { Period: Year.endOf('year').valueOf() - Year.startOf('year').valueOf(), To: Year.endOf('year').valueOf(), Aggreg: '1H', LastSample: 0, Analyzer: Analyzer.ID } ),
          success: function(result) {
            this.$refs.months.SetData(result);
            this.$refs.day.SetData(result);
            this.$refs.calendar.SetData(result);
            this.ShowSpinner = false;
          }.bind(this)
        });
      },
      RangeChanged(min, max, units) {
        this.$refs.colorbar.SetRange(oZm.Round2(min), oZm.Round2(max), units + 'Wh');
      },
      ExportClick: function() {
        var Data = [];
        Data.push(['Daily stats']);
        Data = this.$refs.calendar.GetData(Data);
        Data.push([]);
        Data.push(['Monthly stats']);
        Data = this.$refs.months.GetData(Data);
        Data.push([]);
        Data.push(['Weekday stats']);
        Data = this.$refs.day.GetData(Data);
        this.$refs.tools.SaveCVS('Distribution.csv', Data);
      }
    },
    components: { Spinner, ToolbarYears, ToolbarTools, Calendar, Months, Day, ColorBar }
  }
</script>

<style scoped>
  .header-text {
    font-size: 24px;
    line-height: 16px;
  }
</style>