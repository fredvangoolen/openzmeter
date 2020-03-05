<template>
  <div v-if="analyzer!=null">
    <div class="row">
      <div class="col-lg-12">
        <div class="panel panel-other">
          <div class="panel-heading">
            <span class="header-text">EN50160</span>
            <div class="header-buttons clearfix">
              <!--toolbar-weeks :weeks="Weeks" v-on:WeekChanged="WeekChange"></toolbar-weeks-->
              <toolbar-years :analyzer="analyzer" v-model="Year" aggreg="1H" storage="StatsYear"></toolbar-years>
            </div>
          </div>
          <div class="panel-body main">
            <spinner v-if="Weeks==null"></spinner>
            <counters v-if="Weeks!=null" :data="Weeks" :index="Index"></counters>
          </div>
        </div>
      </div>
    </div>
    <div class="row" v-if="Weeks!=null">
      <div class="col col-lg-4 col-md-12">
        <frequency :data="Weeks" :index="Index" :analyzer="analyzer" :ranges="Frequencies"></frequency>
      </div>
      <div class="col col-lg-4 col-md-12">
        <voltage :data="Weeks" :index="Index" :analyzer="analyzer" :ranges="Voltages"></voltage>
      </div>
      <div class="col col-lg-4 col-md-12">
        <thd :data="Weeks" :index="Index" :analyzer="analyzer" :ranges="THDs"></thd>
      </div>
    </div>
  </div>
  <div v-else-if="analyzer == null">
    <div class="alert alert-danger text-center">
      <p>No selected analyzer</p>
    </div>
  </div>
</template>

