<template>
  <div class="row">
    <div class="col-lg-12">
      <div class="panel panel-other">
        <div class="panel-heading">
          <span class="header-text">Disturbances</span>
          <div class="header-buttons clearfix" ref="Toolbar">
            <toolbar-period variable="Period" v-on:PeriodChanged="GetEvents" ref="period"></toolbar-period>
          </div>
        </div>
        <div class="panel-body">
          <spinner v-if="ShowSpinner"></spinner>
          <div id="disturbances_chart"></div>
        </div>
      </div>
    </div>
    <div class="col-lg-12">
      <events-table ref="table" v-on:UpdateComplete="UpdateComplete"></events-table>
    </div>
  </div>
</template>

<script>
  import vis from 'JS/vis.js';
  import 'CSS/vis.css';
  import 'CSS/jquery.qtip.css';
  import qtip from 'JS/jquery.qtip.js';
  import Spinner from 'Components/Spinner.vue';
  import ToolbarPeriod from 'Components/ToolbarPeriod.vue';
  import EventsTable from '../Analysis_Voltage/Events.vue';
  export default {
    props: { analyzer: { type: Object } },
    data: function() {
      return { ShowSpinner: true };
    },
    created: function() {
      this.TimeLine = null;
      console.log(this. $route);
    },
    mounted: function() {
      this.TimeLine = new vis.Timeline(document.getElementById('disturbances_chart'), null, {
        stack: false,
        editable: false,
        margin: { item: 10, axis: 5 },
        orientation: 'top'
      });
      this.TimeLine.on('rangechanged', this.WindowChange.bind(this));
      var Groups = new vis.DataSet([
        { id: 1, content: 'Dips'},
        { id: 2, content: 'Swells'},
        { id: 3, content: 'Interruptions'},
        { id: 4, content: 'RVC'},
        { id: 5, content: 'Glitchs'}
      ]);
      this.TimeLine.setGroups(Groups);
      this.$refs.period.SetAggregation('1H');
      this.TimeLine.setItems([]);
      $(document).on("mouseover", '.vis-item', this.OnMouseOver);
    },
    methods: {
      WindowChange: function(evt) {
        if(evt.byUser === false) return;
        this.$refs.period.SetCustom(evt.start.getTime(), evt.end.getTime(), false);
      },
      UpdateComplete: function() {
        this.$refs.period.UpdateCompleted(this.$refs.table.LastSample);
        var TimeLineDataset = new vis.DataSet();
        for(var i = 0; i < this.$refs.table.Events.length; i++) {    
          var Item = { 
            id: i, 
            start: this.$refs.table.Events[i].StartTime,
            content: '<table>' +
                     '<tr><td><strong>Time</strong></td><td> : ' + this.$refs.table.Events[i].Date + '</td></tr>' +
                     '<tr><td><strong>Duration</strong></td><td> : ' + this.$refs.table.Events[i].Duration + '</td></tr>' +
                     '<tr><td><strong>ΔU<sub>max</sub></strong></td><td> : ' + this.$refs.table.Events[i].Change + ' V</td></tr>' +
                     '<tr><td><strong>% ΔU<sub>max</sub></strong></td><td> : ' + this.$refs.table.Events[i].Percent + '%</td></tr>' + 
                     '</table>'
          };
          if(this.$refs.table.Events[i].Type == 'INTERRUPTION') {
            Item.className = 'red';
            Item.group = 3;
          } else if(this.$refs.table.Events[i].Type == 'SWELL') {
            Item.className = 'blue';
            Item.group = 2;
          } else if(this.$refs.table.Events[i].Type == 'DIP') {
            Item.className = 'blue';
            Item.group = 1;
          } else if(this.$refs.table.Events[i].Type == 'RVC') {
            Item.className = 'blue';
            Item.group = 4;
          }
          TimeLineDataset.add(Item);
        }
        this.TimeLine.setItems(TimeLineDataset);
        this.TimeLine.setWindow(this.$refs.table.DateWindow[0], this.$refs.table.DateWindow[1]);
        this.ShowSpinner = false;
      },
      GetEvents: function(to, period, lastsample) {
        if(lastsample === 0) this.ShowSpinner = true;
        this.TimeLine.setWindow(to - period, to);
        this.$refs.table.GetEvents(to, period, lastsample);
      },
      OnMouseOver: function($e) {
        $($e.target).qtip({
          overwrite: false,
            content: {
              text: $('.vis-item-content', $e.target).html()
            },
            position: { my: 'top left', at: 'bottom left' },
            style: { classes: 'qtip-bootstrap' }
        });
        $($e.target).qtip().show();
      }
    },
    components: { Spinner, ToolbarPeriod, EventsTable }
  }
</script>

<style scoped>
  .header-text {
    font-size: 24px;
    line-height: 16px;
  }
</style>
