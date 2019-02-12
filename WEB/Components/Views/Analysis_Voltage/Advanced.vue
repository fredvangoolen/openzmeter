<template>
  <div id="FFTPanel">
    <div id="FFTMaximize">
      <div id="FFTChart" class="chart big_chart"></div>
    </div>
  </div>
</template>

<script>
    FFTDataset = [];

 
  for(var i = 0; i < result.Voltage_FFT[0].length; i++) {
    var Row = [];
    for(n = 0; n < result.Voltage_FFT.length; n++) Row.push(result.Voltage_FFT[n][i]);
    for(n = 0; n < result.Current_FFT.length; n++) Row.push(result.Current_FFT[n][i]);
    FFTDataset.push(Row);
  }  

  UpdateFFTChart();
  
  
  /* Grafica de FFT */
var FFTChart            = null;
var FFTVOffset          = 0;
var FFTCOffset          = 0;
function UpdateFFTChart() {
  var ToShow      = [];
  var ShowCurrent = (localStorage.getItem("VoltageDetailCurrent") === 'true');
  var Log         = (localStorage.getItem('VoltageDetailLog') === 'true');
  for(i = 0; i < FFTDataset.length; i++) {
    var Row = [i];
    for(n = 0; n < FFTDataset[i].length; n++) Row.push(Log ? 20.0 * Math.log10(Math.max(0.001, FFTDataset[i][n])) : FFTDataset[i][n]);
    ToShow.push(Row);
  }
  var VoltageVisibility = [], CurrentVisibility = [], Y1Range = [], Y2Range = [], Series = {};
  GroupButtons($('#groupDetailPhasesMulti'), 'VoltageDetailVisiblePhasesMulti', Math.max(Analyzer.GetSeriesNames('Current').length, Analyzer.GetSeriesNames('Voltage').length), false, 
    function(VisibleSeries, Visibles) {
      VoltageVisibility = VisibleSeries.slice(0, Analyzer.GetSeriesNames('Voltage').length);
      CurrentVisibility = VisibleSeries.slice(0, Analyzer.GetSeriesNames('Current').length);
      if(ShowCurrent === false) CurrentVisibility.fill(false);
      Y1Range = CalcRanges(ToShow, Visibles, !Log);
      Y2Range = CalcRanges(ToShow, Visibles.map(function(val) { return val + Analyzer.GetSeriesNames('Voltage').length }), !Log);
      FFTVOffset = (Y1Range[0] < 0.0) ? -Y1Range[0] : 0.0;
      FFTCOffset = (Y2Range[0] < 0.0) ? -Y2Range[0] : 0.0;
      for(i = 0; i < ToShow.length; i++) {
        var Row = [i];
        for(n = 0; n < Analyzer.GetSeriesNames('Voltage').length; n++) Row.push(ToShow[i][n + 1] + FFTVOffset);
        for(n = 0; n < Analyzer.GetSeriesNames('Current').length; n++) Row.push(ToShow[i][n + 1 + Analyzer.GetSeriesNames('Voltage').length] + FFTCOffset);
        ToShow[i] = Row;
      }
      Analyzer.GetSeriesNames('Current').map(function(val) {Series[val] = { axis: 'y2' }; });
      var Options = { 
        strokeWidth: 0.5, gridLineColor: "#BBB", gridLinePattern: [4, 4], fillGraph: true, interactionModel: { mousedown: mouseDownOZM, mousemove: mouseMoveOZM, mouseup  : mouseUpOZM, dblclick : dblClickOZM_Restore },
        legend: 'follow', legendFormatter: LegendFormatter_Follow, labels: ['Bin'].concat(Analyzer.GetSeriesNames('Voltage')).concat(Analyzer.GetSeriesNames('Current')), series: Series, plugins: [ new Dygraph.Plugins.Crosshair( { direction: "vertical" } ) ],
        colors: ColorArray($('#VoltagePanel').css('border-color'), Analyzer.GetSeriesNames('Voltage').length).concat(ColorArray('#428bca', Analyzer.GetSeriesNames('Current').length)), visibility: VoltageVisibility.concat(CurrentVisibility),
        axes: { 
          y: { pixelsPerLabel: 15, axisLabelWidth: 34, valueRange: [Y1Range[0] + FFTVOffset, Y1Range[1] + FFTVOffset], 
               ticker: Log ? function(min, max) { return GraphDBTicks(min, max, FFTVOffset); } : Dygraph.numericTicks,
               valueFormatter: function(x) { return Round2(x - HarmonicsVOffset) + '<em class="x-small">' + (Log ? 'dB' : 'V') + '</em>'; },
               axisLabelFormatter: function (x) { return Round2(x); }
             },
          y2: { pixelsPerLabel: 15, axisLabelWidth: ShowCurrent ? 34 : 0, valueRange: [Y2Range[0] + FFTCOffset, Y2Range[1] + FFTCOffset],
                valueFormatter: function(x) { return Round2(x - FFTCOffset) + ' <em class="x-small">' + (Log ? 'dB' : 'A') + '</em>'; },
                ticker: (Log || ShowCurrent === false) ? function(min, max) { return (ShowCurrent === true) ? GraphDBTicks(min, max, FFTCOffset) : []; } : Dygraph.numericTicks,
                axisLabelFormatter: function (x) { return Round2(x); }
              },
          x: { drawGrid: true,  
               valueFormatter: function (y) { return Math.round(y / 10 * Analyzers[Analyzer].NominalFreq) + '<em class="small"> Hz</em>'; },
               axisLabelFormatter: function (y) {
                 return;
             y = (y / 10 * Nodes['NominalFreq']);
             if(y >= 1000) {
               y = y / 1000;
               if((y % 1) != 0) return "";
               return y + '<em class="small"> kHz</em>';
             } else {
               if((y % 1) != 0) return "";
               return y + '<em class="small"> Hz</em>';
             }
           }
         }
    }
  };
    if(FFTChart === null) {
      FFTChart = new Dygraph(document.getElementById("FFTChart"), ToShow, Options);
    } else {
      FFTChart.updateOptions(Options, true);
      FFTChart.updateOptions({ file: ToShow });
    }
  }, undefined);
}
  
</script>