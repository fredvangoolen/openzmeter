<template>
  <div id="wrapper">
    <div class="row">
      <div class="col-lg-12"><voltage></voltage></div>
    </div>
    <div class="row">
      <div class="col-lg-12">
        <ul class="nav nav-tabs" id="DetailTabs">
          <li><a data-toggle="tab" href="#">Waveform</a></li>
          <li><a data-toggle="tab" href="#">Harmonics</a></li>
          <li><a data-toggle="tab" href="#">Evolution</a></li>
          <li><a data-toggle="tab" href="#">Advanced</a></li>
          <li><a data-toggle="tab" href="#">Phasors</a></li>
          <li><a data-toggle="tab" href="#">Parameters</a></li>
          <li><a data-toggle="tab" href="#">Events</a></li>
        </ul>
        <div class="panel panel-other">
          <div id="DetailSpinner" class="spinnerdiv"></div>
          <div class="panel-heading">
            <div class="header-text" id="DetailTime"></div>
            <div class="header-buttons" id="toolbarDetails">
              <div class="btn-group" id="groupDetailPhasesSingle" data-load-block="PhaseV_Block"></div>
              <span>&nbsp;</span>
              <div class="btn-group" id="groupDetailPhasesMulti" data-load-block="PhaseI_Block"></div>
              <span>&nbsp;</span>
              <div class="btn-group">
                <button type="button" class="btn btn-default btn-xs" id="btnShowDetail1Harmonic" data-toggle="tooltip" title="Show/hide 1st harmonic">1st harmonic</button>
                <button type="button" class="btn btn-default btn-xs" id="btnShowDetailCurrent" data-toggle="tooltip" title="Show/hide current">Current</button>
                <button type="button" class="btn btn-default btn-xs" id="btnShowDetailLog" data-toggle="tooltip" title="Use logarithm scale">Log</button>
              </div>
              <span>&nbsp;</span>
              <div class="btn-group" data-load-block="GraphToolsPlay_Block"></div>
            </div>
            <span class="clearfix"> </span>
          </div>
          <div class="panel-body" id="DetailDivs">
            <div id="WaveformPanel">
              <div id="WaveformMaximize">
                <div id="WaveformChart" class="chart big_chart"></div>
              </div>
            </div>
            <div id="HarmonicsPanel">
              <div id="HarmonicsMaximize">              
                <div id="HarmonicsChart" class="chart big_chart"></div>
              </div>
            </div>
            <div id="EvolutionPanel">
            </div>
            <div id="FFTPanel">
              <div id="FFTMaximize">
                <div id="FFTChart" class="chart big_chart"></div>
              </div>
            </div>
            <div id="PhasorPanel">
              <div class="row">
                <div class="col-md-4 col-lg-3 col-xl-2">
                  <div id="PhasorChart" class="chart big_chart"></div>
                </div>
                <div class="col-md-8 col-lg-9 col-xl-10">
                  <h4>Voltage and current phase angle</h4>
                  <table id="PhasorsTable" class="table small table-striped table-responsive">
                    <thead>
                      <tr>
                        <th>Phase</th><th>Channel</th><th>Angle</th><th>Value</th>
                      </tr>
                    </thead>
                    <tbody>
                    </tbody>
                  </table>
                </div>
              </div>
            </div>
            <div id="ParametersPanel">
            </div>
            <div id="EventsPanel">
              <div class="row">
                <div class="col col-lg-12 col-xl-6">
                  <table id="EventsTable" class="table small table-striped table-responsive" width="100%">
                    <thead>
                      <tr>
                        <th style="width: 130px">Date</th><th>Type</th><th>Duration <em>(s)</em></th><th>ΔU <em>(V)</em></th><th>ΔU <em>(%)</em></th><th style="width: 190px"></th>
                      </tr>
                    </thead>
                    <tbody>
                    </tbody>
                  </table>
                </div>
                <div class="col col-lg-12 col-xl-6 hide">
                  <div class="panel panel-noborder">
                    <div id="EventWaveformPanel">
                      <div id="EventWaveformMaximize">
                        <div id="EventChart" class="chart big_chart"></div>
                      </div>
                    </div>
                  </div>
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
  import Voltage from './Voltage.vue';
  export default {
    data () {
      return { };
    },
    components: { Voltage }
  }
</script>

<style>
</style>
