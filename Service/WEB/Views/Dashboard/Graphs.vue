<template>
  <div class="row q-col-gutter-sm q-ma-none">
    <div class="col" style="min-width: 400px">
      <graph-panel ref="g0" serie="Voltage" :data="Transfer3S.Data" :last="Transfer3S.LastData" :paused.sync="Paused3S" />
    </div>
    <div class="col" style="min-width: 400px">
      <graph-panel ref="g1" serie="Current" :data="Transfer3S.Data" :last="Transfer3S.LastData" :paused.sync="Paused3S" />
    </div>
  </div>
  <div class="row q-col-gutter-sm q-ma-none">
    <div class="col"  style="min-width: 400px">
      <graph-panel ref="g2" serie="Active_Power" :data="Transfer3S.Data" :last="Transfer3S.LastData" :paused.sync="Paused3S" />
    </div>
    <div class="col" v-if="Analyzer.NominalFrequency > 0.0"  style="min-width: 400px">
      <graph-panel ref="g3" serie="Frequency" :data="Transfer3S.Data" :last="Transfer3S.LastData" :paused.sync="Paused3S" />
    </div>
  </div>
</template>

<script>
  import GraphPanel from './GraphsPanel.vue';
  export default {
    data: function() {
      return { Paused3S: false,

                Transfer3S: this.SeriesTransfer(['Voltage', 'Current', 'Active_Power', 'Frequency']),}
    },
    components: { GraphPanel }
  }
</script>