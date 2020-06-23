<template>
  <div v-if="analyzer!=null">
    <div class="col-lg-12">
      <div class="panel panel-other">
        <div class="panel-heading">
          <span class="header-text">Alarms</span>
          <div class="header-buttons clearfix">
            <toolbar-period storage="PeriodAlarms" v-model="Range" :analyzer="analyzer"></toolbar-period>
          </div>
        </div>
        <div class="panel-body">
          <spinner v-if="TransferAlarms.Data==null"></spinner>
          <time-line :analyzer="Analyzer" :range.sync="Range" :alarms="TransferAlarms.Data"></time-line>
        </div>
      </div>
    </div>
    <div class="col-lg-12">
      <alarms-table :analyzer="analyzer" :alarms="TransferAlarms.Data"></alarms-table>
    </div>
  </div>
  <div v-else-if="analyzer==null">
    <div class="alert alert-danger text-center">
      <p>No selected analyzer</p>
    </div>
  </div>
</template>

<script>
  import AlarmsTable from './Alarms.vue';
  import TimeLine from './TimeLine.vue';
  export default {
    props: { analyzer: Object },
    data: function() {
      return { Analyzer: null, Range: null, Alarms: null, TransferAlarms: this.CreateSeriesTransfer(this) };
    },
    created: function () {
      if((this.$route.query.From != null) && (this.$route.query.To != null)) {
        localStorage.setItem('PeriodAlarms', JSON.stringify([this.$route.query.From, this.$route.query.To]));
        this.TransferAlarms.UpdateUnread = true;
      }
      history.pushState({}, null, this.$route.path);
      this.TransferAlarms.SingleShot = true;
      this.TransferAlarms.UpdateAlarms(this.analyzer, this.Range);
      if(this.analyzer != null) {
        this.$http.post('getAnalyzer', { Analyzer: this.analyzer.Analyzer , Alarms: true }).then(
          (result) => { this.Analyzer = result.body; },
          (fail) => {}
        );
      }
    },
    beforeDestroy: function() {
      this.TransferAlarms.Destroy();
    },
    watch: {
      analyzer: function() {
        this.Analyzer = null;
        if(this.analyzer != null) {
          this.$http.post('getAnalyzer', { Analyzer: this.analyzer.Analyzer , Alarms: true }).then(
            (result) => { this.Analyzer = result.body; },
            (fail) => {}
          );
        }
        this.TransferAlarms.UpdateAlarms(this.analyzer, this.Range);
      },
      Range: function() { this.TransferAlarms.UpdateAlarms(this.analyzer, this.Range); }
    },
    components: { AlarmsTable, TimeLine }
  }
</script>

<style scoped>
  .header-text {
    font-size: 24px;
    line-height: 16px;
  }
</style>