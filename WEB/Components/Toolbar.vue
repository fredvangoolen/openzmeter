<template>
  <div class="toolbar">
    <div class="btn-group" v-for="item in options">
      <div class="btn-group" v-if="(item.mode == 'Phases') & (displayPhases == true)">
        <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Show phase R" v-bind:class="{hide:displayPhase('R')}"><img src="Imgs/phase_r.png"></button>
        <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Show phase S" v-bind:class="{hide:displayPhase('S')}"><img src="Imgs/phase_s.png"></button>
        <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Show phase T" v-bind:class="{hide:displayPhase('T')}"><img src="Imgs/phase_t.png"></button>
        <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Show phase N" v-bind:class="{hide:displayPhase('N')}"><img src="Imgs/phase_n.png"></button>
      </div>
      <div class="btn-group toolbar-aggregation" v-if="item.mode == 'Aggreg'">
        <div class="input-group input-group-xs">
          <span class="input-group-addon"><img src="Imgs/clock_15.png"/></span>
          <div class="input-group-btn">
            <button class="btn btn-default dropdown-toggle" type="button" data-toggle="dropdown"><span>{{ SelectedText }}</span><span class="caret"></span></button>
            <ul class="dropdown-menu pull-right">
              <li><a href="#">3 seconds</a></li>
              <li><a href="#">1 minute</a></li>
              <li><a href="#">10 minute</a></li>
              <li><a href="#">15 minutes</a></li>
              <li><a href="#">1 hour</a></li>
            </ul>
          </div>
        </div>
      </div>
      <div class="btn-group toolbar-time" v-if="item.mode == 'Period'">
        <div class="input-group input-group-xs">
          <span class="input-group-addon"><img src="Imgs/outlook_calendar_work_week.png"></span>
          <div class="input-group-btn">
            <button class="btn btn-default dropdown-toggle" type="button" data-toggle="dropdown"><span>{{ SelectedText }}</span><span class="caret"></span></button>
            <ul class="dropdown-menu pull-right">
              <li><a href="#">Day</a></li>
              <li><a href="#">Week</a></li>
              <li><a href="#">Month</a></li>
              <li><a href="#">Year</a></li>
            </ul>
          </div>
        </div>
      </div>
      <div class="btn-group" v-if="item.mode == 'Tools'">
        <button type="button" class="btn btn-default btn-xs" data-function="Export" data-toggle="tooltip" title="Export data to CSV file"><img src="Imgs/table_export.png"></button>
        <button type="button" class="btn btn-default btn-xs" data-function="Maximize" data-toggle="tooltip" title="Maximize graph"><img src="Imgs/arrow_out.png"></button>
      </div>
    </div>
  </div>
</template>

<script>
  export default {
    //Mode: 'Period' -> options: ['Day', 'Week', 'Month', 'Year']
    //Mode: 'Aggreg' -> options: ['3S', '1M', '10M', '15M', '1H']
    //Mode: 'Phases' -> options: ['R', 'S', 'T', 'N'], 'VOLTAGE' or 'CURRENT'
    //Mode: 'Tools'  -> options: ['Export', 'Maximize']
    props: ['options'],
    data() {
      return { SelectedText: '1 hour' };
    },
    computed: {
      displayPhases() {
        if(this.$root.SelectedAnalyzer === null) return false;
        if(this.$root.SelectedAnalyzer.Mode === '1PHASE') return false;
        return true;
      },
      displayPhase(phase) {
        return true;
      }
    },
    methods: {
      
    }
  }
</script>

<style>
  .toolbar {
    display: inline-block;
  }
  .toolbar-aggregation button span:first-child {
    display: inline-block; 
    width: 56px; 
    text-align: left;
  }
</style>
