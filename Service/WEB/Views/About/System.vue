<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">{{ (LastUpdate==null) ? 'Loading data...' : 'Last update at ' + FormatDate(LastUpdate) }}</div>
      <div class="header-buttons">
        <toolbar-options storage="SystemOptions" :options="['Debug', 'Warning', 'Error', 'Info']" v-model="Options"></toolbar-options>
        <toolbar-pager :pageSize="20" :input="Messages" :output.sync="FilterMessages" :filtercols="['Message', 'Date']"></toolbar-pager>
        <div class="btn-group">
          <button type="button" class="btn btn-default btn-xs" title="Update" @click="Update"><img :src="require('Imgs/update.png')"></button>
        </div>
      </div>
    </div>
    <div class="panel-body">
      <spinner v-if="ShowSpinner"></spinner>
      <div class="col-xs-12">
        <vuetable ref="table" :data="FilterMessages" :fields="Columns" :api-mode="false" class="table small table-striped"></vuetable>
      </div>
    </div>
  </div>
</template>

<script>
  import vuetable from 'vuetable-2';
  import moment from 'moment';
  export default {
    data: function() {
      return { ShowSpinner: true, AllMessages: [], Messages: [], FilterMessages: [], Options: {}, LastUpdate: null,
              Columns: [
                { name: 'Type', title: 'Type', callback: this.FormatType },
                { name: 'Date', title: 'Date', callback: this.FormatDate },
                { name: 'Message', title: 'Message' }
              ]
            };
    },
    created: function() {
      this.Update();
    },
    watch: {
      AllMessages: function() { this.Filter(); },
      Options: function() { this.Filter(); }
    },
    methods: {
      FormatDate: function(date) {
        return moment(date).format('DD/MM/YYYY - HH:mm:ss');
      },
      FormatType: function(val) {
        if(val == 'WARNING') return '<span style="color: #f0ad4e">' + val + '</span>';
        if(val == 'INFO') return '<span style="color: #318ce7">' + val + '</span>';
        if(val == 'ERROR') return '<span style="color: #d9534f">' + val + '</span>';
        if(val == 'DEBUG') return '<span style="color: #dddddd">' + val + '</span>';
      },
      Filter: function() {
        this.Messages = [];
        for(var n = 0; n < this.AllMessages.length; n++) {
          if((this.AllMessages[n].Type == 'DEBUG') && (this.Options.Debug == false)) continue;
          if((this.AllMessages[n].Type == 'WARNING') && (this.Options.Warning == false)) continue;
          if((this.AllMessages[n].Type == 'INFO') && (this.Options.Info == false)) continue;
          if((this.AllMessages[n].Type == 'ERROR') && (this.Options.Error == false)) continue;
          this.Messages.push(this.AllMessages[n]);
        }
      },
      Update: function() {
        this.$http.post('getSystem', {}).then(
          (result) => {
            this.AllMessages = [];
            this.LastUpdate = moment();
            for(var n = 1; n < result.data.Messages.length; n++) this.AllMessages.push({Type: result.data.Messages[n][0], Date: result.data.Messages[n][1], Message: result.data.Messages[n][2]});
            this.ShowSpinner = false;
          },
          (fail) => { }
        );
      }
    },
    components: { vuetable }
  }
</script>

<style scoped>
  .header-text {
    font-size: 14px;
  }
  .table >>> th:nth-child(1) {
    text-align: center;
    width: 72px;
  }
  .table >>> td:nth-child(1) {
    text-align: center;
  }
  .table >>> th:nth-child(2) {
    text-align: center;
    width: 160px;
  }
  .table >>> td:nth-child(2) {
    text-align: center;
  }
</style>