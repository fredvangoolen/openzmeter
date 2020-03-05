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
    props: { analyzer: Object, events: Object },
    data: function() {
      return { };
    },
    created: function() {
      this.TimeLine = null;
      this.TimeLineDataset = new vis.DataSet();
    },
    mounted: function() {
      this.TimeLine = new vis.Timeline(this.$el, this.TimeLineDataset, {
        stack: false,
        editable: false,
        margin: { item: 10, axis: 5 },
        orientation: 'top'
      });
      this.TimeLine.on('rangechanged', (evt) => { if(evt.byUser == true) this.$emit('update:range', [moment(evt.start).valueOf(), moment(evt.end).valueOf()]) } );
      var Groups = new vis.DataSet([
        { id: 1, content: 'Dips'},
        { id: 2, content: 'Swells'},
        { id: 3, content: 'Interruptions'},
        { id: 4, content: 'RVC'},
        { id: 5, content: 'Glitchs'}
      ]);
      this.TimeLine.setGroups(Groups);
      $(document).on("mouseover", '.vis-item', (e) => {
        $(e.target).qtip({
          overwrite: false,
            content: {
              text: $('.vis-item-content', e.target).html()
            },
            position: { my: 'top left', at: 'bottom left' },
            style: { classes: 'qtip-bootstrap' }
        });
        $(e.target).qtip().show();
      });
      this.TimeLine.setWindow(moment().subtract(1, 'day').valueOf(), moment().valueOf());
      if(this.events != null) this.SetEvents();
    },
    watch: {
      events: function() {
        if(this.TimeLine != null) this.SetEvents();
      }
    },
    methods: {
      SetEvents: function() {
        this.TimeLineDataset.clear();
        if(this.events == null) return;
        for(var i = 0; i < this.events.StartTime.length; i++) {
          var Item = {
            id: i,
            start: this.events.StartTime[i],
            content: '<table>' +
                     '<tr><td><strong>Time</strong></td><td> : ' + this.analyzer.FormatDateTime(this.events.StartTime[i]) + '</td></tr>' +
//                     '<tr><td><strong>Duration</strong></td><td> : ' + this.$refs.table.Events[i].Duration + '</td></tr>' +
//                     '<tr><td><strong>ΔU<sub>max</sub></strong></td><td> : ' + this.$refs.table.Events[i].Change + ' V</td></tr>' +
//                     '<tr><td><strong>% ΔU<sub>max</sub></strong></td><td> : ' + this.$refs.table.Events[i].Percent + '%</td></tr>' +
                     '</table>'
          };
          if(this.events.Type[i] == 'INTERRUPTION') {
            Item.className = 'red';
            Item.group = 3;
          } else if(this.events.Type[i] == 'SWELL') {
            Item.className = 'blue';
            Item.group = 2;
          } else if(this.events.Type[i] == 'DIP') {
            Item.className = 'blue';
            Item.group = 1;
          } else if(this.events.Type[i] == 'RVC') {
            Item.className = 'blue';
            Item.group = 4;
          }
          this.TimeLineDataset.add(Item);
        }
        this.TimeLine.setWindow(this.events.From, this.events.To);
      }
    }
  }
</script>