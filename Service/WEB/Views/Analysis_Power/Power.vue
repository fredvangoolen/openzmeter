<template>
  <div>
    <graph :options="GraphOptions" :height="height"></graph>
  </div>
</template>

<script>
  import Graph from 'Components/Graph.vue';
  export default {
    props: { data: Object, serie: String, height: Number, analyzer: Object },
    data () {
      return { };
    },
    computed: {
      GraphOptions: function() {
        var Opt = { file: [], axes: { y: {}, x: {} } };
        Opt.showRangeSelector = false;
        Opt.panEdgeFraction = 1;
        Opt.strokeWidth = 0.5;
        Opt.stepPlot = true;
        Opt.drawGapEdgePoints = true;
        Opt.connectSeparatedPoints = false;
        Opt.fillGraph = true;
        Opt.gridLineColor = "#BBB";
        Opt.gridLinePattern = [4, 4];
        Opt.legend = 'follow';
        Opt.legendFormatter = Graph.Formatters.Legend_Follow;
        Opt.interactionModel = Graph.InteractionModels.Scrollable(this);
        Opt.underlayCallback = Graph.Callbacks.Underlay_DrawBorder;
//        Opt.underlayCallback = this.DrawPeriods;
        Opt.labels = (this.serie == 'Active_Power') ? ['Time', 'Active Power'] : ['Time', 'Reactive Power'];
        Opt.colors = [this.analyzer.Color[this.serie]];
        Opt.xAxisHeight = 24;
        Opt.selectMode = 'euclidian';
        Opt.axes.y.pixelsPerLabel = 15;
        Opt.axes.y.axisLabelWidth = 34;
        Opt.axes.x.axisLabelFormatter = (x) => { return this.analyzer.FormatDateTime(x, '<br/>'); };
        Opt.axes.x.valueFormatter = (x) => { return this.analyzer.FormatDateTime(x); };
        Opt.axes.y.valueFormatter = (x) => { return Graph.Formatters.Y_UnitScale(x, (this.serie == 'Active_Power') ? ' W' : 'var', 2, true);  };
        Opt.axes.y.axisLabelFormatter = (x, gran, opts, g) => { return Graph.Formatters.Y_UnitScaleFull(x, gran, opts, g, '', 2, true); };
        if(this.data != null) {
          for(var n in this.data.Sampletime) Opt.file.push([this.data.Sampletime[n], this.data[(this.serie == 'Active_Power') ? 'Active_Power' : 'Reactive_Power'][n]]);
          Opt.axes.y.valueRange = this.MatrixColsRange(Opt.file, [false, true], { incZero: 10 });
          Opt.dateWindow = [this.data.From, this.data.To];
        }
        return Object.seal(Opt);
      }
    },
    created: function() {
      this.Periods = [];
      this.Bills = [];
      this.ActiveData = [];
      this.ReactiveData = [];
    },
    methods: {
      DrawPeriods: function(canvas, area, g) {
        canvas.strokeStyle = '#777';
        canvas.lineWidth = 2;
        canvas.beginPath();
        canvas.moveTo(area.x, 0);
        canvas.lineTo(area.x, 13);
        canvas.stroke();
        for(var i = 0; i < this.Periods.length; i++) {
          var Left = g.toDomXCoord(this.Periods[i].From);
          var Right = g.toDomXCoord(this.Periods[i].To);
          if(Left < (area.x + 1)) Left = (area.x + 1);
          canvas.fillStyle = (this.Periods[i].Color === null) ? "#EEEEEE" : this.Periods[i].Color;
          canvas.fillRect(Left, area.y, Right, 13);
          if(((Right - Left) > 20) && (this.Periods[i].Name !== null)) {
            canvas.fillStyle = '#000000';
            canvas.font = "10px monospace";
            canvas.fillText(this.Periods[i].Name, Left + 1, 10);
          }
          var Y = g.toDomYCoord(this.Periods[i].Power);
          canvas.strokeStyle = '#d9534f';
          canvas.lineWidth = 2;
          canvas.beginPath();
          if(i > 0) {
            canvas.moveTo(Left, g.toDomYCoord(this.Periods[i -1 ].Power));
            canvas.lineTo(Left, Y);
          } else {
            canvas.moveTo(Left, Y);
          }
          canvas.lineTo(Right, Y);
          canvas.stroke();
        }
        for(var i = 0; i < this.Bills.length; i++) {
          canvas.strokeStyle = '#d9534f';
          canvas.lineWidth = 1;
          canvas.beginPath();
          var X = g.toDomXCoord(this.Bills[i].Sampletime);
          canvas.moveTo(X, 13);
          canvas.lineTo(X, area.h);
          canvas.stroke();
        }
        canvas.rect(area.x, area.y + 13, area.w, area.h);
        canvas.clip();
      },
      ExportClickReactive: function() {
        if(this.$root.SelectedAnalyzer === null) return;
        var Analyzer = this.$root.SelectedAnalyzer;
        this.$refs.tools.SaveCVS('Reactive_Power.csv', [['Time'].concat(oZm.Series['REACTIVE_POWER'].Names(Analyzer))].concat(this.ReactiveData));
      },

      UpdateGraph: function(to, period, lastsample, aggreg) {
        if(lastsample === 0) {
          this.ActiveData = [];
          this.ReactiveData = [];
          this.ShowSpinner = true;
        }
        var Analyzer = this.$root.SelectedAnalyzer;
        $.ajax({ url: 'getPrices', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json', data: JSON.stringify({ Aggreg: aggreg, To: to, LastSample: lastsample, Analyzer: Analyzer.Analyzer, Period: period }),
          success: function(result) {
            if((result['Sampletime'] === null) || (result['Sampletime'].length === 0)) return;
            this.ActiveData = oZm.GraphDataAppend(result, 'Active_Power', this.ActiveData);
            this.ReactiveData = oZm.GraphDataAppend(result, 'Reactive_Power', this.ReactiveData);
            lastsample = oZm.LastSampletime(result, lastsample);
            this.Bills = result["Bills"];
            //Group periods
            var LastItem = (this.Periods.length > 0) ? this.Periods.slice(-1)[0] : null;
            for(var i = 0; i < result.Sampletime.length; i++) {
              var Period = { Name: result.Period_Name[i], From: result.Sampletime[i] - ((this.Aggreg === '15M') ? 899999 : 3599999), To: result.Sampletime[i], Color: result.Period_Color[i] };
              if(LastItem === null) {
                this.Periods.push(Period);
                LastItem = Period;
              } else if(Period.From === (LastItem.To + 1)) {
                if(Period.Name === LastItem.Name) {
                  LastItem.To = Period.To;
                  this.Periods.pop();
                  this.Periods.push(LastItem);
                } else {
                  this.Periods.push(Period);
                  LastItem = Period;
                }
              }
              this.Periods = this.Periods.filter(period => (period.To > result.From) || (period.From < result.To) ).sort(function(a, b) { if(a.To < b.To) return -1; if(a.To > b.To) return 1; return 0; });
            }
            this.$refs.graphActive.SetOptions( { dateWindow: [result.From, result.To], file: this.ActiveData, valueRange: oZm.Series['ACTIVE_POWER'].Range(this.ActiveData, [true]) } );
            this.$refs.graphReactive.SetOptions( { dateWindow: [result.From, result.To], file: this.ReactiveData, valueRange: oZm.Series['REACTIVE_POWER'].Range(this.ReactiveData, [true]) } );
            this.$refs.activeCounters.SetStats([result.From, result.To], this.ActiveData, result["Aggregation_Source"]);
            this.$refs.reactiveCounters.SetStats([result.From, result.To], this.ReactiveData, result["Aggregation_Source"]);
          }.bind(this),
          complete: function() {
            this.$nextTick(function() { window.dispatchEvent(new Event('resize')); });
            this.ShowSpinner = false;
            this.$refs.period.UpdateCompleted(lastsample);
          }.bind(this),
          error: function(response) {
            if(response.status === 400) this.$refs.aggregation.AggregationFail();
            this.ShowSpinner = true;
          }.bind(this)
        });
      }
    }
  }
</script>