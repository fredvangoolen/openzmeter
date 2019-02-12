<template>
  <div>
    <ul class="nav nav-tabs" id="DetailTabs">
      <li><a data-toggle="tab" href="#">Waveform</a></li>
      <li><a data-toggle="tab" href="#">Harmonics</a></li>
      <li><a data-toggle="tab" href="#">Evolution</a></li>
      <li><a data-toggle="tab" href="#">Advanced</a></li>
      <li><a data-toggle="tab" href="#">Phasors</a></li>
      <li><a data-toggle="tab" href="#">Parameters</a></li>
      <li><a data-toggle="tab" href="#">Events</a></li>
    </ul>
    <div class="panel panel-other voltage-details">
          <!--div id="DetailSpinner" class="spinnerdiv"></div-->
      <div class="panel-heading clearfix">
        <div class="header-text">{{ Time }}</div>
        <div class="header-buttons">
          <toolbar-phases></toolbar-phases>
          <toolbar-phases></toolbar-phases>
          <div class="btn-group">
            <button type="button" class="btn btn-default btn-xs" id="btnShowDetail1Harmonic" data-toggle="tooltip" title="Show/hide 1st harmonic">1st harmonic</button>
            <button type="button" class="btn btn-default btn-xs" id="btnShowDetailCurrent" data-toggle="tooltip" title="Show/hide current">Current</button>
            <button type="button" class="btn btn-default btn-xs" id="btnShowDetailLog" data-toggle="tooltip" title="Use logarithm scale">Log</button>
          </div>
          <toolbar-tools></toolbar-tools>
        </div>
      </div>
      <div class="panel-body" id="DetailDivs">
        <waveform ref="waveform"></waveform>
        <harmonics ref="harmonics"></harmonics>
        <evolution></evolution>
        <!--advanced></advanced>
        <phasors></phasors>            
        <parameters></parameters>
        <events></events-->
      </div>
    </div>
  </div>
</template>

<script>
  import moment from 'moment';
  import ToolbarPhases from 'Components/ToolbarPhases.vue';
  import ToolbarTools from 'Components/ToolbarTools.vue';
//  import Advanced from './Advanced.vue';
//  import Events from './Events.vue';
  import Evolution from './Evolution.vue';
  import Harmonics from './Harmonics.vue';
//  import Parameters from './Parameters.vue';
//  import Phasors from './Phasors.vue';
  import Waveform from './Waveform.vue';
  export default {
    data () {
      return { Time: 'Loading data...', LastSample: 0, UpdateGraphs1MTimer: null, UpdateGraphsNowTimer: null };
    },
    created: function() {
      this.$root.$on('AnalyzerChanged', this.SetAnalyzer);
    },
    mounted: function() {
      if(this.$root.SelectedAnalyzer !== null) {
        this.UpdateGraphs1M();
        this.UpdateGraphsNow();
      }
    },
    beforeDestroy: function() {
      if(this.UpdateGraphs1MTimer !== null) clearTimeout(this.UpdateGraphs1MTimer);
      if(this.UpdateGraphsNowTimer !== null) clearTimeout(this.UpdateGraphsNowTimer);
    },
    methods: {
      SetAnalyzer() {
        this.LastSample = 0;
        this.UpdateGraphs1M();
        this.UpdateGraphsNow();
      },
      UpdateGraphs1M() {
        if(this.$root.SelectedAnalyzer === null) return;
        $.ajax({ url: 'getSeriesNow', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Series: ['VOLTAGE_SAMPLES', 'CURRENT', 'ACTIVE_POWER', 'FREQUENCY'], Analyzer: this.$root.SelectedAnalyzer.ID }), 
          success: function(result) {
    //            this.$refs.indicator_bar.setValue(result);
          }.bind(this),
          complete: function() {
            if(this.UpdateGraphs1MTimer !== null) clearTimeout(this.UpdateGraphs1MTimer);
            this.UpdateGraphs1MTimer = setTimeout(this.UpdateGraphs1M, 30000);
          }.bind(this), 
          error: function() {
//            this.$refs.indicator_bar.setValue(null);
          }.bind(this)
        });
      },
      UpdateGraphsNow() {
        if(this.$root.SelectedAnalyzer === null) return;
        $.ajax({ url: 'getSeriesNow', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Series: ['VOLTAGE_SAMPLES', 'CURRENT_SAMPLES', 'VOLTAGE_HARMONICS', 'CURRENT_HARMONICS'], Analyzer: this.$root.SelectedAnalyzer.ID }), 
          success: function(result) {
            this.Time = 'Last update at ' + moment(result.Sampletime).format('HH:mm:ss.SSS');
            this.$refs.waveform.SetData(result);
            this.$refs.harmonics.SetData(result);
          }.bind(this),
          complete: function() {
            if(this.UpdateGraphsNowTimer !== null) clearTimeout(this.UpdateGraphsNowTimer);
            this.UpdateGraphsNowTimer = setTimeout(this.UpdateGraphsNow, 1000);
          }.bind(this), 
          error: function() {
//            this.$refs.indicator_bar.setValue(null);
          }.bind(this)
        });
      }
    },
    components: { Waveform, Harmonics, Evolution, ToolbarPhases, ToolbarTools }
  }