<script>
  import moment from 'moment';
  import Frequency from './Frequency.vue';
  import Voltage from './Voltage.vue';
  import Counters from './Counters.vue';
  import Thd from './THD.vue';
  export default {
    props: { analyzer: Object },
    data: function() {
      return { Year: null, Data: null, Events: null, Index: 0,
               Frequencies: [ -Math.INFINITY, -5, -4, -3, -2, -1, -0.9, -0.8, -0.7, -0.6, -0.5, -0.45, -0.40, -0.35, -0.30, -0.25, -0.20, -0.15, -0.10, -0.05, 0.00, 0.05, 0.10, 0.15, 0.20, 0.25, 0.30, 0.35, 0.40, 0.45, 0.5, 0.6, 0.7, 0.8, 0.9, 1, 2, 3, 4, 5, Math.INFINITY ],
               Voltages: [-Math.INFINITY, -20, -15, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 15, 20, Math.INFINITY],
               THDs: [0, 0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5, 5.5, 6, 6.5, 7, 7.5, 8, 8.5, 9, 9.5, 10, 11, 12, 13, 14, 15, Math.INFINITY]
      };
    },
    watch: {
      Year: function(val) {
        this.Events = null;
        this.Data = null;
        var To   = moment().tz(this.analyzer.TimeZone).year(val).endOf('year').valueOf();
        var From = moment().tz(this.analyzer.TimeZone).year(val).startOf('year').valueOf();
        var Promises = [ this.$http.post('getSeries', { From: From, To: To, Aggreg: '1H', Series: ['FREQUENCY_STATS', 'VOLTAGE_STATS', 'VOLTAGE_THD_STATS', 'VOLTAGE_HARMONICS_STATS'], Analyzer: this.analyzer.Analyzer } ),
                         this.$http.post('getEvents', { From: From, To: To, Analyzer: this.analyzer.Analyzer } )];
        Promise.all(Promises).then(
          (result) => {
            this.Data = Object.seal(result[0].body);
            this.Events = Object.seal(result[1].body);
          },
          (fail) => { }
        );
      }
    },
    computed: {
      Weeks: function() {
        if((this.Events == null) || (this.Data == null) || (this.Year == null)) return;
        var Weeks = [];
        Weeks[0] = this.InitWeek(0);
        for(var i = 0; i < this.Events.StartTime.length; i++) {
          var Week = moment(this.Events.StartTime[i]).tz(this.analyzer.TimeZone).isoWeek();
          if(Weeks[Week] == null) Weeks[Week] = this.InitWeek(this.Events.StartTime[i]);
          var Duration = this.Events.Duration[i] / this.Events.SampleRate[i];
          var Percent = this.MaxDiff(this.Events.Peak[i]);
          if((Duration < 60.0) && ((Percent < -15.0) || (Percent > -10.0))) {
            Weeks[Week].Dip = Weeks[Week].Dip + 1;
            Weeks[0].Dip = Weeks[0].Dip + 1;
          }
          if((Duration < 180.0) && (Percent < -99.0)) {
            Weeks[Week].Short = Weeks[Week].Short + 1;
            Weeks[0].Short = Weeks[0].Short + 1;
          }
          if((Duration > 180) && (Percent < -99)) {
            Weeks[Week].Long = Weeks[Week].Long + 1;
            Weeks[0].Long = Weeks[0].Long + 1;
          }
        }
        for(var i = 0; i < this.Data.Sampletime.length; i++) {
          var Week = moment(this.Data.Sampletime[i]).tz(this.analyzer.TimeZone).isoWeek();
          if(Weeks[Week] == null) Weeks[Week] = this.InitWeek(this.Data.Sampletime[i]);
          for(var j = 0; j < this.Frequencies.length - 1; j++) {
            Weeks[Week].Frequency[j] = Weeks[Week].Frequency[j] + this.Data.Frequency_Stats[i][j];
            Weeks[0].Frequency[j] = Weeks[0].Frequency[j] +  this.Data.Frequency_Stats[i][j];
            Weeks[0].FrequencySamples = Weeks[0].FrequencySamples + this.Data.Frequency_Stats[i][j];
            Weeks[Week].FrequencySamples = Weeks[Week].FrequencySamples + this.Data.Frequency_Stats[i][j];
            if((j > 4) && (j < 36)) {
              Weeks[0].FrequencyIN = Weeks[0].FrequencyIN + this.Data.Frequency_Stats[i][j];
              Weeks[Week].FrequencyIN = Weeks[Week].FrequencyIN + this.Data.Frequency_Stats[i][j];
            }
          }
          for(var j = 0; j < this.Voltages.length - 1; j++) {
            Weeks[Week].Voltage[j] = Weeks[Week].Voltage[j] + this.Data.Voltage_Stats[i][j];
            Weeks[0].Voltage[j] = Weeks[0].Voltage[j] + this.Data.Voltage_Stats[i][j];
            Weeks[0].VoltageSamples = Weeks[0].VoltageSamples + this.Data.Voltage_Stats[i][j];
            Weeks[Week].VoltageSamples = Weeks[Week].VoltageSamples + this.Data.Voltage_Stats[i][j];
            if((j > 2) && (j < 24)) {
              Weeks[0].VoltageIN = Weeks[0].VoltageIN + this.Data.Voltage_Stats[i][j];
              Weeks[Week].VoltageIN = Weeks[Week].VoltageIN + this.Data.Voltage_Stats[i][j];
            }
          }
          for(j = 0; j < this.THDs.length - 1; j++) {
            Weeks[Week].THD[j] = Weeks[Week].THD[j] + this.Data.Voltage_THD_Stats[i][j];
            Weeks[0].THD[j] = Weeks[0].THD[j] + this.Data.Voltage_THD_Stats[i][j];
            Weeks[0].THDSamples = Weeks[0].THDSamples + this.Data.Voltage_THD_Stats[i][j];
            Weeks[Week].THDSamples = Weeks[Week].THDSamples + this.Data.Voltage_THD_Stats[i][j];
            if(j < 16) {
              Weeks[0].THDIN = Weeks[0].THDIN + this.Data.Voltage_THD_Stats[i][j];
              Weeks[Week].THDIN = Weeks[Week].THDIN + this.Data.Voltage_THD_Stats[i][j];
            }
          }
          Weeks[Week].HarmonicsSamples = Weeks[Week].HarmonicsSamples + this.Data.Voltage_Harmonics_Stats[i][0] + this.Data.Voltage_Harmonics_Stats[i][1];
          Weeks[0].HarmonicsSamples = Weeks[0].HarmonicsSamples + this.Data.Voltage_Harmonics_Stats[i][0] + this.Data.Voltage_Harmonics_Stats[i][1];
          Weeks[0].HarmonicsIN = Weeks[0].HarmonicsIN + this.Data.Voltage_Harmonics_Stats[i][0];
          Weeks[Week].HarmonicsIN = Weeks[Week].HarmonicsIN + this.Data.Voltage_Harmonics_Stats[i][0];
        }
//        this.Data = null;
//        this.Events = null;
//        for(var Week in this.Weeks) {
//          if((this.Weeks[Week].FrequencySamples == 0) || (this.Weeks[Week].VoltageSamples == 0) || (this.Weeks[Week].THDSamples == 0) || (this.Weeks[Week].HarmonicsSamples == 0)) continue;
//            var InRange = (((this.Weeks[Week].FrequencyIN / this.Weeks[Week].FrequencySamples) * 100.0 >= 99.5) &&
//                            ((this.Weeks[Week].VoltageIN / this.Weeks[Week].VoltageSamples) * 100.0 >= 95.0) &&
//                            ((this.Weeks[Week].THDIN / this.Weeks[Week].VoltageSamples) * 100.0 >= 100.0) &&
//                            ((this.Weeks[Week].HarmonicsIN / this.Weeks[Week].HarmonicsSamples) * 100.0 >= 95.0));
//            this.Weeks[Week].Warning = !InRange;
//        }
        return Weeks;
      }
    },
    methods: {
      InitWeek: function(date) {
        var Week = {};
        Week.Name = (date == 0) ? 'All year' : 'Week ' + moment(date).tz(this.analyzer.TimeZone).isoWeek();
        if(date == 0) {
          Week.Start          = moment().tz(this.analyzer.TimeZone).year(this.Year).startOf('year').valueOf();
          Week.End            = moment().tz(this.analyzer.TimeZone).year(this.Year).endOf('year').valueOf();
        } else {
          Week.Start          = moment(date).tz(this.analyzer.TimeZone).startOf('isoWeek').valueOf();
          Week.End            = moment(date).tz(this.analyzer.TimeZone).endOf('isoWeek').valueOf();
        }
        Week.Frequency        = Array(this.Frequencies.length - 1).fill(0);
        Week.FrequencySamples = 0;
        Week.FrequencyIN      = 0;
        Week.Voltage          = Array(this.Voltages.length - 1).fill(0);;
        Week.VoltageSamples   = 0;
        Week.VoltageIN        = 0;
        Week.THD              = Array(this.THDs.length - 1).fill(0);;
        Week.THDSamples       = 0;
        Week.THDIN            = 0;
        Week.HarmonicsSamples = 0;
        Week.HarmonicsIN      = 0;
        Week.Dip              = 0;
        Week.Short            = 0;
        Week.Long             = 0;
        return Week;
      },
      MaxDiff: function(a) {
        var Max = a[0] - this.analyzer.NominalVoltage;
        for(var n = 1; n < a.length; n++) {
          if(Math.abs(a[n] - this.analyzer.NominalVoltage) > Math.abs(Max)) Max = a[n] - this.analyzer.NominalVoltage;
        }
        return (Max * 100.0) / this.analyzer.NominalVoltage;
      },
//      WeekChange: function(week) {
//        this.$refs.frequency.SetData(this.Weeks[week], this.Frequencies);
//        this.$refs.voltage.SetData(this.Weeks[week], this.Voltages);
//        this.$refs.thd.SetData(this.Weeks[week], this.THDs);
//        this.$refs.counters.SetData(this.Weeks[week]);
//      }
    },
    components: { Frequency, Voltage, Thd, Counters }
}
</script>

<style scoped>
  .header-text {
    font-size: 24px;
    line-height: 16px;
  }
  .main {
    min-height: 100px;
  }
</style>
