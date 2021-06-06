<template>
  <div class="this" :class="{ expanded: expanded }">
    <div class="collapse-icon">
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
    </div>
    <div class="btn-group">
      <button type="button" class="btn btn-default btn-xs" @click="Dialog='Table'" title="Select and configure avaliable analyzers"><img :src="require('Imgs/magnifier.png')"></button>
      <button type="button" class="btn btn-default btn-xs" :disabled="analyzer==null" @click="Dialog='Info'" title="Analyzer information and status"><img :src="require('Imgs/system_monitor.png')"></button>
    </div>
    <div class="name">{{ (analyzer == null) ? 'None selected' : analyzer.Name }}</div>
    <analyzer-table v-if="Dialog=='Table'" :analyzer="analyzer" @update:analyzer="SetAnalyzer($event)" @close="Dialog=null"></analyzer-table>
    <analyzer-info v-if="Dialog=='Info'" :analyzer="analyzer" @close="Dialog=null"></analyzer-info>
  </div>
</template>

<script>
  import AnalyzerTable from './Analyzer/Table.vue';
  import AnalyzerInfo from './Analyzer/Info.vue';
  import moment from 'moment';
  import 'moment-timezone';
  export default {
    props: { analyzer: Object, expanded: Boolean, status: Object },
    data() {
      return { Dialog: null };
    },
    created: function() {
      if(localStorage.getItem("Analyzer") == null) {
        this.SetAnalyzer(null);
        this.Dialog = 'Table';
      } else {
        this.Post('getAnalyzer', { Analyzer: localStorage.getItem("Analyzer") }).then(
          (result) => {
            result.Analyzer = localStorage.getItem("Analyzer");
            this.SetAnalyzer(result);
          },
          (fail) => {
            if(fail != null) {
              this.$toast.open({type:'error', message: "Previous analyzer not found, choose new one", duration: 3000});
              this.SetAnalyzer(null);
              this.Dialog = 'Table';
            }
          }
        );
      }
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
          this.$toast.open({type:'info', message: "Selected analyzer '" + analyzer.Name + "'", duration: 3000});
        }
        this.$emit('update:analyzer', analyzer);
      },
      SerieColor: function(serie) {
        if(serie == 'Frequency') return '#5cb85c';
        if(serie == 'Voltage') return '#d9534f';
        if(serie == 'Current') return '#428bca';
        if(serie == 'Active_Power') return '#f0ad4e';
        if(serie == 'Reactive_Power') return '#DF842A';
        if(serie == 'Apparent_Power') return '#DF842A';
        if(serie == 'Active_Energy') return '#3d3d8b';
        if(serie == 'Reactive_Energy') return '#643d8b';
      },
      SerieColors: function(serie) {
        if((this.analyzer.PhaseNames.length == 1) || (serie == 'Frequency')) return [this.SerieColor(serie)];
        var Colors = [];
        Colors = ['#d9534f', '#5cb85c', '#318ce7'].slice(0, this.analyzer.PhaseNames.length);
        if((serie == 'Current') && (this.analyzer.NeutralName != null)) Colors.push('#333333');
        return Colors;
      },
      SerieUnits: function(serie) {
        if(serie == 'Voltage') return 'V';
        if(serie == 'Current') return 'A';
        if(serie == 'Active_Power') return 'W';
        if(serie == 'Active_Energy') return 'Wh';
        if(serie == 'Frequency') return 'Hz';
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
        var Result = [];
        var Name = this.SerieName(serie);
        if((serie == 'Frequency') || (this.analyzer.PhaseNames.length == 1)) return Name;
        for(var n = 0; n < this.analyzer.PhaseNames.length; n++) Result.push(Name + ' (' + this.analyzer.PhaseNames[n] + ')');
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
      CalcRange: function(range, aggreg) {
        var Now = moment().tz(this.analyzer.TimeZone);
        if(range == 'LastSamples') {
          var Period = (aggreg == '3S') ? 3000 : (aggreg == '1M') ? 60000 : (aggreg == '10M') ? 600000 : (aggreg == '15M') ? 900000 : 3600000;
          return [Now.valueOf() - Period * 2, Now.valueOf()];
        } else if(range == 'LastMinute')
          return [Now.valueOf() - 60000, Now.valueOf()];
        else if(range == 'Last5Minutes')
          return [Now.valueOf() - 300000, Now.valueOf()];
        else if(range == 'Last10Minutes')
          return [Now.valueOf() - 600000, Now.valueOf()];
        else if(range == 'Last15Minutes')
          return [Now.valueOf() - 900000, Now.valueOf()];
        else if(range == 'Last30Minutes')
          return [Now.valueOf() - 1800000, Now.valueOf()];
        else if(range == 'LastHour')
          return [Now.valueOf() - 3600000, Now.valueOf()];
        else if(range == 'LastHours')
          return [Now.valueOf() - 7200000, Now.valueOf()];
        else if(range == 'LastDay')
          return [Now.valueOf() - 86400000, Now.valueOf()];
        else if(range == 'LastWeek')
          return [Now.valueOf() - 604800000, Now.valueOf()];
        else if(range == 'LastMonth')
          return [Now.valueOf() - 2592000000, Now.valueOf()];
        else if(range == 'LastYear')
          return [Now.valueOf() - 31536000000, Now.valueOf()];
        else if(range == 'Day')
          return [Now.startOf('day').valueOf(), Now.endOf('day').valueOf()];
        else if(range == 'Week')
          return [Now.startOf('week').valueOf(), Now.endOf('week').valueOf()];
        else if(range == 'Month')
          return [Now.startOf('month').valueOf(), Now.endOf('month').valueOf()];
        else if(range == 'Year')
          return [Now.startOf('year').valueOf(), Now.endOf('year').valueOf()];
        else if(Array.isArray(range))
          return Array.from(range);
      },
      ShowDisturbances: function() {
        localStorage.setItem('Period', JSON.stringify({Period: moment().valueOf() - parseInt(localStorage.getItem('LastLogin')), To: moment().valueOf() }));
        this.$router.push({path: '/Events/Disturbances'});
      }
    },
    components: { AnalyzerTable, AnalyzerInfo }
  }
