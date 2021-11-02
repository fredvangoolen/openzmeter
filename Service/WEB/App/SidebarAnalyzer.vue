<template>
  <q-item clickable class="q-pa-xs" dark @click="ShowDialog = true">
    <q-item-section side class="q-pr-xs">
      <img class="icon" :src="require('Imgs/disconnect32.png')">
      <!--div class="collapse-icon">
      <img class="icon" :src="require('Imgs/disconnect32.png')">
      <span v-if="(status!=null)&&(status.State!=null)&&(status.PendingActions!=null)&&(status.PendingActions>0)">
        <span class="badge badge-menu badge-green" v-if="status.State=='ONLINE'">{{ status.PendingActions }}</span>
        <span class="badge badge-menu badge-yellow" v-if="status.State=='SYNCING'">{{ status.PendingActions }}</span>
        <span class="badge badge-menu badge-red" v-if="status.State=='OFFLINE'">{{ status.PendingActions }}</span>
      </span>
      <span v-else-if="(status!=null)&&(status.State!=null)">
        <img class="badge-icon" :src="require('Imgs/bullet_green.png')" v-if="status.State=='ONLINE'">
        <img class="badge-icon" :src="require('Imgs/bullet_yellow.png')" v-if="status.State=='SYNCING'">
        <img class="badge-icon" :src="require('Imgs/bullet_red.png')" v-if="status.State=='OFFLINE'">
      </span>
      <img class="badge-icon" :src="require('Imgs/bullet_red.png')" v-else>
    </div-->
    </q-item-section>
    <q-item-section>
      <q-item-label class="ellipsis">{{ Analyzer.Name ? Analyzer.Name : 'No analyzer selected' }}</q-item-label>
      <q-item-label caption class="ellipsis">{{ Analyzer.Analyzer ? Analyzer.Analyzer : 'Select one before continue' }}</q-item-label>
    </q-item-section>
    <q-item-section side>
      <q-icon name="menu" />
    </q-item-section>
    <analyzer-dialog v-if="ShowDialog" @close="ShowDialog = false" />
  </q-item>
</template>

<script>
  import AnalyzerDialog from './Analyzer/Dialog.vue';
