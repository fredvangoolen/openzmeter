<template>
  <div class="row q-gutter-x-xs" v-if="Analyzer.Analyzer != null" @click="Grouped = !Grouped">
    <topbar-indicator serie="Voltage" :grouped="Grouped" />
    <topbar-indicator serie="Current" :grouped="Grouped" />
    <topbar-indicator serie="Active_Power" :grouped="Grouped" />
    <topbar-indicator serie="Frequency" v-if="Analyzer.NominalFrequency != 0.0" />
    <topbar-indicator serie="Active_Energy" :grouped="Grouped" />
  </div>
</template>

<script>
  import TopbarIndicator from './TopbarIndicator.vue';
  export default {
    data () {
      return { Grouped: false,
               //Transfer2: this.SeriesTransfer(['Active_Energy'])
      };
    },
//    created: function() {
//      this.Transfer2.Update(this.Analyzer, '15M', 'Day');
//    },
//    beforeDestroy: function() {
//      this.Transfer2.Destroy();
//    },
//    watch: {
//      analyzer: function() {
//        this.$nextTick(() => {
//          this.Transfer1.Update(this.Analyzer);
//          this.Transfer2.Update(this.Analyzer, '15M', 'Day');
//        });
//      }
//    },
//    computed: {
//      EnergyUsage: function() {
//        var Energy = new Array(this.Analyzer.PhaseNames.length).fill(0);
//        if(this.Transfer2.Data == null) return { Active_Energy: Energy };
//        var EnergyCol = this.Transfer2.Data.Headers.indexOf('Active_Energy');
//        for(var n = 0; n < this.Transfer2.Data.Rows.length; n++) {
//          for(var p = 0; p < this.Transfer2.Data.Rows[n][EnergyCol].length; p++) Energy[p] = Energy[p] + this.Transfer2.Data.Rows[n][EnergyCol][p];
//        }
//        return { Active_Energy: Energy };
//      }
//    },
    components: { TopbarIndicator }
  }
</script>