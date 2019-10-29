<template>
  <div id="wrapper">
    <div class="row">
      <div class="col-lg-12">
        <div class="panel panel-other">
          <div class="panel-heading">
            <span class="header-text">ITIC Curve (CBEMA)</span>
            <div class="header-buttons">
              <toolbar-years v-on:YearChanged="YearChanged" aggreg="1H"></toolbar-years>
            </div>
          </div>
          <div class="panel-body">
            <spinner v-if="ShowSpinner"></spinner>
            <curve ref="itic"></curve>
          </div>
        </div>
      </div>
    </div>
    <div class="row">
      <div class="col-xs-12 col-md-6"><spinner v-if="ShowSpinner"></spinner><by-date ref="date"></by-date></div>
      <div class="col-xs-12 col-md-6"><spinner v-if="ShowSpinner"></spinner><by-type ref="type"></by-type></div>
      <div class="col-xs-12 col-md-6"><spinner v-if="ShowSpinner"></spinner><by-rms ref="rms"></by-rms></div>
      <div class="col-xs-12 col-md-6"><spinner v-if="ShowSpinner"></spinner><by-duration ref="duration"></by-duration></div>
    </div>
  </div>
</template>

<script>
  import ToolbarYears from 'Components/ToolbarYears.vue';
  import Spinner from 'Components/Spinner.vue';
  import Curve from './Curve.vue';
  import ByDate from './ByDate.vue';
  import ByType from './ByType.vue';
  import ByRms from './ByRMS.vue';
  import ByDuration from './ByDuration.vue';
  import Graph from 'Components/Graph.vue';
  import moment from 'JS/moment.js';
  export default {
    data: function() {
      return { ShowSpinner: true };
    },
    created: function() {
      
    },
        beforeRouteEnter (to, from, next) {
      console.log(to, from);
      next();
    // called before the route that renders this component is confirmed.
    // does NOT have access to `this` component instance,
    // because it has not been created yet when this guard is called!
    },
    methods: {
      ProcessRange: function(start, end) {
        this.$refs.date.SetData(start, end, this.Events);
        this.$refs.type.SetData(start, end, this.Events);
        this.$refs.rms.SetData(start, end, this.Events);
        this.$refs.duration.SetData(start, end, this.Events);
        this.$refs.itic.SetData(start, end, this.Events);
        this.ShowSpinner = false;
      },
      YearChanged: function(year) {
        this.Year = year;
        this.ShowSpinner = true;
        if(this.$root.SelectedAnalyzer === null) return;
        var Analyzer = this.$root.SelectedAnalyzer;
        var Year = moment().year(year);
        $.ajax({ url: "getEvents", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify( { Period: Year.endOf('year').valueOf() - Year.startOf('year').valueOf(), To: Year.endOf('year').valueOf(), LastSample: 0, Analyzer: Analyzer.Analyzer } ),
          success: function(result) {
            this.Events = result;
            this.ProcessRange(Year.startOf('year').valueOf(), Year.endOf('year').valueOf());
          }.bind(this)
        });
      }
    },
    components: { Spinner, ToolbarYears, Graph, Curve, ByDate, ByType, ByRms, ByDuration }
  }
</script>

<style scoped>
  .header-text {
    font-size: 24px;
    line-height: 16px;
  }
</style>