</script>

<style>
  .voltage-details.panel-other {
    border-top-left-radius: 0px;
    border-top-right-radius: 0px;
  }
</style>


      
      
//var WaveformDataset = [], HarmonicsDataset = [], FFTDataset = [];
function Live_OnSuccess(result) {
  


  UpdatePhasorChart(result);
  SetCubismNowLabel(PowerFactorChart, result.Power_Factor);
  SetCubismNowLabel(THDChart, result.Voltage_THD); 
  SetCubismNowLabel(PHIChart, result.Phi); 
}
function DetailTabChange(index) {
  $('#groupDetailPhasesSingle, #groupDetailPhasesMulti, #btnShowDetail1Harmonic, #btnShowDetailLog, #btnShowDetailCurrent, #toolbarDetails button[data-function="Maximize"]').hide();
  if((index === 0) || (index === 1) || (index === 3)) $('#groupDetailPhasesMulti').show();
  if((index === 0) || (index === 1) || (index === 3)) $('#btnShowDetailCurrent').show();
  if((index === 0) || (index === 1) || (index === 3)) $('#toolbarDetails button[data-function="Maximize"]').show();
  if((index === 2) || (index === 5)) $('#groupDetailPhasesSingle').show();
  if((index === 1) || (index === 3)) $('#btnShowDetailLog').show();
  if(index === 1) $('#btnShowDetail1Harmonic').show();
  if(WaveformChart !== null) {
    if(index === 0) $(WaveformChart.graphDiv).parent().removeAttr('style');
    UpdateWaveformChart();
  }
  if(HarmonicsChart !== null) {
    if(index === 1) $(HarmonicsChart.graphDiv).parent().removeAttr('style');
    UpdateHarmonicsChart();
  }
  if(FFTChart !== null) {
    if(index === 3) $(FFTChart.graphDiv).parent().removeAttr('style');
    UpdateFFTChart();
  }
  if(PhasorChart !== null) {
    if(index === 4) PhasorResizeEvent();
  }
  if(HarmonicsCharts.length !== 0) {
    if(index === 2) {
      for(i = 0; i < 50; i++) $(HarmonicsCharts[i].graphDiv).parent().removeAttr('style');
    }
  }
  if(PowerFactorChart !== null) {
    if(index === 5) {
      $(PowerFactorChart.graphDiv).parent().removeAttr('style');
      $(THDChart.graphDiv).parent().removeAttr('style');
      $(PHIChart.graphDiv).parent().removeAttr('style');
    }
  }
  if(EventChart !== null) {
    if(index === 6) {
      $(EventChart.graphDiv).parent().removeAttr('style');
      EventsTable.columns.adjust();
    }
  }
}

