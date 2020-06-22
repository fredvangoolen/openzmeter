<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">{{ (data==null) ? 'Loading data...' : 'Last update at ' + analyzer.FormatTimeMS(data.Sampletime) }} {{ paused ? '(Paused)' : MouseOver ? '(Graph paused)' : '' }}</div>
      <div v-if="RmsCurrent<analyzer.MinCurrent" class="title-warning"><img :src="require('Imgs/exclamation.png')">No current or current too low</div>
      <portal to="fs-toolbar" :disabled="!Expanded" class="header-buttons">
        <toolbar-phases storage="DetailsPhases" :analyzer="analyzer" :multi="true" :current="true" v-model="Phases"></toolbar-phases>
        <toolbar-options storage="DetailsOptions" :options="['Current']" v-model="Options"></toolbar-options>
        <div class="btn-group">
          <button type="button" class="btn btn-default btn-xs" @click="ShowStream=true" data-toggle="tooltip" title="Stream waves in realtime"><img :src="require('Imgs/diskette.png')"></button>
          <button-export file="Waveform" :rows="Export"></button-export>
          <button-expand v-model="Expanded"></button-expand>
          <button-pause :value="paused" @input="$emit('update:paused', $event)"></button-pause>
        </div>
      </portal>
    </div>
    <portal to="fs-body" :disabled="!Expanded" class="panel-body">
      <spinner v-if="data==null"></spinner>
      <div @mouseenter="MouseOver=true" @mouseleave="MouseOver=false">
        <graph :options="GraphOptions" :height="Expanded?DialogHeight:300" :paused="MouseOver" :analyzer="analyzer"></graph>
      </div>
    </portal>
    <dlg v-if="ShowStream">
      <template v-slot:header>
        <div class="pull-left"><h4 class="modal-title">Waveform stream</h4></div>
        <div class="pull-right btn-group">
          <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Close dialog" v-if="Streaming==false" @click="ShowStream=false"><img :src="require('Imgs/arrow_undo.png')"></button>
        </div>
      </template>
      <div class="row">
        <div class="col-xs-12">
          <button type="button" class="btn btn-default btn-xs" v-if="Streaming==false" data-toggle="tooltip" title="Close dialog" @click="StartStream">Start</button>
          <button type="button" class="btn btn-default btn-xs" v-if="Streaming==true" data-toggle="tooltip" title="Close dialog" @click="StopStream=false">Stop</button>
        </div>
      </div>
    </dlg>
    <dlg-fs v-if="Expanded" title="Waveform" :height.sync="DialogHeight"></dlg-fs>
  </div>
</template>

