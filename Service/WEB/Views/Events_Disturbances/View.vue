<template>
  <div class="row" v-if="analyzer!=null">
    <div class="col-lg-12">
      <div class="panel panel-other">
        <div class="panel-heading">
          <span class="header-text">Disturbances</span>
          <div class="header-buttons clearfix">
            <toolbar-period storage="PeriodEvents" v-model="Range" :analyzer="analyzer"></toolbar-period>
          </div>
        </div>
        <div class="panel-body">
          <spinner v-if="TransferEvents.Data==null"></spinner>
          <time-line :analyzer="analyzer" :range.sync="Range" :events="TransferEvents.Data"></time-line>
        </div>
      </div>
    </div>
    <div class="col-lg-12">
      <events-table :analyzer="analyzer" :events="TransferEvents.Data" @eventschanged="UpdateEvents"></events-table>
    </div>
  </div>
  <div v-else-if="analyzer==null">
    <div class="alert alert-danger text-center">
      <p>No selected analyzer</p>
    </div>
  </div>
</template>

<script>
  import TimeLine from './TimeLine.vue';
  import EventsTable from './Events.vue';
  export default {
    props: { analyzer: Object },
    data: function() {
      return { Range: null, TransferEvents: this.EventsTransfer(this) };
    },
    created: function() {
      this.TransferEvents.SingleShot = true;
      if((this.$route.query.From != null) && (this.$route.query.To != null)) {
        localStorage.setItem('PeriodEvents', JSON.stringify([parseInt(this.$route.query.From), parseInt(this.$route.query.To)]));
        this.TransferEvents.UpdateUnread = true;
      }
      history.pushState({}, null, this.$route.path);
    },
    beforeDestroy: function() {
      this.TransferEvents.Destroy();
    },
    watch: {
      analyzer: function() { this.TransferEvents.Update(this.analyzer, this.Range); },
      Range: function() { this.TransferEvents.Update(this.analyzer, this.Range); }
    },
    methods: {
      UpdateEvents: function() {
        this.TransferEvents.Data = null;
        this.TransferEvents.Update(this.analyzer, this.Range);
      }
    },
    components: { EventsTable, TimeLine}
  }
</script>

<style scoped>
  .header-text {
    font-size: 24px;
    line-height: 16px;
  }
</style>