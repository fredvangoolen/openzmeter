<template>
              <div id="ParametersPanel">
            </div>
</template>


/* Evolucion de parametros */
var HarmonicsCharts = [], HarmonicsData = [], PowerFactorChart = null, PowerFactorData = [], THDChart = null, THDData = [], PHIChart = null, PHIData = [];
function PeriodView_OnSuccess(result) {
  while(HarmonicsData.length !== 50) HarmonicsData.push([]);
  for(k = 0; k < 50; k++) {
    for(i = 0; i < result.Voltage_Harmonics.length; i++) {
      var Row = [new Date(result.Sampletime[i])];
      for(j = 0; j < result.Voltage_Harmonics[i].length; j++) Row.push(20 * Math.log10(Math.max(0.000000001, result.Voltage_Harmonics[i][j][k])));
      HarmonicsData[k].push(Row);
    }
    ShiftArray(HarmonicsData[k], result.MaxSamples);
  }
  PowerFactorData = GraphDataAppend(result, 'Power_Factor', PowerFactorData);
  THDData = GraphDataAppend(result, 'Voltage_THD', THDData);
  PHIData = GraphDataAppend(result, 'Phi', PHIData);
  if(HarmonicsCharts.length === 0) {
    for(i = 0; i < 50; i++) {
      HarmonicsCharts[i] = AppendCubismGraph($('#EvolutionPanel'), HarmonicsData[i], Analyzer.GetSeriesNames('Harmonics')[i] + ' harmonic', [result['From'], result['To']], function (x) { return (x === null) ? '~' : Round2(x) + '<em>dB</em>' });
    }
    Dygraph.synchronize(HarmonicsCharts, { range: false, zoom: false });
    PowerFactorChart = AppendCubismGraph($('#ParametersPanel'), PowerFactorData, "Power Factor", [result['From'], result['To']], function (x) { return (x === null) ? '~' : Round2(x); });
    THDChart         = AppendCubismGraph($('#ParametersPanel'), THDData, "Voltage THD", [result['From'], result['To']], function (x) { return (x === null) ? '~' : (Round2(x * 100.0) + '<em>%</em>'); });
    PHIChart         = AppendCubismGraph($('#ParametersPanel'), PHIData, "Angle", [result['From'], result['To']], function (x) { return (x === null) ? '~' : (Round2(Math.abs(x)) + '<em>' + ((x < 0) ? 'º CAP' : (x > 0) ? 'º IND' : 'º') + '</em>'); });
    Dygraph.synchronize([PowerFactorChart, THDChart, PHIChart], { range: false, zoom: false });
  } else {
    for(var i = 0; i < 50; i++) HarmonicsCharts[i].updateOptions({ file: HarmonicsData[i], dateWindow: [result['From'], result['To']] });
    PowerFactorChart.updateOptions({ file: PowerFactorData, dateWindow: [result['From'], result['To']] });
    THDChart.updateOptions({ file: THDData, dateWindow: [result['From'], result['To']] });
    PHIChart.updateOptions({ file: PHIData, dateWindow: [result['From'], result['To']] });
  }
  ParametersSeries();
}
function ParametersSeries(event) {
  GraphPhaseButtons(PowerFactorChart, $('#groupDetailPhasesSingle'), 'VoltageDetailVisiblePhasesSingle', true, function(series) { var Range = ArrayAbsMax(CalcRanges(PowerFactorData, series, false)); return [-Range, Range]; }, (event !== undefined) ? event.target : undefined); 
  GraphPhaseButtons(PHIChart,         $('#groupDetailPhasesSingle'), 'VoltageDetailVisiblePhasesSingle', true, function(series) { var Range = ArrayAbsMax(CalcRanges(PHIData, series, false, false, false, true)); return [-Range, Range]; }, undefined); 
  GraphPhaseButtons(THDChart,         $('#groupDetailPhasesSingle'), 'VoltageDetailVisiblePhasesSingle', true, function(series) { var Range = ArrayAbsMax(CalcRanges(THDData, series, false, false, false, true)); return [-Range, Range]; }, undefined); 
  for(var g = 0; g < 50; g++) GraphPhaseButtons(HarmonicsCharts[g], $('#groupDetailPhasesSingle'), 'VoltageDetailVisiblePhasesSingle', true, function(series) { return CalcRanges(HarmonicsData[g], series, false); }, undefined); 
}
