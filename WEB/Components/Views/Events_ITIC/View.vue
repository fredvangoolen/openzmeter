<template>
  <div id="wrapper">
    <div class="row">
      <div class="col-lg-12">
        <div class="panel panel-other">
          <div class="panel-heading">
            <span class="header-text">ITIC Curve (CBEMA)</span>
            <div class="header-buttons">
              <toolbar-years v-on:YearChanged="YearChanged" aggreg="1H"></toolbar-years>
              <span class="small">Time scale:</span><br>
              <div class="btn-group">
                <button type="button" class="btn btn-default btn-xs" id="btnShowEventYear"  data-toggle="tooltip" title="Select exactly a year period from current time">Year</button>
                <button type="button" class="btn btn-default btn-xs" id="btnShowEventMonth" data-toggle="tooltip" title="Select exactly a month period from current time">Month</button>
                <button type="button" class="btn btn-default btn-xs" id="btnShowEventWeek"  data-toggle="tooltip" title="Select exactly a week period from current time">Week</button>
                <button type="button" class="btn btn-default btn-xs" id="btnShowEventDay"   data-toggle="tooltip" title="Select exactly a day period from current time">Day</button>
              </div>
            </div>
            <div class="pull-right">
              <span class="huge-small-unit" id="EventsSelection"></span><br>
            </div>
          </div>
          <div class="panel-body">
            <spinner v-if="ShowSpinner"></spinner>
            <div class="panel-info panel-heading">
              <graph  class="chart" style="height: 50px"></graph>
            </div>
            <div class="row">
              <div class="col-md-12 col-lg-6"><curve ref="itic"></curve></div> 
              <div class="col-md-12 col-lg-6">
                <div class="row" ref="subgraphs">
                  <div class="col-xs-12 col-md-6"><by-date ref="date"></by-date></div>
                  <div class="col-xs-12 col-md-6"><by-type ref="type"></by-type></div>
                  <div class="col-xs-12 col-md-6"><by-rms ref="rms"></by-rms></div>
                  <div class="col-xs-12 col-md-6"><by-duration ref="duration"></by-duration></div>
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
  import ToolbarYears from 'Components/ToolbarYears.vue';
  import Spinner from 'Components/Spinner.vue';
  import Curve from './Curve.vue';
  import ByDate from './ByDate.vue';
  import ByType from './ByType.vue';
  import ByRms from './ByRMS.vue';
  import ByDuration from './ByDuration.vue';
  import Graph from 'Components/Graph.vue';
  import moment from 'JS/moment.js';
  export default {
    data: function() {
      return { ShowSpinner: true };
    },
    created: function() {
      
    },
    methods: {
      ProcessRange: function(start, end) {
        this.$refs.date.SetData(start, end, this.Events);
        this.$refs.type.SetData(start, end, this.Events);
        this.$refs.rms.SetData(start, end, this.Events);
        this.$refs.duration.SetData(start, end, this.Events);
        this.$refs.itic.SetData(start, end, this.Events);
        this.ShowSpinner = false;
      },
      YearChanged: function(year) {
        this.Year = year;
        this.ShowSpinner = true;
        if(this.$root.SelectedAnalyzer === null) return;
        var Analyzer = this.$root.SelectedAnalyzer;
        var Year = moment().year(year);
        $.ajax({ url: "getEvents", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify( { Period: Year.endOf('year').valueOf() - Year.startOf('year').valueOf(), To: Year.endOf('year').valueOf(), LastSample: 0, Analyzer: Analyzer.ID } ),
          success: function(result) {
            this.Events = result;
            this.ProcessRange(Year.startOf('year').valueOf(), Year.endOf('year').valueOf());
          }.bind(this)
        });
      }
    },
    components: { Spinner, ToolbarYears, Graph, Curve, ByDate, ByType, ByRms, ByDuration }
  }
</script>

<style>
</style>

var VoltageData = [];
var EventsData = [];

