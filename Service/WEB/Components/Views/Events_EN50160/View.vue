<template>
  <div>
    <div class="row">
      <div class="col-lg-12">
        <div class="panel panel-other">
          <div class="panel-heading">
            <span class="header-text">EN50160</span>
            <div class="header-buttons clearfix">
              <toolbar-weeks :weeks="Weeks" v-on:WeekChanged="WeekChange"></toolbar-weeks>
              <toolbar-years v-on:YearChanged="YearChanged" aggreg="1H"></toolbar-years>
            </div>
          </div>
          <counters ref="counters"></counters>
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
  import Frequency from './Frequency.vue';
  import Voltage from './Voltage.vue';
  import Counters from './Counters.vue';
  import Thd from './THD.vue';
  export default {
    data: function() {
      return { ShowSpinner: true, Weeks: [] };
    },
    created: function() {
      this.Year = null;
      this.Events = null;
      this.Data = null;      
      this.Frequencies = [ -Math.INFINITY, -5, -4, -3, -2, -1, -0.9, -0.8, -0.7, -0.6, -0.5, -0.45, -0.40, -0.35, -0.30, -0.25, -0.20, -0.15, -0.10, -0.05, 0.00, 0.05, 0.10, 0.15, 0.20, 0.25, 0.30, 0.35, 0.40, 0.45, 0.5, 0.6, 0.7, 0.8, 0.9, 1, 2, 3, 4, 5, Math.INFINITY ];
      this.Voltages    = [-Math.INFINITY, -20, -15, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 15, 20, Math.INFINITY];
      this.THDs        = [0, 0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5, 5.5, 6, 6.5, 7, 7.5, 8, 8.5, 9, 9.5, 10, 11, 12, 13, 14, 15, Math.INFINITY];
    },
    methods: {
      InitWeek: function(week, start, end) {
        this.Weeks[week] = [];
        this.Weeks[week].Name = (week === 0) ? 'All year' : 'Week ' + week;
        this.Weeks[week].Start            = start;
        this.Weeks[week].End              = end;
        this.Weeks[week].Frequency        = [];
        this.Weeks[week].FrequencySamples = 0;
        this.Weeks[week].FrequencyIN      = 0;
        this.Weeks[week].Voltage          = [];
        this.Weeks[week].VoltageSamples   = 0;
        this.Weeks[week].VoltageIN        = 0;
        this.Weeks[week].THD              = [];
        this.Weeks[week].THDSamples       = 0;
        this.Weeks[week].THDIN            = 0;
        this.Weeks[week].HarmonicsSamples = 0;
        this.Weeks[week].HarmonicsIN      = 0;
        this.Weeks[week].Dip              = 0;
        this.Weeks[week].Short            = 0;
        this.Weeks[week].Long             = 0;
        for(var j = 0; j < this.Frequencies.length - 1; j++) this.Weeks[week].Frequency.push(0);
        for(var j = 0; j < this.Voltages.length - 1; j++) this.Weeks[week].Voltage.push(0);
        for(var j = 0; j < this.THDs.length - 1; j++) this.Weeks[week].THD.push(0);
      },
      ProcessYear: function() {
        if((this.Events === null) || (this.Data === null)) return;
        var Analyzer = this.$root.SelectedAnalyzer;
        this.Weeks = [];
        if(this.Weeks[0] === undefined) this.InitWeek(0, moment().year(this.Year).startOf('year').valueOf(), moment().year(this.Year).endOf('year').valueOf());
        for(var i = 0; i < this.Events['StartTime'].length; i++) {
          var Week = moment(this.Events['StartTime'][i]).isoWeek();
          if(this.Weeks[Week] === undefined) this.InitWeek(Week, moment(this.Events['StartTime'][i]).startOf('isoWeek').valueOf(), moment(this.Events['StartTime'][i]).endOf('isoWeek').valueOf());
          var Duration = this.Events['Duration'][i] / this.Events['SampleRate'][i];
          var Percent = this.Events['Percent'][i];
          if((Duration < 60.0) && ((Percent < -15.0) || (Percent > -10.0))) {
            this.Weeks[Week].Dip = this.Weeks[Week].Dip + 1;
            this.Weeks[0].Dip = this.Weeks[0].Dip + 1;
          }
          if((Duration < 180.0) && (Percent < -99.0)) {
            this.Weeks[Week].Short = this.Weeks[Week].Short + 1;
            this.Weeks[0].Short = this.Weeks[0].Short + 1;
          }
          if((Duration > 180) && (Percent < -99)) {
            this.Weeks[Week].Long = this.Weeks[Week].Long + 1;
            this.Weeks[0].Long = this.Weeks[0].Long + 1;
          }
        }
        for(var i = 0; i < this.Data['Sampletime'].length; i++) {
          var Week = moment(this.Data['Sampletime'][i]).isoWeek();
          if(this.Weeks[Week] === undefined) this.InitWeek(Week, moment(this.Data['Sampletime'][i]).startOf('isoWeek').valueOf(), moment(this.Data['Sampletime'][i]).endOf('isoWeek').valueOf());
          for(var j = 0; j < this.Frequencies.length - 1; j++) {
            this.Weeks[Week].Frequency[j] = this.Weeks[Week].Frequency[j] + this.Data['Frequency_Stats'][i][j];
            this.Weeks[0].Frequency[j] = this.Weeks[0].Frequency[j] +  this.Data['Frequency_Stats'][i][j];
            this.Weeks[0].FrequencySamples = this.Weeks[0].FrequencySamples + this.Data['Frequency_Stats'][i][j];
            this.Weeks[Week].FrequencySamples = this.Weeks[Week].FrequencySamples + this.Data['Frequency_Stats'][i][j];
            if((j > 4) && (j < 36)) {
              this.Weeks[0].FrequencyIN = this.Weeks[0].FrequencyIN + this.Data['Frequency_Stats'][i][j];
              this.Weeks[Week].FrequencyIN = this.Weeks[Week].FrequencyIN + this.Data['Frequency_Stats'][i][j];
            }
          }
          for(var j = 0; j < this.Voltages.length - 1; j++) {
            this.Weeks[Week].Voltage[j] = this.Weeks[Week].Voltage[j] + this.Data['Voltage_Stats'][i][j];
            this.Weeks[0].Voltage[j] = this.Weeks[0].Voltage[j] + this.Data['Voltage_Stats'][i][j];
            this.Weeks[0].VoltageSamples = this.Weeks[0].VoltageSamples + this.Data['Voltage_Stats'][i][j];
            this.Weeks[Week].VoltageSamples = this.Weeks[Week].VoltageSamples + this.Data['Voltage_Stats'][i][j];
            if((j > 2) && (j < 24)) {
              this.Weeks[0].VoltageIN = this.Weeks[0].VoltageIN + this.Data['Voltage_Stats'][i][j];
              this.Weeks[Week].VoltageIN = this.Weeks[Week].VoltageIN + this.Data['Voltage_Stats'][i][j];
            }
          }
          for(j = 0; j < this.THDs.length - 1; j++) {
            this.Weeks[Week].THD[j] = this.Weeks[Week].THD[j] + this.Data['Voltage_THD_Stats'][i][j];
            this.Weeks[0].THD[j] = this.Weeks[0].THD[j] + this.Data['Voltage_THD_Stats'][i][j];
            this.Weeks[0].THDSamples = this.Weeks[0].THDSamples + this.Data['Voltage_THD_Stats'][i][j];
            this.Weeks[Week].THDSamples = this.Weeks[Week].THDSamples + this.Data['Voltage_THD_Stats'][i][j];
            if(j < 16) {
              this.Weeks[0].THDIN = this.Weeks[0].THDIN + this.Data['Voltage_THD_Stats'][i][j];
              this.Weeks[Week].THDIN = this.Weeks[Week].THDIN + this.Data['Voltage_THD_Stats'][i][j];
            }
          }
          this.Weeks[Week].HarmonicsSamples = this.Weeks[Week].HarmonicsSamples + this.Data['Voltage_Harmonics_Stats'][i][0] + this.Data['Voltage_Harmonics_Stats'][i][1];
          this.Weeks[0].HarmonicsSamples = this.Weeks[0].HarmonicsSamples + this.Data['Voltage_Harmonics_Stats'][i][0] + this.Data['Voltage_Harmonics_Stats'][i][1];
          this.Weeks[0].HarmonicsIN = this.Weeks[0].HarmonicsIN + this.Data['Voltage_Harmonics_Stats'][i][0];
          this.Weeks[Week].HarmonicsIN = this.Weeks[Week].HarmonicsIN + this.Data['Voltage_Harmonics_Stats'][i][0];
        }
        this.Data = null;
        this.Events = null;
        for(var Week in this.Weeks) {
          if((this.Weeks[Week].FrequencySamples == 0) || (this.Weeks[Week].VoltageSamples == 0) || (this.Weeks[Week].THDSamples == 0) || (this.Weeks[Week].HarmonicsSamples == 0)) continue;
            var InRange = (((this.Weeks[Week].FrequencyIN / this.Weeks[Week].FrequencySamples) * 100.0 >= 99.5) && 
                            ((this.Weeks[Week].VoltageIN / this.Weeks[Week].VoltageSamples) * 100.0 >= 95.0) && 
                            ((this.Weeks[Week].THDIN / this.Weeks[Week].VoltageSamples) * 100.0 >= 100.0) &&
                            ((this.Weeks[Week].HarmonicsIN / this.Weeks[Week].HarmonicsSamples) * 100.0 >= 95.0));
            this.Weeks[Week].Warning = !InRange;
        }
        this.ShowSpinner = false;
      },
      YearChanged: function(year) {
        this.Year = year;
        this.ShowSpinner = true;
        this.Events = null;
        this.Data = null;
        this.Weeks = [];
        if(this.$root.SelectedAnalyzer === null) return;
        var Analyzer = this.$root.SelectedAnalyzer;
        var Year = moment().year(year);
        $.ajax({ url: "getSeries", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify( { Period: Year.endOf('year').valueOf() - Year.startOf('year').valueOf(), To: Year.endOf('year').valueOf(), Aggreg: '1H', LastSample: 0, Series: ['FREQUENCY_STATS', 'VOLTAGE_STATS', 'VOLTAGE_THD_STATS', 'VOLTAGE_HARMONICS_STATS'], Analyzer: Analyzer.ID } ),
          success: function(result) {
            this.Data = result;
            this.ProcessYear();
         }.bind(this)
       });
        $.ajax({ url: "getEvents", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify( { Period: Year.endOf('year').valueOf() - Year.startOf('year').valueOf(), To: Year.endOf('year').valueOf(), LastSample: 0, Analyzer: Analyzer.ID } ),
          success: function(result) {
            this.Events = result;
            this.ProcessYear();
          }.bind(this)
        });
      },
      WeekChange: function(week) {
        this.$refs.frequency.SetData(this.Weeks[week], this.Frequencies);
        this.$refs.voltage.SetData(this.Weeks[week], this.Voltages);
        this.$refs.thd.SetData(this.Weeks[week], this.THDs);
        this.$refs.counters.SetData(this.Weeks[week]);
      }
    },
    components: { ToolbarYears, ProgressBar, ToolbarWeeks, Spinner, Frequency, Voltage, Thd, Counters }
}
</script>

<style scoped>
  .header-text {
    font-size: 24px;
    line-height: 16px;
  }
</style>
