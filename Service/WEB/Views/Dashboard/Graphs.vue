<template>
  <div class="row">
    <div class="col-lg-6 col-md-12">
      <graphs ref="g0" serie="Voltage" :data="Transfer3S.Data" :last="Transfer3S.LastData" :paused.sync="Paused3S"></graphs>
    </div>
      <div class="col-lg-6 col-md-12">
        <!--graphs ref="g1" serie="Current" :data="Transfer3S.Data" :last="Transfer3S.LastData" :paused.sync="Paused3S"></graphs-->
      </div>
      <div :class="{'col-lg-6': (Analyzer.NominalFrequency > 0.0)}" class="col-md-12">
        <!--graphs ref="g2" serie="Active_Power" :data="Transfer3S.Data" :last="Transfer3S.LastData" :paused.sync="Paused3S"></graphs-->
      </div>
      <div class="col-lg-6 col-md-12" v-if="Analyzer.NominalFrequency > 0.0">
        <!--graphs ref="g3" serie="Frequency" :data="Transfer3S.Data" :last="Transfer3S.LastData" :paused.sync="Paused3S"></graphs-->
      </div>
    </div>
</template>

<script>
export default {
  data: function() {
    return {Paused3S: false,

               Transfer3S: this.SeriesTransfer(['Voltage', 'Current', 'Active_Power', 'Frequency']),}
  }
}
</script>