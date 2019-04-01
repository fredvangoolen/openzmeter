<template>
  <div id="wrapper">
    <div class="row">
      <div class="col-lg-12">
        <div class="panel panel-other">
          <div class="panel-heading">
            <span class="header-text">EN50160</span>
            <div class="header-buttons clearfix">
              <toolbar-weeks variable="Week" v-on:PeriodChanged="WeekChange"></toolbar-weeks>
              <toolbar-years v-on:YearChanged="YearChanged" aggreg="1H"></toolbar-years>
            </div>
          </div>
          <div class="panel-body">
            <spinner v-if="ShowSpinner"></spinner>
            <div class="row">
              <div class="col col-lg-3 col-md-6 col-xs-12">
                <div class="panel panel-lightblue-info text-center" style="position: relative">
                  <p><span class="huge-small">Voltage swell or dips</span><br/>Less than 1 min and -15%/+10% nominal voltage</p>
                      <div class="progress" style="margin-bottom:0px; position: relative">
                        <div class="progress-bar" id="DipEvents" role="progressbar" style="width:0%; background-color: #5cb85c; position: absolute"></div>
                        <span id="DipEventsText" style="color: #333; position: absolute; left: 0px; width: 100%; text-align: center"></span>
                      </div>
                      <div style="font-size: 10px">10 to 1000 allowed per year</div>
                </div>
              </div>
              <div class="col col-lg-3 col-md-6 col-xs-12">
                    <div class="panel panel-lightblue-info text-center">
                      <p><span class="huge-small">Short interruptions</span><br/>Less than 3 min and under 1% nominal voltage</p>
                      <div class="progress" style="margin-bottom:0px; position: relative">
                        <div class="progress-bar" id="ShortEvents" role="progressbar" style="width:0%; background-color: #5cb85c; position: absolute"></div>
                        <span id="ShortEventsText" style="color: #333; position: absolute; left: 0px; width: 100%; text-align: center"></span>
                      </div>
                      <div style="font-size: 10px">10 to 100 allowed per year</div>
                    </div>
              </div>                  
                  <div class="col col-lg-3 col-md-6 col-xs-12">
                    <div class="panel panel-lightblue-info text-center">
                      <p><span class="huge-small">Long interruptions</span><br/>More than 3 min and under 1% nominal voltage</p>
                      <div class="progress" style="margin-bottom:0px; position: relative">
                        <div class="progress-bar" id="LongEvents" role="progressbar" style="width:0%; background-color: #5cb85c; position: absolute"></div>
                        <span id="LongEventsText" style="color: #333; position: absolute; left: 0px; width: 100%; text-align: center"></span>
                      </div>
                      <div style="font-size: 10px">10 to 50 allowed per year</div>
                    </div>
                  </div>
                  <div class="col col-lg-3 col-md-6 col-xs-12">
                    <div class="panel panel-lightblue-info text-center">
                      <p><span class="huge-small">Harmonics</span><br/>Harmonics in valid ranges as detailed in EN50160</p>
                      <div class="progress" style="margin-bottom:0px; position: relative">
                        <div class="progress-bar" id="HarmonicsInRange" role="progressbar" style="width:0%; background-color: #5cb85c; position: absolute"></div>
                        <span id="HarmonicsInRangeText" style="color: #333; position: absolute; left: 0px; width: 100%; text-align: center"></span>
                      </div>
                      <div style="float: left; font-size: 10px">90%</div><div style="float:right; font-size: 10px">100%</div>
                      <div class="clearfix"></div>
                    </div>
                  </div>
                </div>
              </div>
            </div>
          </div>

    </div>
    <div class="row">
      <div class="col col-lg-4 col-md-12"><frequency ref="frequency"></frequency></div>
      <div class="col col-lg-4 col-md-12"><voltage ref="voltage"></voltage></div>
      <div class="col col-lg-4 col-md-12"><thd ref="thd"></thd></div>
    </div>
  </div>
</template>

