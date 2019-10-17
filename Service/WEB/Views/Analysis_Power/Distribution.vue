<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">{{ Time }}</div> <div v-if="ShowWarning" class="title-warning"><img :src="require('Imgs/exclamation.png')">No current or current too low</div>
      <div class="header-buttons" ref="Toolbar">
        <toolbar-tools variable="To" ref="tools" :options="['EXPORT', 'PLAY']" v-on:PlayStatus="$parent.PlayStatusChanged" v-on:ExportClick="ExportClick"></toolbar-tools>
      </div>
    </div>
    <div class="panel-body">
      <spinner v-bind:class="{ 'hide': !ShowSpinner }"></spinner>
      <div class="row">
        <div class="col-sm-12 col-md-8 col-lg-6 graphDiv" ref="graphContainer">
          <vue-apex-charts ref="graph" type="donut" width=380 :options="chartOptions" :series="series"></vue-apex-charts>
        </div>
        <!--div class="col-sm-12 col-md-4 col-lg-6">
          <h4>Voltage and current phase angle</h4>
          <vuetable ref="table" :fields="Columns" :api-mode="false" class="table small table-striped"></vuetable>
        </div-->
      </div>
    </div>
  </div>
</template>

<script>
  import * as oZm from 'JS/openZmeter.js';
  import moment from 'JS/moment.js';
  import Dygraph from 'JS/dygraph.js';
  import Spinner from 'Components/Spinner.vue';
  import Graph from 'Components/Graph.vue';
  import vuetable from 'vuetable-2';
  import 'CSS/table-responsive.css';
  export default {
    data: function() {
      return { ShowWarning: true,  ShowSpinner: true, Time: 'Loading data...',
      Columns: [
          {name: 'Phase', title: 'Phase'},
          {name: 'Channel', title: 'Channel'},
          {name: 'Angle', title: 'Angle'}, 
          {name: 'SecAngle', title: ''},
          {name: 'Value', title: 'Value'}
        ],
        
        series: [44, 55, 13, 43, 22],
        chartOptions: {
          labels: ['Team A', 'Team B', 'Team C', 'Team D', 'Team E'],
 plotOptions: {
    pie: {
      expandOnClick: false
    },
    donut: {
        labels: {
          show: true,
          name: "bliu",
          value: 'blau'
        }
    
  }
 },
          responsive: [{
            breakpoint: 480,
            options: {
              chart: {
                width: 200
              },
              legend: {
                position: 'bottom'
              }
            }
          }]
        }
        
      };
    },
    created: function() {
      this.Data = [];
    },
    mounted: function() {
      $(window).on('resize', this.Redraw.bind(this));
    },
    updated: function(){
//        oZm.createCaptionsForResponsiveTable(this.$refs.table);
    },
    beforeDestroy: function() {
      $(window).off('resize', this.Redraw);
    },
    methods: {
      Redraw: function() {
//        this.$nextTick(function() {
//          if((this.$refs === undefined) || (this.$refs.graphContainer === undefined)) return;
//          var Width = this.$refs.graphContainer.clientWidth - 30;
//          if(Width > 600) Width = 600;
//          this.$refs.graph.Redraw(Width, Width);
//        });
      },
      ExportClick: function() {
//        this.$refs.tools.SaveCVS('Phasors.csv', [['Phase', 'Channel', 'Angle', 'Value']].concat(this.Data));
      },
      SetData(result) {
        var Sum = 0;
        for(var i = 1; i < result['Power_Harmonics'][0]; i++) Sum += result['Power_Harmonics'][0][i];
        this.series = [result['Power_Harmonics'][0][0], Sum];
//        for(var i = 0; i < )
//        var Analyzer = this.$root.SelectedAnalyzer;
//        this.ShowWarning = (oZm.ArrayAbsMax(result.Current) < Analyzer.MinCurrent); 
//        this.Time = 'Last update at ' + moment(result.Sampletime).format('HH:mm:ss.SSS');
//        var Phases = (oZm.Series['VOLTAGE'].Names(Analyzer).length === 1) ? ['L'] : ['R', 'S', 'T'];
//        this.Data = [];
//        var Table = [];
//        var Vectors = [];
//        for(var i = 0; i < result['Voltage'].length; i++) {
//          Vectors.push([i * 2, result.Voltage_Phase[i], result.Voltage[i]]);
//          Vectors.push([i * 2 + 1, result.Voltage_Phase[i] + result.Phi[i], result.Current[i]]);
//          this.Data.push([Phases[i], "Voltage", result.Voltage_Phase[i], result['Voltage'][i]]);
//          this.Data.push([Phases[i], "Current", result.Voltage_Phase[i] + result.Phi[i], result['Current'][i]]);
//          Table.push( { Phase: Phases[i], Channel: "Voltage", Angle: oZm.Round2(result.Voltage_Phase[i]) + '°', Value: oZm.Round2(result['Voltage'][i]) + ' V' } );
//          Table.push( { Phase: Phases[i], Channel: "Current", Angle: oZm.Round2(((result.Phi[i] == null) || (result.Voltage_Phase[i] == null)) ? null : result.Voltage_Phase[i] + result.Phi[i]) + '°', SecAngle: '(' + oZm.Round2(result.Phi[i]) + '°)', Value: oZm.Round2(result.Current[i]) + ' A'} );
//        }
//        this.$refs.graph.SetOptions({ file: Vectors });
//        this.$refs.table.setData( { data: Table });
        this.ShowSpinner = false;
      }
    },
    components: { Graph, vuetable }
  }
</script>

<style scoped>
  .graphDiv {
    text-align: center;
  }
  .graphDiv >>> .graph {
    margin: auto;
  }
  @media screen and (min-width: 600px) {
    .table >>> th:nth-child(1)  {
      width: 40px;
      text-align: center;
    }  
    .table >>> th:nth-child(2)  {
      width: 50px;
      text-align: center;
    }
    .table >>> th:nth-child(3), .table >>> th:nth-child(5)  {
      text-align: right;
    }
    .table >>> td {
      font-size: 16px
    }
    .table >>> td:nth-child(1), .table >>> td:nth-child(1) {
      text-align: center;
    }
    .table >>> td:nth-child(3), .table >>> td:nth-child(5) {
      text-align: right;
    }
  }
  .header-text {
    font-size: 14px;
  }
  .title-warning {
    line-height: 20px;
    font-size: 14px;
  }
  .title-warning img {
    margin: -4px 4px 0px 0px;
  }
</style>
