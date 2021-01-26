<template>
  <div class="panel panel-other">
    <div class="panel-body">
      <spinner v-if="ShowSpinner"></spinner>
      <div class="col-xs-12">
        <vuetable ref="table" :data="Messages" :fields="Columns" :api-mode="false" class="table small table-striped"></vuetable>
      </div>
    </div>
  </div>
</template>

<script>
  import vuetable from 'vuetable-2';
  import moment from 'moment';
  export default {
   data: function() {
     return { ShowSpinner: true, Messages: null,
              Columns: [
                { name: 'Type', title: 'Type' },
                { name: 'Date', title: 'Date', callback: this.FormatDate },
                { name: 'Message', title: 'Message' }
              ]
            };
   },
   mounted: function() {
     this.$http.post('getSystem', {}).then(
       (result) => {
         this.Messages = [];
         for(var n = 1; n < result.data.Messages.length; n++) this.Messages.push({Type: result.data.Messages[n][0], Date: result.data.Messages[n][1], Message: result.data.Messages[n][2]});
         this.ShowSpinner = false;
       },
       (fail) => { }
     );
   },
   methods: {
     FormatDate: function(date) {
       return moment(date).format('DD/MM/YYYY - HH:mm:ss');
     }
   },
   components: { vuetable }
  }
</script>

<style scoped>

</style>