<script>
  import ToolbarYears from 'Components/ToolbarYears.vue';
  import ToolbarWeeks from 'Components/ToolbarWeeks.vue';
  import ProgressBar from 'Components/ProgressBar.vue';
  import Spinner from 'Components/Spinner.vue';
  import moment from 'JS/moment.js';
  export default {
    data: function() {
      return { ShowSpinner: true };
    },
    created: function() {
      this.Events = null;
      this.Data = null;
      this.Weeks = null;
    },
    mounted: function() {
      this.SetAnalyzer();
    },
    methods: {
      ProcessYear: function() {
        if((this.Events === null) || (this.Data === null)) return;
        
        
        
      },
      YearChanged: function(year) {
        this.ShowSpinner = true;
        this.Events = null;
        this.Data = null;
        this.Weeks = null;
        if(this.$root.SelectedAnalyzer === null) return;
        var Analyzer = this.$root.SelectedAnalyzer;
        var Year = moment().year(year);
        $.ajax({ url: "getSeries", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify( { Period: Year.endOf('year').valueOf() - Year.startOf('year').valueOf(), To: Year.endOf('year').valueOf(), Aggreg: '1H', LastSample: 0, Series: ['FREQUENCY_STATS', 'VOLTAGE_STATS', 'VOLTAGE_THD_STATS', 'VOLTAGE_HARMONICS_STATS'], Analyzer: Analyzer.ID } ),
          complete: function(result) {
            this.Data = result;
            this.ProcessYear();
//             $('#ProgressYear').css('width', ((result['Sampletime'][result['Sampletime'].length - 1] - new Date(Date.UTC(selected_year, 0, 1, 0, 0, 0, 0)).getTime()) * 100 / 31536000000)  +'%');
//             if(WeekStats[0] === undefined) InitWeek(0, new Date(selected_year, 0, 1, 0, 0, 0, 0), new Date(selected_year, 11, 31, 23, 59, 59, 999));
//             for(i = 0; i < result['Sampletime'].length - 1; i++) {
//               var Week = GetWeekOfYear(result['Sampletime'][i]);
//               if(WeekStats[Week] === undefined) {
//                 var Start = GetDateOfISOWeek(Week, selected_year);
//                 var End = new Date(Start.getFullYear(), Start.getMonth(), Start.getDate() + 6);
//                 InitWeek(Week, Start, End);
//               }
//               for(j = 0; j < Frequencies.length - 1; j++) {
//                 WeekStats[Week].Frequency[j] = WeekStats[Week].Frequency[j] + result['stat_frequency'][i][j];
//                 WeekStats[0].Frequency[j] = WeekStats[0].Frequency[j] +  result['stat_frequency'][i][j];
//                 WeekStats[0].FrequencySamples = WeekStats[0].FrequencySamples + result['stat_frequency'][i][j];
//                 WeekStats[Week].FrequencySamples = WeekStats[Week].FrequencySamples + result['stat_frequency'][i][j];
//                 if((j > 4) && (j < 36)) {
//                   WeekStats[0].FrequencyIN = WeekStats[0].FrequencyIN + result['stat_frequency'][i][j];
//                   WeekStats[Week].FrequencyIN = WeekStats[Week].FrequencyIN + result['stat_frequency'][i][j];
//                 }
//               }
//               for(j = 0; j < Voltages.length - 1; j++) {
//                 WeekStats[Week].Voltage[j] = WeekStats[Week].Voltage[j] + result['stat_voltage'][i][j];
//                 WeekStats[0].Voltage[j] = WeekStats[0].Voltage[j] + result['stat_voltage'][i][j];
//                 WeekStats[0].VoltageSamples = WeekStats[0].VoltageSamples + result['stat_voltage'][i][j];
//                 WeekStats[Week].VoltageSamples = WeekStats[Week].VoltageSamples + result['stat_voltage'][i][j];
//                 if((j > 2) && (j < 24)) {
//                   WeekStats[0].VoltageIN = WeekStats[0].VoltageIN + result['stat_voltage'][i][j];
//                   WeekStats[Week].VoltageIN = WeekStats[Week].VoltageIN + result['stat_voltage'][i][j];
//                 }
//               }
//               for(j = 0; j < THDs.length - 1; j++) {
//                 WeekStats[Week].THD[j] = WeekStats[Week].THD[j] + result['stat_thd'][i][j];
//                 WeekStats[0].THD[j] = WeekStats[0].THD[j] + result['stat_thd'][i][j];
//                 WeekStats[0].THDSamples = WeekStats[0].THDSamples + result['stat_thd'][i][j];
//                 WeekStats[Week].THDSamples = WeekStats[Week].THDSamples + result['stat_thd'][i][j];
//                 if(j < 16) {
//                   WeekStats[0].THDIN = WeekStats[0].THDIN + result['stat_thd'][i][j];
//                   WeekStats[Week].THDIN = WeekStats[Week].THDIN + result['stat_thd'][i][j];
//                 }
//               }
//               WeekStats[Week].HarmonicsSamples = WeekStats[Week].HarmonicsSamples + result['stat_harmonics'][i][0] + result['stat_harmonics'][i][1];
//               WeekStats[0].HarmonicsSamples = WeekStats[0].HarmonicsSamples + result['stat_harmonics'][i][0] + result['stat_harmonics'][i][1];
//               WeekStats[0].HarmonicsIN = WeekStats[0].HarmonicsIN + result['stat_harmonics'][i][1];
//               WeekStats[Week].HarmonicsIN = WeekStats[Week].HarmonicsIN + result['stat_harmonics'][i][1];
//             }
//             for(var Week in WeekStats) {
//               if((WeekStats[Week].FrequencySamples == 0) || (WeekStats[Week].VoltageSamples == 0) || (WeekStats[Week].THDSamples == 0) || (WeekStats[Week].HarmonicsSamples == 0)) continue;
//               var InRange = (((WeekStats[Week].FrequencyIN / WeekStats[Week].FrequencySamples) * 100.0 >= 99.5) && 
//                              ((WeekStats[Week].VoltageIN / WeekStats[Week].VoltageSamples) * 100.0 >= 95.0) && 
//                              ((WeekStats[Week].THDIN / WeekStats[Week].VoltageSamples) * 100.0 >= 100.0) &&
//                              ((WeekStats[Week].HarmonicsIN / WeekStats[Week].HarmonicsSamples) * 100.0 >= 100.0));
//               $('#WeekList').append('<button type="button" class="btn btn-default btn-xs" id="btnWeek_' + Week + '" onclick="LoadWeek(' + Week + ')" data-toggle="tooltip" title="From ' + WeekStats[Week].Start.toLocaleDateString() + " to " + WeekStats[Week].End.toLocaleDateString() + '">' + ((Week == 0) ? 'Totals' : Week) + (InRange ? '' : ' <i class="fa fa-warning"></i>') + '</button>');
//             }
//             LoadWeek(0);
//             $('#EN50160Spinner').hide();
           
         }.bind(this)
       });
        $.ajax({ url: "getEvents", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify( { Period: Year.endOf('year').valueOf() - Year.startOf('year').valueOf(), To: Year.endOf('year').valueOf(), LastSample: 0, Analyzer: Analyzer.ID } ),
          complete: function(result) {
            this.Events = result;
            this.ProcessYear();
          }.bind(this)
        });
      },
      WeekChange: function(week) {
        
      }
    },
    components: { ToolbarYears, ProgressBar, ToolbarWeeks, Spinner }
}
</script>

<style scoped>
  .header-text {
    font-size: 24px;
    line-height: 16px;
  }
</style>
