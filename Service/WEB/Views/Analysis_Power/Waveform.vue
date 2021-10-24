<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">{{ (data==null) ? 'Loading data...' : 'Last update at ' + analyzer.FormatTimeMS(data.Sampletime) }} {{ paused ? '(Paused)' : MouseOver ? '(Graph paused)' : '' }}</div>
      <div v-if="RmsCurrent<analyzer.MinCurrent" class="title-warning"><img :src="require('Imgs/exclamation.png')">No current or current too low</div>
      <portal to="fs-toolbar" :disabled="!Expanded" class="header-buttons">
        <toolbar-phases storage="DetailsPhases" :multi="false" :analyzer="analyzer" v-model="Phase"></toolbar-phases>
        <div class="btn-group">
          <button-export file="Waveform" :rows="Export"></button-export>
          <button-expand v-model="Expanded"></button-expand>
          <button-pause :value="paused" @input="$emit('update:paused', $event)"></button-pause>
        </div>
      </portal>
    </div>
    <portal to="fs-body" :disabled="!Expanded" class="panel-body">
      <spinner v-if="data==null"></spinner>
      <div @mouseenter="MouseOver=true" @mouseleave="MouseOver=false">
        <graph :options="GraphOptions" :height="Expanded?DialogHeight:300" :paused="MouseOver"></graph>
      </div>
    </portal>
    <dlg-fs v-if="Expanded" title="Waveform" :height.sync="DialogHeight"></dlg-fs>
  </div>
</template>

<script>
  import Graph from 'Components/Graph.vue';
  export default {
    props: { data: Object, paused: Boolean },
    data: function() {
      return { MouseOver: false, Phase: null, Expanded: false, DialogHeight: 300, CurrentMult: 1.0 };
    },
    computed: {
      RmsCurrent: function() { return (this.data == null) ? 0.0 : this.ArrayAbsMax(this.data.Current); },
      GraphOptions: function() {
        var Opt = {file: [], axes: { y: {}, y2: {}, x: {} } };
        Opt.plugins = [ Graph.Plugins.Vertical_Crosshair() ];
        Opt.interactionModel = Graph.InteractionModels.Fixed(this);
        Opt.legendFormatter = Graph.Formatters.Legend_Follow;
        Opt.underlayCallback = Graph.Callbacks.Underlay_DrawBorder;
        Opt.drawAxesAtZero = true;
        Opt.gridLineColor = "#DDD";
        Opt.gridLinePattern = [4, 4];
        Opt.legend = 'follow';
        Opt.showRangeSelector = false;
        Opt.strokeWidth = 0.5;
        Opt.series = { 'Voltage': { axis: 'y2' }, 'Current': { axis: 'y2' }, 'Power': { fillGraph: true }, 'Power SUM': { axis: 'y', strokeWidth: 2 } };
        Opt.labels = ['Sample', 'Voltage', 'Current', 'Power'].concat(this.analyzer.VoltageChannels > 1 ? ['Power SUM'] : []);
        Opt.colors = [this.analyzer.SerieColor('Voltage'), this.analyzer.SerieColor('Current'), this.analyzer.SerieColor('Active_Power')].concat(this.analyzer.PhaseNames.length > 1 ? [this.analyzer.SerieColor('Active_Power')] : []);
        Opt.axes.y.pixelsPerLabel = 15;
        Opt.axes.y.axisLabelWidth = 34;
        Opt.axes.y2.pixelsPerLabel = 15;
        Opt.axes.y2.axisLabelWidth = 72;
        Opt.axes.x.ticker = (min, max) => { return Graph.Tickers.WaveTimeTicks(max, this.analyzer); };
        Opt.axes.x.valueFormatter = (x) => { return this.Round((x * 1000) / this.analyzer.SampleRate, 2, false) + ' ms'; };
        Opt.axes.y.valueFormatter = (x) => { return Graph.Formatters.Y_UnitScale(x, 'W', 2, true);  };
        Opt.axes.y.axisLabelFormatter = (x, gran, opts, g) => { return Graph.Formatters.Y_UnitScaleFull(x, gran, opts, g, '', 2, true); };
        Opt.axes.y2.valueFormatter = (x, opts, seriesName) => { return this.Round((seriesName == 'Voltage') ? x : (x / this.CurrentMult), 2, false) + ((seriesName == 'Voltage') ? ' V' : ' A'); };
        Opt.axes.y2.axisLabelFormatter = (x) => { return '<div style="display: inline-block; text-align: right; width: 36px; color: ' + this.analyzer.SerieColor('Voltage') + ' ">' + this.Round(x, 2, false) + '</div><div style="display: inline-block; text-align: right; width: 36px; color: ' + this.analyzer.SerieColor('Current') + ' ">' + this.Round(x / this.CurrentMult, 2, false) + '</div>'; };
        if((this.Phase != null) && (this.data != null)) {
          for(var i = 0; i < this.data.Voltage_Samples[0].length; i++) {
            var Row = [i, this.data.Voltage_Samples[this.Phase][i], this.data.Current_Samples[this.Phase][i]];
            Row.push(Row[1] * Row[2]);
            if(this.analyzer.VoltageChannels > 1) {
              var Sum = 0;
              for(var j = 0; j < this.analyzer.VoltageChannels; j++) Sum = Sum + this.data.Voltage_Samples[j][i] * this.data.Current_Samples[j][i];
              Row.push(Sum);
            }
            Opt.file.push(Row);
          }
          var VoltageRange = this.MatrixColsRange(Opt.file, [false, true], { wave: 1.0 });
          var CurrentRange = this.MatrixColsRange(Opt.file, [false, false, true], { wave: 1.0 });
          var MaxVoltage = Math.max(Math.abs(VoltageRange[0]), Math.abs(VoltageRange[1]));
          var MaxCurrent = Math.max(Math.abs(CurrentRange[0]), Math.abs(CurrentRange[1]));
          this.CurrentMult = MaxVoltage / MaxCurrent;
          for(var i = 0; i < Opt.file.length; i++) Opt.file[i][2] = Opt.file[i][2] * this.CurrentMult;
          var PowerRange = this.MatrixColsRange(Opt.file, [false, false, false, true, true], { wave: 1.0 });
          var MaxY1      = Math.max(Math.abs(PowerRange[0]), Math.abs(PowerRange[1]));
          Opt.axes.y.valueRange = [-MaxY1, MaxY1];
          Opt.axes.y2.valueRange = [-MaxVoltage, MaxVoltage];
        }
        return Object.seal(Opt);
      }
    },
    methods: {
      Export: function() {
        if((this.data == null) || (this.analyzer == null)) return [];
        var Header = ['Sample']
          .concat(this.analyzer.SerieNames('Voltage'))
          .concat(this.analyzer.SerieNames('Current'))
          .concat(this.analyzer.SerieNames('Power'));
        var Data = [Header];
        for(var i = 0; i < this.data.Voltage_Samples[0].length; i++) {
          var Row = [i];
          for(var n = 0; n < this.data.Voltage_Samples.length; n++) Row.push(this.Round(this.data.Voltage_Samples[n][i], 3));
          for(var n = 0; n < this.data.Voltage_Samples.length; n++) Row.push(this.Round(this.data.Current_Samples[n][i], 3));
          for(var n = 0; n < this.data.Voltage_Samples.length; n++) Row.push(this.Round(this.data.Voltage_Samples[n][i] * this.data.Current_Samples[n][i], 3));
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