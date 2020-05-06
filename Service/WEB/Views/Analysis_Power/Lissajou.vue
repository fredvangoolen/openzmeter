<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">{{ (data==null) ? 'Loading data...' : 'Last update at ' + analyzer.FormatTimeMS(data.Sampletime) }} {{ paused ? '(Paused)' : MouseOver ? '(Graph paused)' : '' }}</div>
      <div v-if="RmsCurrent<analyzer.MinCurrent" class="title-warning"><img :src="require('Imgs/exclamation.png')">No current or current too low</div>
      <portal to="fs-toolbar" :disabled="!Expanded" class="header-buttons">
        <toolbar-phases storage="DetailsPhases" :multi="false" :analyzer="analyzer" v-model="Phase"></toolbar-phases>
        <div class="btn-group">
          <button-export file="Lissajou" :rows="Export"></button-export>
          <button-expand v-model="Expanded"></button-expand>
          <button-pause :value="paused" @input="$emit('update:paused', $event)"></button-pause>
        </div>
      </portal>
    </div>
    <portal to="fs-body" :disabled="!Expanded" class="panel-body">
      <spinner v-if="data==null"></spinner>
      <div @mouseenter="MouseOver=true" @mouseleave="MouseOver=false" ref="Container" style="text-align: center">
        <graph :options="GraphOptions" :height="Expanded?DialogHeight:Height" :width="Expanded?DialogHeight:Height" :paused="MouseOver"></graph>
      </div>
    </portal>
    <dlg-fs v-if="Expanded" title="Lissajou" :height.sync="DialogHeight"></dlg-fs>
  </div>
</template>

<script>
  import Graph from 'Components/Graph.vue';
  export default {
    props: { analyzer: Object, data: Object, paused: Boolean },
    data() {
      return { MouseOver: false, Phase: null, Expanded: false, Height: 300, DialogHeight: 300, CurrentMult: 1.0 };
    },
    mounted: function() {
      window.addEventListener('resize', this.Resize);
      this.$nextTick(() => { window.dispatchEvent(new Event('resize')); });
    },
    computed: {
      RmsCurrent: function() { return (this.data == null) ? 0.0 : this.ArrayAbsMax(this.data.Current); },
      GraphOptions: function() {
        var Opt = {file: [], axes: { y: {}, x: {} } };
        Opt.interactionModel = Graph.InteractionModels.Fixed(this);
        Opt.legendFormatter = Graph.Formatters.Legend_Follow;
        Opt.underlayCallback = Graph.Callbacks.Underlay_DrawBorder;
        Opt.drawAxesAtZero = true;
        Opt.legend = 'follow';
        Opt.selectMode = 'euclidian';
        Opt.showRangeSelector = false;
        Opt.strokeWidth = 0.5;
        Opt.labels = ['Voltage', 'Current'];
        Opt.colors = [this.analyzer.Color.Voltage, this.analyzer.Color.Current, this.analyzer.Color.Active_Power].concat(this.analyzer.VoltageChannels > 1 ? [this.analyzer.Color.Active_Power] : []);
        Opt.axes.y.pixelsPerLabel = 15;
        Opt.axes.y.axisLabelWidth = 34;
        if((this.Phase != null) && (this.data != null)) {
          for(var i = 0; i < this.data.Voltage_Samples[0].length; i++) {
            var Row = [this.data.Voltage_Samples[this.Phase][i], this.data.Current_Samples[this.Phase][i]];
            Opt.file.push(Row);
          }
          var VoltageRange = this.MatrixColsRange(Opt.file, [ true], { defMin: 0, defMax: 1, percent: 0.05, minWidth: 10 });
          var CurrentRange = this.MatrixColsRange(Opt.file, [false, true], { defMin: 0, defMax: 1, percent: 0.05, minWidth: 10 });
          var MaxVoltage = Math.max(Math.abs(VoltageRange[0]), Math.abs(VoltageRange[1]));
          var MaxCurrent = Math.max(Math.abs(CurrentRange[0]), Math.abs(CurrentRange[1]));
          Opt.axes.y.valueRange = [-MaxCurrent, MaxCurrent];
          Opt.dateWindow = [-MaxVoltage, MaxVoltage];
        }
        return Object.seal(Opt);
      },
    },
    methods: {
      Resize: function() {
        this.Height = Math.min(600, this.$refs.Container.clientWidth - 30);
      },
      Export: function() {
        if((this.data == null) || (this.analyzer == null)) return [];
        var Header = ['Sample']
          .concat(this.analyzer.GetSeriesNames('Voltage', this.analyzer.VoltageChannels))
          .concat(this.analyzer.GetSeriesNames('Current', this.analyzer.VoltageChannels))
          .concat(this.analyzer.GetSeriesNames('Power', this.analyzer.VoltageChannels));
        var Data = [Header];
        for(var i = 0; i < this.data.Voltage_Samples[0].length; i++) {
          var Row = [i];
          for(var n = 0; n < this.data.Voltage_Samples.length; n++) Row.push(this.data.Voltage_Samples[n][i]);
          for(var n = 0; n < this.data.Voltage_Samples.length; n++) Row.push(this.data.Current_Samples[n][i]);
          for(var n = 0; n < this.data.Voltage_Samples.length; n++) Row.push(this.data.Voltage_Samples[n][i] * this.data.Current_Samples[n][i]);
          Data.push(Row);
        }
        return Object.seal(Data);
      }
    }
  }
</script>

<style scoped>
  .header-text {
    font-size: 14px;
  }
</style>