<template>
  <div class="panel panel-reactive-power" id="ReactivePowerPanel">
    <div class="panel-heading" id="ReactivePowerPanel">
      <div class="header-text"><i class="fa fa-area-chart fa-fw"></i> Reactive power</div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" id="btnExportReactivePower"   data-toggle="tooltip" title="Export data to CSV file"><i class="fa fa-table fa-fw"></i></button>
        <button type="button" class="btn btn-default btn-xs" id="btnMaximizeReactivePower" data-toggle="tooltip" title="Maximize graph"><i class="fa fa-expand"></i></button>
      </div>
    </div>
    <div class="panel-body">
      <spinner v-bind:class="{ 'hide': !ShowSpinner }"></spinner>
      <div class="row">
        <div class="col-md-6 col-lg-4 col-xl-3" ref="counters">
          <div class="subpanel">
            <div class="counters-title">Last value:</div>
            <div class="counters-aggreg">From 3s seconds aggregation</div>
            <div class="clearfix"></div>
            <div class="current-value" v-html="CurrentValue3S"></div>
          </div>
          <div class="subpanel">
            <div class="counters-title">Last value:</div>
            <div class="counters-aggreg">From 1 minute aggregation</div>
            <div class="clearfix"></div>
            <div class="current-value" v-html="CurrentValue1M"></div>
          </div>
          <div class="subpanel">
            <div class="counters-title">Last value:</div>
            <div class="counters-aggreg">From 15 minutes aggregation</div>
            <div class="clearfix"></div>
            <div class="current-value" v-html="CurrentValue15M"></div>
          </div>
          <div class="subpanel">
            <div class="counters-title">Last value:</div>
            <div class="counters-aggreg">From 1 hour aggregation</div>
            <div class="clearfix"></div>
            <div class="current-value" v-html="CurrentValue1H"></div>
          </div>
          <div class="subpanel clearfix">
            <div class="counters-title">Period stats:</div>
            <div class="counters-aggreg">{{ CountersAggregation }}</div>
            <div class="line">
              <div class="float-left">
                <div class="counters-stat-type">Min value:</div>
                <div class="counters-stat-time">{{ MinTime }}</div>
              </div>
              <div class="counters-stat-val" v-html="MinValue"></div>
            </div>
            <div class="line line-border">
              <div class="float-left">
                <div class="counters-stat-type">Max value:</div>
                <div class="counters-stat-time">{{ MaxTime }}</div>
              </div>
              <div class="counters-stat-val" v-html="MaxValue"></div>
            </div>
          </div>
        </div>
        <div class="col-md-6 col-lg-8 col-xl-9">
          <graph ref="graph" :options="GraphOptions"></graph>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
  import ToolbarAggregation from 'Components/ToolbarAggregation.vue';
  import ToolbarPeriod from 'Components/ToolbarPeriod.vue';
  import ToolbarTools from 'Components/ToolbarTools.vue';
  import Spinner from 'Components/Spinner.vue';
  import Graph from 'Components/Graph.vue';
  import moment from 'JS/moment.js';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    data () {
      return { };
    },
    components: { Spinner, ToolbarAggregation, ToolbarPeriod, ToolbarTools, Graph }
  }

</script>