</script>

<style scoped>
  .collapse-icon {
    position: relative;
  }
  .this {
    background-color: #474544;
    color: #FAFAFA;
    width: 304px;
    height: 33px;
    display: flex;
    margin-left: 0;
    transition: margin-left 0.2s ease-in-out;
    border-bottom: 1px solid #333;
    border-top: 1px solid #333;
  }
  .this.expanded {
    margin-left: -48px;
    transition: margin-left 0.2s ease-in-out;
  }
  .icon {
    height: 24px;
    width: 24px;
    margin: 4px 12px;
  }
  .name {
    width: 192px;
    padding-left: 2px;
    line-height: 24px;
    padding-top: 4px;
    text-overflow: ellipsis;
    overflow: hidden;
    white-space: nowrap;
  }
  .btn-group {
    margin-top: 4px;
    margin-left: -1px;
  }
  .btn-group > .btn {
    background: #444;
    border-color: #333;
    padding: 6px 6px 7px 6px;
    margin-top: -5px;
    border-radius: 0px;
    margin-left: -1px;
  }
  .btn-group > .btn:hover {
    background: #555;
    border-color: #333;
  }
  .badge-icon {
    position: absolute;
    top: 17px;
    right: 8px;
  }
  .badge-menu {
    position: absolute;
    top: 16px;
    right: 12px;
    font-size: 9px;
    padding: 1px 2px;
    border-radius: 3px;
  }
  .badge-red {
    background: #c9302c;
  }
  .badge-green {
    background: #5cb85c;
  }
  .badge-yellow {
    background: #f0ad4e;
  }
</style>