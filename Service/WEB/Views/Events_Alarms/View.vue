<template>
  <q-page>
    <div class="col-lg-12">
      <div class="panel panel-other">
        <div class="panel-heading">
          <span class="header-text">Alarms</span>
          <div class="header-buttons clearfix">
            <toolbar-period storage="PeriodAlarms" v-model="Range"></toolbar-period>
          </div>
        </div>
        <div class="panel-body">
          <spinner v-if="TransferAlarms.Data==null"></spinner>
          <time-line :range.sync="Range" :alarms="TransferAlarms.Data"></time-line>
        </div>
      </div>
    </div>
    <div class="col-lg-12">
      <alarms-table :alarms="TransferAlarms.Data" @alarmschanged="UpdateAlarms"></alarms-table>
    </div>
  </q-page>
</template>

<script>
  import AlarmsTable from './Alarms.vue';
  import TimeLine from './TimeLine.vue';
  export default {
    data: function() {
      return { Range: null, TransferAlarms: this.AlarmsTransfer() };
    },
    created: function () {
      if((this.$route.query.From != null) && (this.$route.query.To != null)) {
        localStorage.setItem('PeriodAlarms', JSON.stringify([parseInt(this.$route.query.From), parseInt(this.$route.query.To)]));
        this.TransferAlarms.UpdateUnread = true;
      }
      history.pushState({}, null, this.$route.path);
    },
    beforeDestroy: function() {
      this.TransferAlarms.Destroy();
    },
    watch: {
      analyzer: function() { this.TransferAlarms.Update(this.analyzer, this.Range); },
      Range: function() { this.TransferAlarms.Update(this.analyzer, this.Range); }
    },
    methods: {
      UpdateAlarms: function() {
        this.TransferAlarms.Data = null;
        this.TransferAlarms.Update(this.analyzer, this.Range);
      }
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