<template>
  <div v-if="analyzer != null">
    <div class="row">
      <div class="col-lg-12">
        <div class="panel panel-other">
          <div class="panel-heading">
            <div class="header-text">200ms parameters</div>
            <div class="header-buttons clearfix">
 aqui va el selector de series (Series sin usar, al hacer click se añade y se quita de la lista)
 y rangos (1M, 5MIN, 10MIN, 15MIN, 30MIN, 1HOUR)
            </div>
          </div>
          <div class="panel-body">
            <spinner v-if="TransferGraph.Data==null"></spinner>
            <div class="row">
              <div class="col-lg-12">
                <graphs-panel :analyzer="analyzer" :data="TransferGraph.Data" serie="CURRENT" :paused="false"></graphs-panel>
              </div>
              <div class="col-lg-12">
                <graphs-panel :analyzer="analyzer" :data="TransferGraph.Data" serie="VOLTAGE" :paused="false"></graphs-panel>
              </div>
              <div class="col-lg-12">
                <graphs-panel :analyzer="analyzer" :data="TransferGraph.Data" serie="ACTIVE_POWER" :paused="false"></graphs-panel>
              </div>
            </div>
          </div>
        </div>
      </div>
      <div class="col-lg-12">
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
  import GraphsPanel from './GraphsPanel.vue';
  export default {
    props: { analyzer: { type: Object } },
    data: function() {
      return { Height: 300, Paused: false, Expanded: false,
               TransferGraph: this.CreateSeriesTransfer(this, ['CURRENT', 'VOLTAGE', 'ACTIVE_POWER'])
      };
    },
    created: function() {
      //this.TransferGraph.AggregFail = () => { this.Aggreg = null };
      this.TransferGraph.Update(this.analyzer, '200MS', 'Last5Minutes');
    },
    beforeDestroy: function() {
      this.TransferGraph.Destroy();
    },
    watch: {
      analyzer: function(val) {
        this.TransferGraph.Update(this.analyzer, '200MS', 'Last5Minutes');
      },
//      Aggreg: function() {
//        this.TransferGraph.Update(this.analyzer, this.Aggreg, this.Range);
//      },
//      Range: function(val, prev) {
//        if(prev != null) this.TransferGraph.Update(this.analyzer, this.Aggreg, this.Range);
//      },
//      Paused: function() {
//        if(this.Paused == true) {
//          this.TransferNow.Destroy();
//        } else {
//          this.TransferNow.UpdateNow(this.analyzer);
//        }
//      }
    },
    methods: {
      Export: function() {
        if(this.TransferGraph.Data == null) return [];
        var Rows = [['Sampletime'].concat(this.analyzer.GetSeriesNames('Current', this.analyzer.CurrentChannels))];
        for(var n in this.TransferGraph.Data.Sampletime) {
          var Row = [this.TransferGraph.Data.Sampletime[n]];
          for(var i in this.TransferGraph.Data.Current[n]) Row.push(this.Round(this.TransferGraph.Data.Current[n][i], 3));
          Rows.push(Row);
        }
        return Object.seal(Rows);
      }
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