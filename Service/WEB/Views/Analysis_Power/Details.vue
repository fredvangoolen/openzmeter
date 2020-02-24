<script>
  export default {
    data () {
      return { ActiveTab: '' };
    },
    created: function() {
      this.Ajax = null;
      this.LastSample = 0;
      this.UpdateGraphs1MTimer = null;
      this.UpdateGraphsNowTimer = null;
      this.$root.$on('AnalyzerChanged', this.SetAnalyzer);
    },
//    mounted: function() {
//      if(this.$root.SelectedAnalyzer !== null) {
//        this.UpdateGraphs1M();
//        this.UpdateGraphsNow();
//      }
//    },
//    beforeDestroy: function() {
//      if(this.UpdateGraphs1MTimer !== null) clearTimeout(this.UpdateGraphs1MTimer);
//      if(this.UpdateGraphsNowTimer !== null) clearTimeout(this.UpdateGraphsNowTimer);
//    },
    watch: {
//      ActiveTab: function(val) {
//        if(val === 'Waveform') this.$refs.waveform.Redraw();
//        if(val === 'Harmonics') this.$refs.harmonics.Redraw();
//        if(val === 'Evolution') this.$refs.evolution.Redraw();
//        if(val === 'Distribution') this.$refs.distribution.Redraw();
//        if(val === 'Parameters') this.$refs.parameters.Redraw();
//      }
    },
    methods: {
      PlayStatusChanged: function(stat) {
        if(stat === true) {
          this.Ajax.abort();
          clearInterval(this.UpdateGraphsNowTimer);
        } else {
          this.UpdateGraphsNow();
        }
        this.$refs.waveform.$refs.tools.SetPlayStatus(stat);
      },
      PhasesChangedSingle: function(phase) {
        this.$refs.parameters.SetPhases(phase);
        this.$refs.evolution.SetPhases(phase);
      },
      PhasesChangedMulti: function(phases) {
        this.$refs.waveform.SetPhases(phases);
        this.$refs.harmonics.SetPhases(phases);
      },
//      SetAnalyzer() {
//        this.LastSample = 0;
//        this.UpdateGraphs1M();
//        this.UpdateGraphsNow();
//      },
      UpdateGraphs1M() {
        if(this.$root.SelectedAnalyzer === null) return;
        this.Ajax = $.ajax({ url: 'getSeries', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Aggreg: '1M', Period: 86400000, To: 0, LastSample: this.LastSample, Series: ['POWER_HARMONICS', 'ACTIVE_POWER', 'REACTIVE_POWER', 'VOLTAGE', 'CURRENT', 'POWER_FACTOR', 'PHI'], Analyzer: this.$root.SelectedAnalyzer.Analyzer }),
          success: function(result) {
            result['Apparent_Power'] = [];
            for(var n = 0; n < result['Sampletime'].length; n++) result['Apparent_Power'].push(result['Voltage'][n].map( function(val, index) { return val * result['Current'][n][index]; }));
            this.$refs.evolution.SetData(result);
            this.$refs.parameters.SetData(result);
            this.LastSample = oZm.LastSampletime(result, this.LastSample);
          }.bind(this),
          complete: function() {
            if(this.UpdateGraphs1MTimer !== null) clearTimeout(this.UpdateGraphs1MTimer);
            this.UpdateGraphs1MTimer = setTimeout(this.UpdateGraphs1M, 30000);
          }.bind(this)
        });
      },

    },
//    components: { Distribution, Waveform, Harmonics, Evolution, Parameters }
  }
</script>

<style scoped>
  .panel-other {
    border-top-left-radius: 0px;
    border-top-right-radius: 0px;
  }
</style>