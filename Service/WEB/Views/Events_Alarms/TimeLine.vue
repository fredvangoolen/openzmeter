<template>
  <div></div>
</template>

<script>
//  import moment from 'moment';
//  import { DataSet } from "vis-data/peer";
//  import { Timeline } from "vis-timeline/peer";
//  import "vis-timeline/styles/vis-timeline-graph2d.css";
  export default {
    props: { alarms: Object },
    data: function() {
      return { };
    },
    created: function() {
      this.TimeLine = null;
      this.TimeLineDataset = new DataSet();
      this.TimeLineGroups = new DataSet();
    },
    mounted: function() {
      this.TimeLine = new Timeline(this.$el, this.TimeLineDataset, {
        stack: false,
        editable: false,
        margin: { item: 2, axis: 2 },
        orientation: 'top'
      });
      this.TimeLine.on('rangechanged', (evt) => { if(evt.byUser == true) this.$emit('update:range', [moment(evt.start).valueOf(), moment(evt.end).valueOf()]) } );
      this.TimeLine.setWindow(moment().subtract(1, 'day').valueOf(), moment().valueOf());
      this.SetAlarms();
    },
    watch: {
      alarms: function() { if(this.TimeLine != null) this.SetAlarms(); },
      analyzer: function() { if(this.TimeLine != null) this.SetAlarms(); },
    },
    methods: {
      SetAlarms: function() {
        this.TimeLineGroups.clear();
        this.TimeLineDataset.clear();
        var IDs   = [];
        var Names = [];
        var Count = [];
        var Prio  = [];
        var Colors = ['green', 'yellow', 'red'];
        if((this.alarms == null) || (this.analyzer == null)) return;
        for(var n = 0; n < this.analyzer.Alarms.length; n++) {
          IDs.push(this.analyzer.Alarms[n].AlarmID);
          Names.push(this.analyzer.Alarms[n].Name);
          Count.push(0);
          Prio.push(this.analyzer.Alarms[n].Priority);
        }
        var IDCol      = this.alarms.Headers.indexOf('AlarmID');
        var TriggerCol = this.alarms.Headers.indexOf('Trigger');
        var ReleaseCol = this.alarms.Headers.indexOf('Release');
        for(var n = 0; n < this.alarms.Rows.length; n++) {
          var Index = IDs.indexOf(this.alarms.Rows[n][IDCol]);
          Count[Index] = Count[Index] + 1;
          this.TimeLineDataset.add({ start: this.alarms.Rows[n][TriggerCol], className: Colors[Prio[Index]], group: IDs[Index], end: (this.alarms.Rows[n][ReleaseCol] == null) ? this.alarms.Rows[n][TriggerCol] : this.alarms.Rows[n][ReleaseCol] });
        }
        this.TimeLine.setWindow(this.alarms.From, this.alarms.To);
        for(var n = 0; n < IDs.length; n++) this.TimeLineGroups.add({ id: IDs[n], title: Names[n], content: '<div><div>' + Names[n] + '</div><span class="badge">' + Count[n] + '</span></div>'});
        this.TimeLine.setGroups(this.TimeLineGroups);
      }
    }
  }
</script>

<style scoped>
  div >>> .vis-item.red {
    border-color: #d9534f;
    background-color: #E07572;
  }
  div >>> .vis-item.yellow {
    border-color: #f0ad4e;
    background-color: #f3bd71;
  }
  div >>> .vis-item.green {
    border-color: #5cb85c;
    background-color: #7cc67c;
  }
  div >>> .vis-inner > div {
    width: 200px;
    line-height: 18px;
  }
  div >>> .vis-inner > div > span {
    margin-top: 0px;
    float: right;
    min-width: 10px;
    padding: 3px 7px;
    font-size: 12px;
    font-weight: bold;
    line-height: 1;
    color: #fff;
    text-align: center;
    white-space: nowrap;
    vertical-align: middle;
    background-color: #777777;
    border-radius: 10px;
  }
  div >>> .vis-inner > div > div {
    width: 150px;
    text-overflow: ellipsis;
    display: inline-block;
    white-space: nowrap;
    overflow: hidden;
  }
</style>