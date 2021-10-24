<template>
  <q-page v-if="analyzer != null">
    <div class="row">
      <div class="col-lg-12">
        <div class="panel panel-other">
          <div class="panel-heading">
            <div class="header-text">200ms parameters</div>
            <div class="header-buttons clearfix">
              <toolbar-period-b storage="HiResPeriod" v-model="Range" :analyzer="analyzer"></toolbar-period-b>
              <option-adder v-if="AvaliableSeries.length>0" :options="AvaliableSeries" @add="AddSerie"></option-adder>
            </div>
          </div>
          <div class="panel-body">
            <div class="row">
              <div class="col-lg-12">
                <div class="alert alert-danger text-center" v-if="Series.length==0">
                  <p>No selected series to show</p>
                </div>
              </div>
              <div class="col-lg-12" v-for="(serie, idx) in Series" :key="serie">
                <graphs-panel :analyzer="analyzer" :serie="serie" :ref="'graph' + idx" :range="Range" :paused.sync="Paused" @remove="RemoveSerie"></graphs-panel>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  </q-page>
  <q-page v-else-if="analyzer == null">
    <div class="alert alert-danger text-center">
      <p>No selected analyzer</p>
    </div>
  </q-page>
</template>

<script>
  import GraphsPanel from './GraphsPanel.vue';
  export default {
    data: function() {
      return { Paused: false, Range: null, AvaliableSeries:[], Series: [] };
    },
    created: function() {
      var AllSeries = ['Voltage', 'Current', 'Active_Power', 'Frequency'].sort();
      var Stored = JSON.parse(localStorage.getItem('200MS_Series'));
      if(Array.isArray(Stored) == false) Stored = [];
      for(var n in AllSeries) {
        if(Stored.indexOf(AllSeries[n]) != -1) {
          this.Series.push(AllSeries[n]);
        } else {
          this.AvaliableSeries.push(AllSeries[n]);
        }
      }
    },
    methods: {
      AddSerie: function(serie) {
        this.AvaliableSeries.splice(this.AvaliableSeries.indexOf(serie), 1);
        this.Series.push(serie);
        localStorage.setItem('200MS_Series', JSON.stringify(this.Series));
      },
      RemoveSerie: function(serie) {
        this.Series.splice(this.Series.indexOf(serie), 1);
        this.AvaliableSeries = this.AvaliableSeries.push(serie).sort();
        localStorage.setItem('200MS_Series', JSON.stringify(this.Series));
      },
    },
    components: { GraphsPanel }
  }
</script>

<style scoped>
  .header-text {
    font-size: 24px;
    line-height: 16px;
  }
</style>