function DetailsButtonUpdate(event) {
  GroupButtons($('#groupDetailPhasesMulti'), 'DetailPhases', Math.max(Analyzer.GetSeriesNames('Current').length, Analyzer.GetSeriesNames('Voltage').length), false, undefined, undefined);
  if(localStorage.getItem('VoltageDetailFirstHarmonic') === 'true') {
    $('#btnShowDetail1Harmonic').addClass('active');
  } else {
    $('#btnShowDetail1Harmonic').removeClass('active');
  }
  if(localStorage.getItem('VoltageDetailCurrent') === 'true') {
    $('#btnShowDetailCurrent').addClass('active');
  } else {
    $('#btnShowDetailCurrent').removeClass('active');
  }
  if(localStorage.getItem('VoltageDetailLog') === 'true') {
    $('#btnShowDetailLog').addClass('active');
  } else {
    $('#btnShowDetailLog').removeClass('active');
  }
}
$('#groupDetailPhasesMulti').on('click', 'button', function(evt) {
  GroupButtons($('#groupDetailPhasesMulti'), 'VoltageDetailVisiblePhasesMulti', Math.max(Analyzer.GetSeriesNames('Current').length, Analyzer.GetSeriesNames('Voltage').length), false,
    function() { UpdateWaveformChart(); UpdateHarmonicsChart(); }, evt.target);
});
$('#btnShowDetailCurrent').click(function() {
  localStorage.setItem('VoltageDetailCurrent', (localStorage.getItem('VoltageDetailCurrent') === 'true') ? 'false' : 'true');
  DetailsButtonUpdate();
  UpdateWaveformChart();
  UpdateHarmonicsChart();
  UpdateFFTChart();
  if(WaveformChart !== null) { WaveformChart.updateOptions({ showRangeSelector: true}); WaveformChart.updateOptions({ showRangeSelector: false}); }
  if(HarmonicsChart !== null) { HarmonicsChart.updateOptions({ showRangeSelector: true}); HarmonicsChart.updateOptions({ showRangeSelector: false}); }
  if(FFTChart !== null) { FFTChart.updateOptions({ showRangeSelector: true}); FFTChart.updateOptions({ showRangeSelector: false}); }
});
$('#btnShowDetail1Harmonic').click(function() {
  localStorage.setItem('VoltageDetailFirstHarmonic', (localStorage.getItem('VoltageDetailFirstHarmonic') === 'true') ? 'false' : 'true');
  DetailsButtonUpdate();
  UpdateWaveformChart();
  UpdateHarmonicsChart();
});
$('#btnShowDetailLog').click(function() {
  localStorage.setItem('VoltageDetailLog', (localStorage.getItem('VoltageDetailLog') === 'true') ? 'false' : 'true');
  DetailsButtonUpdate();
  UpdateWaveformChart();
  UpdateHarmonicsChart();
  UpdateFFTChart();
});
$('#groupDetailPhasesSingle').on('click', 'button', ParametersSeries);


