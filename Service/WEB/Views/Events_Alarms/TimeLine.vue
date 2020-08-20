<template>
  <div></div>
</template>

<script>
  import moment from 'moment';
  import vis from 'vis';
//  import 'CSS/vis.css';
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
        var Counts = [];
        var Colors = ['green', 'yellow', 'red'];
        if((this.alarms == null) || (this.analyzer == null)) return;
        var NameCol    = this.alarms.Headers.indexOf('Name');
        var TriggerCol = this.alarms.Headers.indexOf('Trigger');
        var ReleaseCol = this.alarms.Headers.indexOf('Release');
        for(var n = 0; n < this.alarms.Rows.length; n++) {
          var Index = Groups.indexOf(this.alarms.Rows[n][NameCol]);
          if(Index == -1) {
            Groups.push(this.alarms.Rows[n][NameCol]);
            Counts.push(0);
            Index = Groups.length - 1;
          }
          Counts[Index] = Counts[Index] + 1;
          var Row = { start: this.alarms.Rows[n][TriggerCol], className: Colors[0], group: Index, end: (this.alarms.Rows[n][ReleaseCol] == null) ? moment().valueOf() : this.alarms.Rows[n][ReleaseCol] };
          this.TimeLineDataset.add(Row);
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