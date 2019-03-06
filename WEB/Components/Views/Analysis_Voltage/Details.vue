<template>
  <div>
    <detail-tabs :tabs="['Waveform', 'Harmonics', 'Evolution', 'Advanced', 'Phasors', 'Parameters', 'Events']" variable="VoltageDetailTab" v-on:TabChange="TabChange"></detail-tabs>
    <waveform ref="waveform" v-bind:class="{ hide: (ActiveTab != 'Waveform') }"></waveform>
    <harmonics ref="harmonics" v-bind:class="{ hide: (ActiveTab != 'Harmonics') }"></harmonics>
    <evolution ref="evolution" v-bind:class="{ hide: (ActiveTab != 'Evolution') }"></evolution>
    <advanced ref="advanced" v-bind:class="{ hide: (ActiveTab != 'Advanced') }"></advanced>
    <parameters ref="parameters" v-bind:class="{ hide: (ActiveTab != 'Parameters') }"></parameters>
      
      
    <!--div class="panel-heading clearfix">
        <div class="header-text">{{ Time }}</div>
        <div class="header-buttons" ref="Toolbar">
          <toolbar-phases variable="DetailsPhaseSingle" v-if="(ActiveTab == 'Evolution') || (ActiveTab == 'parameters')"></toolbar-phases>
          
          
          <toolbar-tools variable="To" v-if="(ActiveTab == 'Phasors')" ref="Tools" :options="['EXPORT', 'PLAY']"></toolbar-tools>
          <toolbar-tools variable="To" v-if="(ActiveTab == 'Evolution') || (ActiveTab == 'Parameters') || (ActiveTab == 'Events')" ref="Tools" :options="['EXPORT']"></toolbar-tools>
        </div>
      </div>
      <div class="panel-body">
        <spinner v-bind:class="{ 'hide': !ShowSpinner }"></spinner>
        
        
        
        <phasors ref="phasors" v-bind:class="{ hide: (ActiveTab != 'Phasors') }"></phasors>
        
        <events ref="events" v-bind:class="{ hide: (ActiveTab != 'Events') }"></events>
      </div-->
  </div>
</template>

