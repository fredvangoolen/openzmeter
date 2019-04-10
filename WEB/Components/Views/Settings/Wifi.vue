<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="huge-small pull-left">Wifi</div>
      <div class="btn-group pull-right">
        <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Add network" @click="Add"><img :src="require('Imgs/plus.png')">Add</button>
        <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Refresh" @click="UpdateNetworks"><img :src="require('Imgs/arrow_rotate_clockwise.png')">Refresh</button>  
      </div>
    </div>
    <div class="panel-body">
      <spinner v-if="ShowSpinner"></spinner>
      <div class="row">
        <div class="col-sm-12">
          <vuetable ref="table" :fields="Columns" :api-mode="false" class="table small table-striped table-responsive">
            <template slot="actions" slot-scope="props">
              <div class="btn-group" style="width: 100%">
                <button class="btn btn-xs btn-default" v-if="(props.rowData.ConfigPath !== undefined)" v-on:click="Edit(props.rowData)" style="width:50%">Edit</button>
                <button class="btn btn-xs btn-danger" v-if="(props.rowData.ConfigPath !== undefined)" v-on:click="Forget(props.rowData)" style="width:50%">Forget</button>
                <button class="btn btn-xs btn-default" v-if="(props.rowData.ConfigPath === undefined)" v-on:click="Configure(props.rowData)" style="width:100%">Configure</button>
              </div>
            </template>
            <template slot="strength" slot-scope="props">
              <progress-bar :value="props.rowData.Power"></progress-bar>
            </template>
          </vuetable>
        </div>
      </div>
    </div>
    <wifi-dialog ref="dialog"></wifi-dialog>
  </div>
</template>

<script>
  import vuetable from 'vuetable-2';
  import Spinner from 'Components/Spinner.vue';
  import ProgressBar from 'Components/ProgressBar.vue';
  import WifiDialog from './WiFiEdit.vue';
  export default {
    data: function() {
      return { ShowSpinner: true,
        Columns: [
          { name: 'Status', title: 'Status'}, 
          { name: 'SSID', title: 'SSID'}, 
          { name: 'MAC', title: 'BSSID'}, 
          { name: 'Channel', title: 'Channel'}, 
          { name: 'Mode', title: 'Mode'}, 
          { name: '__slot:strength', title: 'Power'}, 
          { name: '__slot:actions', title: ''}
        ]
      };
    },
    created: function() {
      this.Networks = [];
    },
    mounted: function(){
      this.UpdateNetworks();
    },
    methods: {
      Add: function() {
        this.$refs.dialog.Show(null); 
      },
      Edit: function() {
        
      },
      Forget: function() {
        
      },
      Configure: function() {
        
      },      
      UpdateNetworks: function() {
        $.ajax({ url: "getNetworks", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({}), 
          success: function(result) {
            this.Networks = [];
            for(var i = 0; i < result['AccessPoints'].length; i++) this.Networks.push( { MAC: result['AccessPoints'][i]['MAC'], Mode: result['AccessPoints'][i]['Mode'], SSID: result['AccessPoints'][i]['SSID'], Channel: (result['AccessPoints'][i]['Freq'] - 2412) / 5 + 1, Power: result['AccessPoints'][i]['Power'], Status: '' } );
            for(var i = 0; i < result['Settings'].length; i++) {
              var Found = 0;
              for(var n = 0; n < this.Networks.length; n++) {
                if((this.Networks[n]['SSID'] === result['Settings'][i]['SSID']) && ((this.Networks[n]['MAC'] === result['Settings'][i]['MAC']) || (result['Settings'][i]['MAC'] === undefined))) {
                  this.Networks[n] = Object.assign(this.NetworksDataset[n], { Status: 'Configured' }, result['Settings'][i]);
                  Found++;
                }
              }
              if(Found === 0) this.Networks.push( Object.assign({ MAC: (result['Settings'][i]['MAC'] === undefined) ? 'ANY' : result['Settings'][i]['MAC'], Mode: result['Settings'][i]['Mode'], SSID: result['Settings'][i]['SSID'], Channel: '', Power: 0, Status: 'Out of range' }, result['Settings'][i]));
            }
            for(var i = 0; i < result['ActiveConnections'].length; i++) {
              for(var n = 0; n < this.Networks.length; n++) {
                if((this.Networks[n]['SSID'] === result['ActiveConnections'][i]['SSID']) && (this.Networks[n]['MAC'] === result['ActiveConnections'][i]['MAC'])) this.Networks[n] = Object.assign(this.Networks[n], { Status: 'Connected' }, result['ActiveConnections'][i]);
              }
            }
            this.$refs.table.setData( { data: this.Networks } );
            this.ShowSpinner = false;

return;
    for(n = 0; n < NetworksDataset.length; n++) {
    var Buttons = '';

      Contents.append('<tr class="' + ((NetworksDataset[n].Status === 'Connected') ? 'info' : '') + '">' +
      '<td data-sort="' + NetworksDataset[n].Power + '"><div class="progress" style="margin-bottom:0px">' +
      '  <div class="progress-bar" role="progressbar" style="width:' + NetworksDataset[n].Power + '%"></div>' +
      '</div></td>' +
      '<td><div class="btn-group" style="width: 100%">' + Buttons + '</div></td></tr>');
  }
             
           }.bind(this),
           complete: function() {
             this.ShowSpinner = false;
           }.bind(this)
        });
      }
      
    },
    components: { vuetable, Spinner, ProgressBar, WifiDialog }
  }
</script>

<style scoped>
  .table >>> th:nth-child(1), .table >>> th:nth-child(3) {
    text-align: center;
    width: 105px;
  }
  .table >>> td:nth-child(1), .table >>> td:nth-child(3) {
    text-align: right;
  }
  .table >>> th:nth-child(4), .table >>> th:nth-child(5) {
    text-align: center;
    width: 20px;
  }
  .table >>> th:nth-child(6) {
    width: 80px;
    text-align: center;
  }
  .table >>> th:nth-child(7) {
    width: 120px;
  }
  .table >>> .progress {
    height: 14px;
    background-color: #474544;
    margin-bottom: 2px;
  }
</style>