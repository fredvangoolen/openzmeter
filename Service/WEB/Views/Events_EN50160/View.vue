<template>
  <div v-if="analyzer!=null">
    <div class="row">
      <div class="col-lg-12">
        <div class="panel panel-other">
          <div class="panel-heading">
            <span class="header-text">EN50160</span>
            <div class="header-buttons clearfix">
              <toolbar-weeks :weeks="Weeks" v-model="Index"></toolbar-weeks>
              <toolbar-years :analyzer="analyzer" v-model="Year" aggreg="1H" storage="StatsYear"></toolbar-years>
            </div>
          </div>
          <div class="panel-body main">
            <spinner v-if="!Ready"></spinner>
            <counters v-if="Ready" :data="Weeks[Index]"></counters>
          </div>
        </div>
      </div>
    </div>
    <div class="row" v-if="Ready">
      <div :class="[(analyzer.VoltageChannels==1) ? 'col-lg-4' : 'col-lg-6', 'col-md-12']">
        <frequency :data="Weeks[Index]" :analyzer="analyzer" :ranges="Frequencies"></frequency>
      </div>
      <div :class="[(analyzer.VoltageChannels==1) ? 'col-lg-4' : 'col-lg-6', 'col-md-12']">
        <voltage :data="Weeks[Index]" :analyzer="analyzer" :ranges="Voltages"></voltage>
      </div>
      <div :class="[(analyzer.VoltageChannels==1) ? 'col-lg-4' : 'col-lg-6', 'col-md-12']">
        <thd :data="Weeks[Index]" :analyzer="analyzer" :ranges="THDs"></thd>
      </div>
      <div v-if="analyzer.VoltageChannels>1" class="col-lg-6 col-md-12">
        <unbalance :data="Weeks[Index]" :analyzer="analyzer" :ranges="Unbalances"></unbalance>
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
  import Unbalance from './Unbalance.vue';
  export default {
    props: { analyzer: Object },
    data: function() {
      return { Ready: false, Year: null, Index: 'All year', TransferEvents: this.EventsTransfer(), TransferSeries: this.SeriesTransfer(['Frequency_Stats', 'Voltage_Stats', 'Voltage_THD_Stats', 'Voltage_Harmonics_Stats', 'Unbalance_Stats']),
               Frequencies: [ -Math.INFINITY, -5, -4, -3, -2, -1, -0.9, -0.8, -0.7, -0.6, -0.5, -0.45, -0.40, -0.35, -0.30, -0.25, -0.20, -0.15, -0.10, -0.05, 0.00, 0.05, 0.10, 0.15, 0.20, 0.25, 0.30, 0.35, 0.40, 0.45, 0.5, 0.6, 0.7, 0.8, 0.9, 1, 2, 3, 4, 5, Math.INFINITY ],
               Voltages: [-Math.INFINITY, -20, -15, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 15, 20, Math.INFINITY],
               THDs: [0, 0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5, 5.5, 6, 6.5, 7, 7.5, 8, 8.5, 9, 9.5, 10, 11, 12, 13, 14, 15, Math.INFINITY],
               Unbalances: [0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0, 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9, 2.0, 2.25, 2.50, 2.75, 3.0, 3.25, 3.50, 3.75, 4.0, 4.5, 5.0, Math.INFINITY]
      };
    },
    watch: {
      analyzer: function() {
        this.Ready = false;
        this.TransferEvents.Update(this.analyzer, ((this.analyzer == null) || (this.Year == null)) ? [0, 0] : [moment().tz(this.analyzer.TimeZone).year(this.Year).startOf('year').valueOf(), moment().tz(this.analyzer.TimeZone).year(this.Year).endOf('year').valueOf()]);
        this.TransferSeries.Update(this.analyzer, '1H', ((this.analyzer == null) || (this.Year == null)) ? [0, 0] : [moment().tz(this.analyzer.TimeZone).year(this.Year).startOf('year').valueOf(), moment().tz(this.analyzer.TimeZone).year(this.Year).endOf('year').valueOf()]);
      },
      Year: function(val) {
        this.Ready = false;
        this.TransferEvents.Update(this.analyzer, (this.analyzer == null) ? [0, 0] : [moment().tz(this.analyzer.TimeZone).year(val).startOf('year').valueOf(), moment().tz(this.analyzer.TimeZone).year(val).endOf('year').valueOf()]);
        this.TransferSeries.Update(this.analyzer, '1H', (this.analyzer == null) ? [0, 0] : [moment().tz(this.analyzer.TimeZone).year(val).startOf('year').valueOf(), moment().tz(this.analyzer.TimeZone).year(val).endOf('year').valueOf()]);
      }
    },
    computed: {
      Weeks: function() {
        this.Index = 'All year';
        var Weeks = {};
        Weeks['All year'] = this.InitWeek(0);
        if((this.TransferEvents.Data == null) || (this.TransferSeries.Data == null)) return Weeks;
        var TimeCol       = this.TransferEvents.Data.Headers.indexOf('Time');
        var DurationCol   = this.TransferEvents.Data.Headers.indexOf('Duration');
        var SampleRateCol = this.TransferEvents.Data.Headers.indexOf('SampleRate');
        var PeakCol       = this.TransferEvents.Data.Headers.indexOf('Peak');
        for(var i = 0; i < this.TransferEvents.Data.Rows.length; i++) {
          var Week = 'Week ' + moment(this.TransferEvents.Data.Rows[i][TimeCol]).tz(this.analyzer.TimeZone).isoWeek();
          if(Weeks[Week] == null) Weeks[Week] = this.InitWeek(this.TransferEvents.Data.Rows[i][TimeCol]);
          var Duration = this.TransferEvents.Data.Rows[i][DurationCol] / this.TransferEvents.Data.Rows[i][SampleRateCol];
          var Percent = this.MaxDiff(this.TransferEvents.Data.Rows[i][PeakCol]);
          if((Duration < 60.0) && ((Percent > -15.0) && (Percent < 10.0))) {
            Weeks[Week].Dip = Weeks[Week].Dip + 1;
            Weeks['All year'].Dip = Weeks['All year'].Dip + 1;
          }
          if((Duration < 180.0) && (Percent < -99.0)) {
            Weeks[Week].Short = Weeks[Week].Short + 1;
            Weeks['All year'].Short = Weeks['All year'].Short + 1;
          }
          if((Duration > 180) && (Percent < -99.0)) {
            Weeks[Week].Long = Weeks[Week].Long + 1;
            Weeks['All year'].Long = Weeks['All year'].Long + 1;
          }
        }
        TimeCol = this.TransferSeries.Data.Headers.indexOf('Time');
        var FrequencyCol = this.TransferSeries.Data.Headers.indexOf('Frequency_Stats');
        var VoltageCol = this.TransferSeries.Data.Headers.indexOf('Voltage_Stats');
        var THDCol = this.TransferSeries.Data.Headers.indexOf('THD_Stats');
        var HarmonicsCol = this.TransferSeries.Data.Headers.indexOf('Harmonics_Stats');
        var UnbalanceCol = this.TransferSeries.Data.Headers.indexOf('Unbalance_Stats');
        for(var i = 0; i < this.TransferSeries.Data.Rows.length; i++) {
          var Week = 'Week ' + moment(this.TransferSeries.Data.Rows[i][TimeCol]).tz(this.analyzer.TimeZone).isoWeek();
          if(Weeks[Week] == null) Weeks[Week] = this.InitWeek(this.TransferSeries.Data.Rows[i][TimeCol]);
          for(var j = 0; j < this.Frequencies.length - 1; j++) {
            Weeks[Week].Frequency[j] = Weeks[Week].Frequency[j] + this.TransferSeries.Data.Rows[i][FrequencyCol][j];
            Weeks[Week].FrequencySamples = Weeks[Week].FrequencySamples + this.TransferSeries.Data.Rows[i][FrequencyCol][j];
            Weeks['All year'].Frequency[j] = Weeks['All year'].Frequency[j] +  this.TransferSeries.Data.Rows[i][FrequencyCol][j];
            Weeks['All year'].FrequencySamples = Weeks['All year'].FrequencySamples + this.TransferSeries.Data.Rows[i][FrequencyCol][j];
            if((j > 4) && (j < 36)) {
              Weeks['All year'].FrequencyIN = Weeks['All year'].FrequencyIN + this.TransferSeries.Data.Rows[i][FrequencyCol][j];
              Weeks[Week].FrequencyIN = Weeks[Week].FrequencyIN + this.TransferSeries.Data.Rows[i][FrequencyCol][j];
            }
          }
          for(var j = 0; j < this.Voltages.length - 1; j++) {
            Weeks[Week].Voltage[j] = Weeks[Week].Voltage[j] + this.TransferSeries.Data.Rows[i][VoltageCol][j];
            Weeks[Week].VoltageSamples = Weeks[Week].VoltageSamples + this.TransferSeries.Data.Rows[i][VoltageCol][j];
            Weeks['All year'].Voltage[j] = Weeks['All year'].Voltage[j] + this.TransferSeries.Data.Rows[i][VoltageCol][j];
            Weeks['All year'].VoltageSamples = Weeks['All year'].VoltageSamples + this.TransferSeries.Data.Rows[i][VoltageCol][j];
            if((j > 2) && (j < 24)) {
              Weeks['All year'].VoltageIN = Weeks['All year'].VoltageIN + this.TransferSeries.Data.Rows[i][VoltageCol][j];
              Weeks[Week].VoltageIN = Weeks[Week].VoltageIN + this.TransferSeries.Data.Rows[i][VoltageCol][j];
            }
          }
          for(var j = 0; j < this.THDs.length - 1; j++) {
            Weeks[Week].THD[j] = Weeks[Week].THD[j] + this.TransferSeries.Data.Rows[i][THDCol][j];
            Weeks[Week].THDSamples = Weeks[Week].THDSamples + this.TransferSeries.Data.Rows[i][THDCol][j];
            Weeks['All year'].THD[j] = Weeks['All year'].THD[j] + this.TransferSeries.Data.Rows[i][THDCol][j];
            Weeks['All year'].THDSamples = Weeks['All year'].THDSamples + this.TransferSeries.Data.Rows[i][THDCol][j];
            if(j < 16) {
              Weeks['All year'].THDIN = Weeks['All year'].THDIN + this.TransferSeries.Data.Rows[i][THDCol][j];
              Weeks[Week].THDIN = Weeks[Week].THDIN + this.TransferSeries.Data.Rows[i][THDCol][j];
            }
          }
          for(var j = 0; j < this.Unbalances.length - 1; j++) {
            Weeks[Week].Unbalance[j] = Weeks[Week].Unbalance[j] + this.TransferSeries.Data.Rows[i][UnbalanceCol][j];
            Weeks[Week].UnbalanceSamples = Weeks[Week].UnbalanceSamples + this.TransferSeries.Data.Rows[i][UnbalanceCol][j];
            Weeks['All year'].Unbalance[j] = Weeks['All year'].Unbalance[j] + this.TransferSeries.Data.Rows[i][UnbalanceCol][j];
            Weeks['All year'].UnbalanceSamples = Weeks['All year'].UnbalanceSamples + this.TransferSeries.Data.Rows[i][UnbalanceCol][j];
            if(j < 21) {
              Weeks['All year'].UnbalanceIN = Weeks['All year'].UnbalanceIN + this.TransferSeries.Data.Rows[i][UnbalanceCol][j];
              Weeks[Week].UnbalanceIN = Weeks[Week].UnbalanceIN + this.TransferSeries.Data.Rows[i][UnbalanceCol][j];
            }
          }
          Weeks[Week].HarmonicsSamples = Weeks[Week].HarmonicsSamples + this.TransferSeries.Data.Rows[i][HarmonicsCol][0] + this.TransferSeries.Data.Rows[i][HarmonicsCol][1];
          Weeks[Week].HarmonicsIN = Weeks[Week].HarmonicsIN + this.TransferSeries.Data.Rows[i][HarmonicsCol][0];
          Weeks['All year'].HarmonicsSamples = Weeks['All year'].HarmonicsSamples + this.TransferSeries.Data.Rows[i][HarmonicsCol][0] + this.TransferSeries.Data.Rows[i][HarmonicsCol][1];
          Weeks['All year'].HarmonicsIN = Weeks['All year'].HarmonicsIN + this.TransferSeries.Data.Rows[i][HarmonicsCol][0];
        }
        for(var Week in Weeks) {
          if((Weeks[Week].FrequencySamples == 0) || (Weeks[Week].VoltageSamples == 0) || (Weeks[Week].THDSamples == 0) || (Weeks[Week].HarmonicsSamples == 0)) continue;
          var InRange = (((Weeks[Week].FrequencyIN / Weeks[Week].FrequencySamples) * 100.0 >= 99.5) &&
                         ((Weeks[Week].VoltageIN / Weeks[Week].VoltageSamples) * 100.0 >= 95.0) &&
                         ((Weeks[Week].THDIN / Weeks[Week].VoltageSamples) * 100.0 >= 100.0) &&
                         ((Weeks[Week].HarmonicsIN / Weeks[Week].HarmonicsSamples) * 100.0 >= 95.0));
          Weeks[Week].Warning = !InRange;
        }
        this.Ready = true;
        return Weeks;
      }
    },
    methods: {
      InitWeek: function(date) {
        var Week = {};
        Week.Warning          = false;
        if((this.analyzer == null) || (this.Year == null)) {
          Week.Start          = 0;
          Week.End            = 0;
        } else if(date == 0) {
          Week.Start          = moment().tz(this.analyzer.TimeZone).year(this.Year).startOf('year').valueOf();
          Week.End            = moment().tz(this.analyzer.TimeZone).year(this.Year).endOf('year').valueOf();
        } else {
          Week.Start          = moment(date).tz(this.analyzer.TimeZone).startOf('isoWeek').valueOf();
          Week.End            = moment(date).tz(this.analyzer.TimeZone).endOf('isoWeek').valueOf();
        }
        Week.Frequency        = new Array(this.Frequencies.length - 1).fill(0);
        Week.FrequencySamples = 0;
        Week.FrequencyIN      = 0;
        Week.Voltage          = new Array(this.Voltages.length - 1).fill(0);
        Week.VoltageSamples   = 0;
        Week.VoltageIN        = 0;
        Week.THD              = new Array(this.THDs.length - 1).fill(0);
        Week.THDSamples       = 0;
        Week.Unbalance        = new Array(this.Unbalances.length - 1).fill(0);
        Week.UnbalanceSamples = 0;
        Week.UnbalanceIN      = 0;
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
    components: { Frequency, Voltage, Thd, Counters, Unbalance }
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