<script>
  import moment from 'moment';
  import Spinner from 'Components/Spinner.vue';
  import ToolbarPhases from 'Components/ToolbarPhases.vue';
  import ToolbarOptions from 'Components/ToolbarOptions.vue';
  import ToolbarTools from 'Components/ToolbarTools.vue';
  import Advanced from './Advanced.vue';
  import Events from './Events.vue';
  import Evolution from './Evolution.vue';
  import Harmonics from './Harmonics.vue';
  import Parameters from './Parameters.vue';
  import Phasors from './Phasors.vue';
  import Waveform from './Waveform.vue';
  import DetailTabs from 'Components/Tabs.vue';
  export default {
    data () {
      return { ActiveTab: '', Time: 'Loading data...', LastSample: 0, UpdateGraphs1MTimer: null, UpdateGraphsNowTimer: null, ShowSpinner: true };
    },
    created: function() {
      this.$root.$on('AnalyzerChanged', this.SetAnalyzer);
    },
    mounted: function() {
      if(this.$root.SelectedAnalyzer !== null) {
        this.UpdateGraphs1M();
        this.UpdateGraphsNow();
      }
      this.TabChange();
    },
    beforeDestroy: function() {
      if(this.UpdateGraphs1MTimer !== null) clearTimeout(this.UpdateGraphs1MTimer);
      if(this.UpdateGraphsNowTimer !== null) clearTimeout(this.UpdateGraphsNowTimer);
    },
    methods: {
      Export() {
//        this.$refs.tools.SaveCVS(this.SerieProps.Title + '.csv', [['Time'].concat(this.SerieProps.Names(Analyzer))].concat(this.Data));
//        oZm.exportToCsv('Waveform.csv', [['Time'].concat(Analyzer.GetSeriesNames('Voltage')).concat(Analyzer.GetSeriesNames('Current'))].concat(WaveformDataset));
      },

      TabChange(tab, name) {
        if(name !== undefined) this.ActiveTab = name;
        if(this.ActiveTab === 'Waveform') this.$refs.waveform.Redraw();
        if(this.ActiveTab === 'Harmonics') this.$refs.harmonics.Redraw();
        if(this.ActiveTab === 'Evolution') this.$refs.evolution.Redraw();
        if(this.ActiveTab === 'Advanced') this.$refs.advanced.Redraw();
        if(this.ActiveTab === 'Parameters') this.$refs.parameters.Redraw();
        
//  if(PhasorChart !== null) {
//    if(index === 4) PhasorResizeEvent();
//  }
//  if(HarmonicsCharts.length !== 0) {
//    if(index === 2) {
//      for(i = 0; i < 50; i++) $(HarmonicsCharts[i].graphDiv).parent().removeAttr('style');
//    }
//  }

//  if(EventChart !== null) {
//    if(index === 6) {
//      $(EventChart.graphDiv).parent().removeAttr('style');
//      EventsTable.columns.adjust();
//    }
//  }

      },
      PhasesChangedMulti: function(phases) {
        this.$refs.waveform.SetPhases(phases);
        this.$refs.harmonics.SetPhases(phases);
        this.$refs.advanced.SetPhases(phases);
      },
      SetAnalyzer() {
        this.Time = 'Loading data...';
        this.LastSample = 0;
        this.UpdateGraphs1M();
        this.UpdateGraphsNow();
      },
      UpdateGraphs1M() {
        if(this.$root.SelectedAnalyzer === null) return;
        $.ajax({ url: 'getSeries', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Aggreg: '1M', Period: 86400000, To: 0, LastSample: this.LastSample, Series: ['VOLTAGE_HARMONICS', 'POWER_FACTOR', 'VOLTAGE_THD', 'PHI'], Analyzer: this.$root.SelectedAnalyzer.ID }),
          success: function(result) {
            this.$refs.evolution.SetData(result);
            this.$refs.parameters.SetData(result);
          }.bind(this),
          complete: function() {
            if(this.UpdateGraphs1MTimer !== null) clearTimeout(this.UpdateGraphs1MTimer);
            this.UpdateGraphs1MTimer = setTimeout(this.UpdateGraphs1M, 30000);
          }.bind(this), 
          error: function() {
  //deshabilitar¿?
    //            this.$refs.indicator_bar.setValue(null);
          }.bind(this)
        });
      },
      UpdateGraphsNow() {
        if(this.$root.SelectedAnalyzer === null) return;
        $.ajax({ url: 'getSeriesNow', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Series: ['VOLTAGE_SAMPLES', 'CURRENT_SAMPLES', 'VOLTAGE_HARMONICS', 'CURRENT_HARMONICS', 'VOLTAGE_FFT', 'CURRENT_FFT', 'POWER_FACTOR', 'VOLTAGE_THD', 'PHI'], Analyzer: this.$root.SelectedAnalyzer.ID }), 
          success: function(result) {
            this.ShowSpinner = false;
            this.Time = 'Last update at ' + moment(result.Sampletime).format('HH:mm:ss.SSS');
            this.$refs.waveform.SetData(result);
            this.$refs.harmonics.SetData(result);
            this.$refs.advanced.SetData(result);
            this.$refs.parameters.SetDataNow(result);
            
            
//              UpdatePhasorChart(result);
//  SetCubismNowLabel(PowerFactorChart, result.Power_Factor);
//  SetCubismNowLabel(THDChart, result.Voltage_THD); 
//  SetCubismNowLabel(PHIChart, result.Phi); 
          }.bind(this),
          complete: function() {
            if(this.UpdateGraphsNowTimer !== null) clearTimeout(this.UpdateGraphsNowTimer);
            this.UpdateGraphsNowTimer = setTimeout(this.UpdateGraphsNow, 1000);
          }.bind(this), 
          error: function() {
//deshabilitar¿?
    //            this.$refs.indicator_bar.setValue(null);
          }.bind(this)
        });
      }
    },
    components: { Spinner, DetailTabs, Waveform, Harmonics, Evolution, Events, Parameters, Phasors, Advanced, ToolbarPhases, ToolbarOptions, ToolbarTools }
  }
</script>

<style>
  .voltage-details {

    
  }
</style>

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
      
