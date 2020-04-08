<template>
  <div></div>
</template>

<script>
  import moment from 'moment';
  import vis from 'JS/vis.js';
  import 'CSS/vis.css';
  import 'CSS/jquery.qtip.css';
  import qtip from 'JS/jquery.qtip.js';
  export default {
    props: { analyzer: Object, alarms: Object },
    data: function() {
      return { };
    },
    created: function() {
      this.TimeLine = null;
      this.TimeLineDataset = new vis.DataSet();
      this.TimeLineGroups = new vis.DataSet();
    },
    mounted: function() {
      this.TimeLine = new vis.Timeline(this.$el, this.TimeLineDataset, {
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
        var Groups = [];
        var StartDate = [];
        var Counts = [];
        var Colors = ['green', 'yellow', 'red'];
        if((this.alarms == null) || (this.analyzer == null)) return;
        for(var n = 0; n < this.analyzer.Alarms.length; n++) {
          Groups.push(this.analyzer.Alarms[n].Name);
          StartDate.push(null);
          Counts.push(0);
        }
        for(var n = 0; n < this.alarms.AlarmTime.length; n++) {
          var Index = Groups.indexOf(this.alarms.Name[n]);
          if(Index == -1) continue;
          if((this.alarms.Triggered[n] == false) && (StartDate[Index] == null)) {
            Counts[Index] = Counts[Index] + 1;
            this.TimeLineDataset.add({ start: this.alarms.From, className: Colors[this.alarms.Priority[n]], group: Index, end: this.alarms.AlarmTime[n] });
          } else if((this.alarms.Triggered[n] == false) && (StartDate[Index] != null)) {
            this.TimeLineDataset.add({ start: StartDate[Index], className: Colors[this.alarms.Priority[n]], group: Index, end: this.alarms.AlarmTime[n] });
            StartDate[Index] = null;
            Counts[Index] = Counts[Index] + 1;
          } else if((this.alarms.Triggered[n] == true) && (StartDate[Index] != null)) {
            continue;
          } else if((this.alarms.Triggered[n] == true) && (StartDate[Index] == null)) {
            StartDate[Index] = this.alarms.AlarmTime[n];
          }
        }
        for(var n = 0; n < Groups.length; n++) {
          if(StartDate[n] != null) this.TimeLineDataset.add({ start: StartDate[n], className: Colors[this.alarms.Priority[n]], group: n, end: this.alarms.To });
        }
        this.TimeLine.setWindow(this.alarms.From, this.alarms.To);
        for(var n = 0; n < Groups.length; n++) {
          this.TimeLineGroups.add({ id: n, content: '<div class="fixed_container"><div class="overflow_container" data-toggle="tooltip" title="' + Groups[n] + '">' + Groups[n] + '</div><span class="badge">' + Counts[n] + '</span></div>'});
        }
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
  div >>> .fixed_container {
    width: 200px;
    line-height: 18px;
  }
  div >>> .badge {
    margin-top: 0px;
    float: right;
  }
  div >>> .overflow_container {
    width: 150px;
    text-overflow: ellipsis;
    display: inline-block;
    white-space: nowrap;
    overflow: hidden;
  }
</style>