//  import moment from 'moment';
//  import 'moment-timezone';
  export default {
    props: { status: Object },
    data() {
      return { ShowDialog: false };
    },
    created: function() {
//      if(localStorage.getItem("Analyzer") == null) {
//        this.SetAnalyzer(null);
//        this.Dialog = 'Table';
//      } else {
//        this.Post('getAnalyzer', { Analyzer: localStorage.getItem("Analyzer") }).then(
//          (result) => {
//            result.Analyzer = localStorage.getItem("Analyzer");
//            this.SetAnalyzer(result);
//          },
//          (fail) => {
//            if(fail != null) {
//              this.$toast.open({type:'error', message: "Previous analyzer not found, choose new one", duration: 3000});
//              this.SetAnalyzer(null);
//              this.Dialog = 'Table';
//            }
//          }
//        );
//      }
    },
    methods: {
      SetAnalyzer: function(analyzer) {
        if(analyzer != null) {
//          analyzer.VoltageChannels = analyzer.PhaseNames.length;
//          analyzer.CurrentChannels = analyzer.VoltageChannels + ((analyzer.NeutralName != null) ? 1 : 0);
//          analyzer.PhaseColors = ['#d9534f', '#5cb85c', '#318ce7'].slice(0, analyzer.VoltageChannels);
//          analyzer.NeutralColor = (analyzer.NeutralName != null) ? '#333333' : null;
          analyzer.SerieColor = this.SerieColor.bind(this);
          analyzer.SerieColors = this.SerieColors.bind(this);
          analyzer.SerieName = this.SerieName.bind(this);
          analyzer.SerieUnits = this.SerieUnits.bind(this);
          analyzer.SerieNames = this.SerieNames.bind(this);
//         analyzer.Channels = { Voltage: analyzer.VoltageChannels, Current: analyzer.CurrentChannels, Active_Power: analyzer.VoltageChannels, Frequency: 1 };

          analyzer.HarmonicsNames = ['1st', '2nd', '3rd', '4th', '5th', '6th', '7th', '8th', '9th', '10th', '11th', '12th', '13th', '14th', '15th', '16th', '17th', '18th', '19th', '20th', '21th', '22th', '23th', '24th', '25th', '26th', '27th', '28th', '29th', '30th', '31th', '32th', '33th', '34th', '35th', '36th', '37th', '38th', '39th', '40th', '41th', '42th', '43th', '44th', '45th', '46th', '47th', '48th', '49th', '50th'];

          analyzer.FormatAggregation = this.FormatAggregation.bind(this);
          analyzer.FormatTime = this.FormatTime.bind(this);
          analyzer.FormatDate = this.FormatDate.bind(this);
          analyzer.FormatTimeMS = this.FormatTimeMS.bind(this);
          analyzer.FormatDateTime = this.FormatDateTime.bind(this);
          analyzer.CalcRange = this.CalcRange.bind(this);
//          this.$toast.open({type:'info', message: "Selected analyzer '" + analyzer.Name + "'", duration: 3000});
        }
        this.$emit('update:analyzer', analyzer);
      },
      SerieColors: function(serie) {
        if((this.analyzer.PhaseNames.length == 1) || (serie == 'Frequency')) return [this.SerieColor(serie)];
        let Colors = [];
        Colors = ['#d9534f', '#5cb85c', '#318ce7'].slice(0, this.analyzer.PhaseNames.length);
        if((serie == 'Current') && (this.analyzer.NeutralName != null)) Colors.push('#333333');
        return Colors;
      },
      SerieName: function(serie) {
        if(serie == 'Voltage') return 'Voltage';
        if(serie == 'Current') return 'Current';
        if(serie == 'Active_Power') return 'Active Power';
        if(serie == 'Frequency') return 'Frequency';
        if(serie == 'Current_THD') return 'Current THD';
        return serie;
      },
      SerieNames: function(serie) {
        let Result = [];
        let Name = this.SerieName(serie);
        if((serie == 'Frequency') || (this.analyzer.PhaseNames.length == 1)) return [Name];
        for(let n = 0; n < this.analyzer.PhaseNames.length; n++) Result.push(Name + ' (' + this.analyzer.PhaseNames[n] + ')');
        if(((serie == 'Current') || (serie == 'Current_THD')) && (this.analyzer.NeutralName != null)) Result.push(Name + ' (' + this.analyzer.NeutralName + ')');
        return Result;
      },
      FormatAggregation: function(val) {
        return (val == '1H') ? '1 hour' : (val == '15M') ? '15 minutes' : (val == '10M') ? '10 minutes' : (val == '1M') ? '1 minute' : (val == '3S') ? '3 seconds' : (val == '200MS') ? '200 miliseconds' : 'unknown';
      },
      FormatTime: function(val) {
        return moment(val).tz(this.analyzer.TimeZone).format('HH:mm:ss');
      },
      FormatDate: function(val) {
        return moment(val).tz(this.analyzer.TimeZone).format('DD/MM/YYYY');
      },
      FormatTimeMS: function(val) {
        return moment(val).tz(this.analyzer.TimeZone).format('HH:mm:ss.SSS');
      },
      FormatDateTime: function(val, spacer) {
        if(spacer == null) spacer = ' - ';
        return moment(val).tz(this.analyzer.TimeZone).format('HH:mm:ss') + spacer + moment(val).tz(this.analyzer.TimeZone).format('DD/MM/YYYY');
      },
      Style: function(serie, primary) {
        if(this.analyzer.Channels[serie] == 1) return { Colors: [this.analyzer.Color[serie]], Width: (primary == true) ? 0.5 : 1.5 };
        var Colors = ['#d9534f', '#5cb85c', '#318ce7', '#333333'];
        return { Colors: Colors.slice(0, this.analyzer.Channels[serie]), Width: (primary == true) ? 0.5 : 1.5 }
      },

      ShowDisturbances: function() {
        localStorage.setItem('Period', JSON.stringify({Period: moment().valueOf() - parseInt(localStorage.getItem('LastLogin')), To: moment().valueOf() }));
        this.$router.push({path: '/Events/Disturbances'});
      }
    },
    components: { AnalyzerDialog }
  }
</script>

<style scoped>
  .icon {
    height: 24px;
    width: 24px;
    margin-left: 3px;
  }
</style>