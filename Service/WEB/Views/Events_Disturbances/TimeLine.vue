<template>
  <div></div>
</template>

<script>
  import moment from 'moment';
  import { DataSet } from "vis-data/peer";
  import { Timeline } from "vis-timeline/peer";
  import "vis-timeline/styles/vis-timeline-graph2d.css";
//  import 'CSS/jquery.qtip.css';
//  import qtip from 'JS/jquery.qtip.js';
  export default {
    props: { analyzer: Object, events: Object },
    data: function() {
      return { };
    },
    created: function() {
      this.TimeLine = null;
      this.TimeLineDataset = new DataSet();
      this.TimeLineGroups = new DataSet();
    },
    mounted: function() {
      this.TimeLineGroups.add({ id: 1, content: '<div style="width:150px">Dips</div>'});
      this.TimeLineGroups.add({ id: 2, content: '<div style="width:150px">Swells</div>'});
      this.TimeLineGroups.add({ id: 3, content: '<div style="width:150px">Interruptions</div>'});
      this.TimeLineGroups.add({ id: 4, content: '<div style="width:150px">RVC</div>'});
      this.SetEvents();
      this.TimeLine = new Timeline(this.$el, this.TimeLineDataset, {
        stack: false,
        editable: false,
        margin: { item: 10, axis: 5 },
        orientation: 'top',
        tooltip: {
      followMouse: true,
      overflowMethod: 'cap'
    }
      });
      this.TimeLine.setGroups(this.TimeLineGroups);
//      $(document).on("mouseover", '.vis-item', (e) => {
//        $(e.target).qtip({
//          overwrite: false,
//            content: {
//              text: $('.vis-item-content', e.target).html()
//            },
//            position: { my: 'top left', at: 'bottom left' },
//            style: { classes: 'qtip-bootstrap' }
//        });
//        $(e.target).qtip().show();
//      });
      this.TimeLine.on('rangechanged', (evt) => { if(evt.byUser == true) this.$emit('update:range', [moment(evt.start).valueOf(), moment(evt.end).valueOf()]) } );
      if(this.events == null)
        this.TimeLine.setWindow(moment().subtract(1, 'day').valueOf(), moment().valueOf());
      else
        this.TimeLine.setWindow(this.events.From, this.events.To);
    },
    watch: {
      events: function() {
        if((this.TimeLine != null) && (this.events != null)) {
          this.SetEvents();
          this.TimeLine.setWindow(this.events.From, this.events.To);
        }
      }
    },
    methods: {
      SetEvents: function() {
        this.TimeLineDataset.clear();
        if(this.events == null) return;
        var Count = {Interruption: 0, Swell: 0, Dip: 0, RVC: 0 };
        var StartTimeCol  = this.events.Headers.indexOf("StartTime");
        var SampleRateCol = this.events.Headers.indexOf("SampleRate");
        var DurationCol   = this.events.Headers.indexOf("Duration");
        var TypeCol       = this.events.Headers.indexOf("Type");
        for(var i = 0; i < this.events.Rows.length; i++) {
          var Item = {
            id: i,
            start: this.events.Rows[i][StartTimeCol],
            end: this.events.Rows[i][StartTimeCol] + this.events.Rows[i][DurationCol] / (this.events.Rows[i][SampleRateCol] / 1000),
            title: '<table>' +
                     '<tr><td><strong>Time</strong></td><td> : ' + this.analyzer.FormatDateTime(this.events.Rows[i][StartTimeCol]) + '</td></tr>' +
                     '<tr><td><strong>Duration</strong></td><td> : ' + this.Round(this.events.Rows[i][DurationCol] / this.events.Rows[i][SampleRateCol], 2) + ' sec</td></tr>' +
//                     '<tr><td><strong>ΔU<sub>max</sub></strong></td><td> : ' + this.$refs.table.Events[i].Change + ' V</td></tr>' +
//                     '<tr><td><strong>% ΔU<sub>max</sub></strong></td><td> : ' + this.$refs.table.Events[i].Percent + '%</td></tr>' +
                     '</table>'
          };
          if(this.events.Rows[i][TypeCol] == 'INTERRUPTION') {
            Item.className = 'red';
            Item.group = 3;
            Count.Interruption = Count.Interruption + 1;
          } else if(this.events.Rows[i][TypeCol] == 'SWELL') {
            Item.className = 'blue';
            Item.group = 2;
            Count.Swell = Count.Swell + 1;
          } else if(this.events.Rows[i][TypeCol] == 'DIP') {
            Item.className = 'blue';
            Item.group = 1;
            Count.Dip = Count.Dip + 1;
          } else if(this.events.Rows[i][TypeCol] == 'RVC') {
            Item.className = 'blue';
            Item.group = 4;
            Count.RVC = Count.RVC + 1;
          }
          this.TimeLineDataset.add(Item);
        }
        this.TimeLineGroups.update({ id: 1, content: '<div style="width:150px">Dips<span class="pull-right badge">' + Count.Dip + '</span></div>'});
        this.TimeLineGroups.update({ id: 2, content: '<div style="width:150px">Swells<span class="pull-right badge">' + Count.Swell + '</span></div>'});
        this.TimeLineGroups.update({ id: 3, content: '<div style="width:150px">Interruptions<span class="pull-right badge">' + Count.Interruption + '</span></div>'});
        this.TimeLineGroups.update({ id: 4, content: '<div style="width:150px">RVC<span class="pull-right badge">' + Count.RVC + '</span></div>'});
      }
    }
  }
</script>