function GenerateGraphs(min, max) {
  var StartDate = new Date(min);
  var EndDate = new Date(max);
  $('#RangeLabel').text('From ' + StartDate.toLocaleDateString() + ' at ' + StartDate.toLocaleTimeString([], {hour: '2-digit', minute:'2-digit'}) + ' to ' + EndDate.toLocaleDateString() + ' at ' + EndDate.toLocaleTimeString([], {hour: '2-digit', minute:'2-digit'}));
  EventsCount = 0;
  
  $('#EventsSelection').text(EventsCount + ' events in selection');
  
  $('#EventSpinner').hide();
}
function CreateVoltageRangeSelector(result) {
  VoltageData.push([new Date(result['From']), Analyzers[Analyzer].NominalVoltage]);
  for(i = 0; i < result['Sampletime'].length; i++) {
    var Max = -Infinity;
    for(n = 0; n < result['rms_v'][i].length; n++) Max = Math.max(Max, result['rms_v'][i][n]);
    VoltageData.push([new Date(result['Sampletime'][i]), Max]);
  }
  VoltageGraph = new Dygraph(document.getElementById("VoltageRangeChart"),
    VoltageData,
    { labels: ["Time", "Voltage"],
      stepPlot: true,
      dateWindow: [(result['From']), (result['To'])],
      xAxisHeight: 0,
      showRangeSelector: true,
      rangeSelectorHeight: 40,
      legend: "never",
      axisLineColor: "#CCC",
      series: {
        'Voltage': {
          strokeWidth: 0,
          showInRangeSelector: true
        }
      },
      rangeSelectorPlotFillColor: "#FFFFFF",
      rangeSelectorPlotStrokeColor: "#d9534f",
      drawCallback: function(dygraph, is_initial) {
        if(is_initial) return;
        GenerateGraphs(dygraph.dateWindow_[0], dygraph.dateWindow_[1]);
        $('#btnShowEventYear').removeClass('active');
        $('#btnShowEventMonth').removeClass('active');
        $('#btnShowEventWeek').removeClass('active');
        $('#btnShowEventDay').removeClass('active');
      },
      axes: {
        x: { 
          drawAxis: false
        },
        y: { 
          drawAxis: false
        }
      }
    }
  );
  GenerateGraphs((result['From']), (result['To']));
  ButtonsClick();
}



/* Seleccion rapida de periodo */
function ButtonsClick() {
  $('#btnShowEventYear').removeClass('active');
  $('#btnShowEventMonth').removeClass('active');
  $('#btnShowEventWeek').removeClass('active');
  $('#btnShowEventDay').removeClass('active');
  var Range = VoltageGraph.getOption('dateWindow');
  if(this.innerText === 'Day') {
    localStorage.setItem('ITICPeriod', 'Day');
  } else if(this.innerText === 'Week') {
    localStorage.setItem('ITICPeriod', 'Week');
  } else if(this.innerText === 'Month') {
    localStorage.setItem('ITICPeriod', 'Month');
  } else if(this.innerText === 'Year') {
    localStorage.setItem('ITICPeriod', 'Year');
  }
  var Period = localStorage.getItem('ITICPeriod');
  if(Period === 'Day') {
    Period = 86400000;
    $('#btnShowEventDay').addClass('active');
  } else if(Period === 'Week') {
    Period = 604800000;
    $('#btnShowEventWeek').addClass('active');
  } else if(Period === 'Year') {
    Period = 31536000000;
    $('#btnShowEventYear').addClass('active');
  } else {
    Period = 2592000000;
    localStorage.setItem('ITICPeriod', 'Month');
    $('#btnShowEventMonth').addClass('active');
  }
  VoltageGraph.updateOptions({dateWindow: [Range[1] - Period, Range[1]]});
  GenerateGraphs(Range[1] - Period, Range[1]);
}
$('#btnShowEventYear').click(ButtonsClick);
$('#btnShowEventMonth').click(ButtonsClick);
$('#btnShowEventWeek').click(ButtonsClick);
$('#btnShowEventDay').click(ButtonsClick);