<script>
  import * as oZm from 'JS/openZmeter.js';
  import Dygraph from 'JS/dygraph.js';
  import Graph from 'Components/Graph.vue';
  export default {
    props: { analyzer: Object, data: Object, paused: Boolean },
    data() {
      return { MouseOver: false, Options: null, Phases: null, Expanded: false, DialogHeight: 300, ShowStream: false, Streaming: false };
    },
    computed: {
      RmsCurrent: function() { return (this.data == null) ? 0.0 : oZm.ArrayAbsMax(this.data.Current); },
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
        Opt.axes.x.ticker = (min, max) => { return Graph.Tickers.WaveTimeTicks(max, this.analyzer); };
        if((this.Phases == null) || (this.Options == null)) return Opt;
        Opt.visibility = [];
        var VoltageStyle = oZm.SerieStyle(oZm.Series['VOLTAGE'], this.analyzer, ((this.Options['Current'] == true) ? 1 : 0));
        var CurrentStyle = oZm.SerieStyle(oZm.Series['CURRENT'], this.analyzer, 0);
        var VoltageNames = oZm.Series['VOLTAGE'].Names(this.analyzer);
        var CurrentNames = oZm.Series['CURRENT'].Names(this.analyzer);
        for(var n in VoltageNames) Opt.visibility.push(this.Phases[n]);
        for(var n in CurrentNames) Opt.visibility.push(this.Phases[n] && this.Options['Current']);
        Opt.series = {};
        for(var n in CurrentNames) Opt.series[CurrentNames[n]] = { axis: 'y2' };
        Opt.axes = { y: {}, y2: {}, x: {} };
        Opt.axes.y.pixelsPerLabel = 15;
        Opt.axes.y.axisLabelWidth = 34;
        Opt.axes.y.valueFormatter = (x) => { return this.Round(x, 2, true) + ' V'; };
        Opt.axes.y.axisLabelFormatter = (x) => { return this.Round(x, 2, true); };
        Opt.axes.y.strokeWidth = VoltageStyle.Width;
        Opt.axes.y2.pixelsPerLabel = 15;
        Opt.axes.y2.drawAxis = this.Options['Current'];
        Opt.axes.y2.axisLabelWidth = this.Options['Current'] ? 34 : 0;
        Opt.axes.y2.strokeWidth = CurrentStyle.Width;
        Opt.axes.y2.valueFormatter = (x) => { return this.Round(x, 2, true) + ' A'; };
        Opt.axes.y2.axisLabelFormatter = this.Options['Current'] ? (x) => { return this.Round(x, 2, true); } : (x)  => { return ''};
        Opt.axes.x.valueFormatter = (x) => { return this.Round((x * 1000) / this.analyzer.SampleRate, 2, true) + ' ms'; };
        Opt.labels = ['Sample'].concat(VoltageNames).concat(CurrentNames);
        Opt.colors = VoltageStyle.Colors.concat(CurrentStyle.Colors);
        if(this.data != null) {
          for(var i = 0; i < this.data.Voltage_Samples[0].length; i++) {
            var Row = [i];
            for(var n = 0; n < this.data.Voltage_Samples.length; n++) Row.push(this.data.Voltage_Samples[n][i]);
            for(var n = 0; n < this.data.Current_Samples.length; n++) Row.push(this.data.Current_Samples[n][i]);
            Opt.file.push(Row);
          }
          var V = [false], I = [false];
          for(var n in VoltageNames) { V.push(this.Phases[n]); I.push(false); }
          for(var n in CurrentNames) { I.push(this.Phases[n]); V.push(false); }
          Opt.axes.y.valueRange = this.MatrixColsRange(Opt.file, V, { wave: 1.0 });
          Opt.axes.y2.valueRange = this.MatrixColsRange(Opt.file, I, { wave: 1.0 });
          Opt.dateWindow = [0, Opt.file.length];
        }
        return Object.seal(Opt);
      }
    },
    methods: {
      StartStream: function() {
        //this.$http.post(, { Analyzer: this.analyzer.Analyzer }, )
        var Req = new XMLHttpRequest();
        Req.open('POST', 'getWaveStream', true);
        Req.setRequestHeader('Content-type', 'application/json');
        Req.overrideMimeType('text/plain');
        var receivedIndex = 0;
        Req.onprogress = (pe) => {
//            console.log("PROGRESS: response: " + Req.response);
//            console.log("PROGRESS: responseType: " + Req.responseType);
            console.log("PROGRESS: allResponseHeaders: " + Req.getAllResponseHeaders());

            if (pe.lengthComputable) {
                console.log("PROGRESS: loaded: " + pe.loaded );
                console.log("PROGRESS: total: " + pe.total );

            }

            if((Req.readyState == 3 || Req.readyState == 2) && Req.status == 200) {
                //slice the response(Text) to show chunk by chunk
                var chunk = Req.response.slice(receivedIndex);
                if(chunk) { //not empty
                    //$("#myList").append("<li>"+ chunk +"</li>");
 console.log("Nuevo bloque");
                }
                receivedIndex = Req.response.length; //if Blob or other use size
            } else {
                console.error("PROGRESS: Chrome! don't call onprogress for readyState ==  " +
                Req.readyState + " and status == "+ Req.status);
            }
        };
        Req.onerror = function () {
            console.log("An error occurred...");
            $("#myDiv").html("An error occurred...");
        };
        Req.ontimeout = function () {
            console.log("The request timed out...");
            $("#myDiv").html("The request timed out...");
        };
        Req.onabort = function () {
            console.log("The request aborted by the user....");
            $("#myDiv").html("The request aborted by the user...");
        };

        Req.responseType = "text";
        Req.onreadystatechange = function () {
        if(Req.readyState === 4 && Req.status === 200) {
          var filename = "WaveStream.csv";
          if(typeof window.chrome !== 'undefined') {
                // Chrome version
                var link = document.createElement('a');
                link.href = window.URL.createObjectURL(Req.response);
//                link.download = filename;
//                link.click();
            } else if (typeof window.navigator.msSaveBlob !== 'undefined') {
                // IE version
                var blob = new Blob([Req.response], { type: 'application/force-download' });
                window.navigator.msSaveBlob(blob, filename);
            } else {
                // Firefox version
                var file = new File([Req.response], filename, { type: 'application/force-download' });
                window.open(URL.createObjectURL(file));
            }
        }
    };
    Req.timeout = 5000000000000;
        Req.send(JSON.stringify({ Analyzer: this.analyzer.Analyzer, Series: ['VOLTAGE'] }));
      },
      StopStream: function() {

      },
      Export: function() {
        var Rows = [['Sample'].concat(oZm.Series['VOLTAGE'].Names(this.analyzer).concat(oZm.Series['CURRENT'].Names(this.analyzer)))];
        if(this.data != null) {
          for(var i = 0; i < this.data.Voltage_Samples[0].length; i++) {
            var Row = [i];
            for(var n = 0; n < this.data.Voltage_Samples.length; n++) Row.push(this.Round(this.data.Voltage_Samples[n][i], 3));
            for(var n = 0; n < this.data.Current_Samples.length; n++) Row.push(this.Round(this.data.Current_Samples[n][i], 3));
            Rows.push(Row);
          }
        }
        return Object.seal(Rows);
      }
    }
  }
</script>

<style scoped>
  .header-text {
    font-size: 14px;
  }
</style>