function ExportDetail() {
  var Data = [];
  var Filename;
  if(DetailTabSelector.GetSaved() === 0) {
    exportToCsv('Waveform.csv', [['Time'].concat(Analyzer.GetSeriesNames('Voltage')).concat(Analyzer.GetSeriesNames('Current'))].concat(WaveformDataset));
  } else if(DetailTabSelector.GetSaved() === 3) {
    var Header = ['Index'];
    for(var i = 0; i < Analyzer.GetSeriesNames('Voltage').length; i++) { Header.push(Analyzer.GetSeriesNames('Voltage')[i]); Header.push(Analyzer.GetSeriesNames('Voltage')[i] + ' (dB)'); }
    for(var i = 0; i < Analyzer.GetSeriesNames('Current').length; i++) { Header.push(Analyzer.GetSeriesNames('Current')[i]); Header.push(Analyzer.GetSeriesNames('Current')[i] + ' (dB)'); }
    Data.push(Header);
    for(var i = 0; i < FFTDataset.length; i++) {
      var Row = [i];
      for(var n = 0; n < Analyzer.GetSeriesNames('Voltage').length + Analyzer.GetSeriesNames('Current').length; n++) { Row.push(FFTDataset[i][n]); Row.push(20.0 * Math.log10(Math.max(0.000000001, FFTDataset[i][n]))); }
      Data.push(Row);
    }
    exportToCsv('FFT.csv', Data);
  } else if(DetailTabSelector.GetSaved() === 1) {
    var Header = ['Harmonic'];
    for(var i = 0; i < Analyzer.GetSeriesNames('Voltage').length; i++) { Header.push(Analyzer.GetSeriesNames('Voltage')[i]); Header.push(Analyzer.GetSeriesNames('Voltage')[i] + ' (dB)'); }
    for(var i = 0; i < Analyzer.GetSeriesNames('Current').length; i++) { Header.push(Analyzer.GetSeriesNames('Current')[i]); Header.push(Analyzer.GetSeriesNames('Current')[i] + ' (dB)'); }
    Data.push(Header);
    for(i = 0; i < HarmonicsDataset.length; i++) {
      var Row = [SeriesNames.Harmonics[i]];
      for(var n = 0; n < Analyzer.GetSeriesNames('Voltage').length + Analyzer.GetSeriesNames('Current').length; n++) { Row.push(HarmonicsDataset[i][n]); Row.push(20.0 * Math.log10(Math.max(0.000000001, HarmonicsDataset[i][n]))); }
      Data.push(Row);
    }
    exportToCsv('Harmonics.csv', Data);
  } else if(DetailTabSelector.GetSaved() === 2) {
    var Header = ['Date/Time'];
    for(var i = 0; i < SeriesNames.Harmonics.length; i++) {
      for(var n = 0; n < Analyzer.GetSeriesNames('Voltage').length; n++) Header.push( SeriesNames.Harmonics[i] + '(' + Analyzer.GetSeriesNames('Voltage')[n] + ')');
    }
    Data.push(Header);
    for(var i = 0; i < HarmonicsData[0].length; i++) {
      var Row = [new Date(HarmonicsData[0][i][0]).toLocaleTimeString()];
      for(var j = 0; j < SeriesNames.Harmonics.length; j++) {
        for(var k = 0; k < Analyzer.GetSeriesNames('Voltage').length; k++) Row.push(HarmonicsData[j][i][k + 1]);
      }
      Data.push(Row);
    }
    exportToCsv('Voltage_Harmonics_Evolution.csv', Data);
  } else if(DetailTabSelector.GetSaved() === 5) {
    var Header = ['Date/Time'].concat(SeriesNames.PowerFactor()).concat(SeriesNames.THDv()).concat(SeriesNames.PHI());
    Data.push(Header);
    for(var i = 0; i < PowerFactorData.length; i++) {
      var Row = [new Date(PowerFactorData[i][0]).toLocaleTimeString()];
      for(var k = 0; k < SeriesNames.PowerFactor().length; k++) Row.push(PowerFactorData[i][k + 1]);
      for(var k = 0; k < SeriesNames.THDv().length; k++) Row.push(THDData[i][k + 1]);
      for(var k = 0; k < SeriesNames.PHI().length; k++) Row.push(PHIData[i][k + 1]);
      Data.push(Row);
    }
    exportToCsv('PF_THDv_PHI.csv', Data);
  }
}
var MainScrollable = null;
var DetailTabSelector = null;
function UpdatePage() {
  DetailTabSelector = new TabSelector('VoltageDetailView', '#DetailTabs', '#DetailDivs', DetailTabChange);
  DetailsButtonUpdate();
  MainScrollable = new Scrollable( {Spinners: ['#VoltageSpinner'], NowBtn: '#toolbarVoltage button[data-function="Update"]', Series: ['VOLTAGE'], OnSuccess: Scrollable_OnSuccess, Counters: Scrollable_UpdateCounters } );
  new Live( {NowBtn: '#toolbarDetails button[data-function="Update"]', RefreshInterval: 5000, Spinners: ['#DetailSpinner'], Series: ['VOLTAGE_SAMPLES', 'CURRENT_SAMPLES', 'VOLTAGE_HARMONICS', 'CURRENT_HARMONICS', 'VOLTAGE_FFT', 'CURRENT_FFT', 'POWER_FACTOR', 'VOLTAGE_THD', 'PHI', 'VOLTAGE', 'CURRENT'], OnSuccess: Live_OnSuccess});
  new PeriodView( {Spinners: [], Series: ['POWER_FACTOR', 'VOLTAGE_THD', 'PHI', 'VOLTAGE_HARMONICS'], Period: 86400000, Aggreg: '1M', OnSuccess: PeriodView_OnSuccess} );
  new GraphTools(function() { return ['Waveform', 'Harmonics', '', 'Advanced'][DetailTabSelector.GetSaved()]; }, function() { return [WaveformChart, HarmonicsChart, null, FFTChart][DetailTabSelector.GetSaved()]; }, '#toolbarDetails', undefined, ExportDetail);
}
      
