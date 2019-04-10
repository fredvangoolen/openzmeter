<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="huge-small pull-left">Wifi connection</div>
    </div>
    <div class="panel-body">
      <spinner v-if="ShowSpinner"></spinner>
      <div class="row">
        <div class="col-sm-12">
          <vuetable ref="table" :fields="Columns" :api-mode="false" class="table small table-striped table-responsive">
            
          </vuetable>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
  import vuetable from 'vuetable-2';
  import Spinner from 'Components/Spinner.vue';
    export default {
    data: function() {
      return { ShowSpinner: false,
        Columns: [
          { name: 'Status', title: 'Status'}, 
          { name: 'SSID', title: 'SSID'}, 
          { name: 'BSSID', title: 'BSSID'}, 
          { name: 'Channel', title: 'Channel'}, 
          { name: 'Mode', title: 'Mode'}, 
          { name: 'Power', title: 'Power'}, 
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
      UpdateNetworks: function() {
        $.ajax({ url: "getNetworks", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({}), 
           success: function(result) {
               NetworksDataset = [];
  for(i = 0; i < result['AccessPoints'].length; i++) NetworksDataset.push( { MAC: result['AccessPoints'][i]['MAC'], Mode: result['AccessPoints'][i]['Mode'], SSID: result['AccessPoints'][i]['SSID'], Channel: (result['AccessPoints'][i]['Freq'] - 2412) / 5 + 1, Power: result['AccessPoints'][i]['Power'], Status: '' } );
  for(i = 0; i < result['Settings'].length; i++) {
    var Found = 0;
    for(n = 0; n < NetworksDataset.length; n++) {
      if((NetworksDataset[n]['SSID'] === result['Settings'][i]['SSID']) && ((NetworksDataset[n]['MAC'] === result['Settings'][i]['MAC']) || (result['Settings'][i]['MAC'] === undefined))) {
        NetworksDataset[n] = Object.assign(NetworksDataset[n], { Status: 'Configured' }, result['Settings'][i]);
        Found++;
      }
    }
    if(Found === 0) NetworksDataset.push( Object.assign({ MAC: (result['Settings'][i]['MAC'] === undefined) ? 'ANY' : result['Settings'][i]['MAC'], Mode: result['Settings'][i]['Mode'], SSID: result['Settings'][i]['SSID'], Channel: '', Power: 0, Status: 'Out of range' }, result['Settings'][i]));
  }
  for(i = 0; i < result['ActiveConnections'].length; i++) {
    for(n = 0; n < NetworksDataset.length; n++) {
      if((NetworksDataset[n]['SSID'] === result['ActiveConnections'][i]['SSID']) && (NetworksDataset[n]['MAC'] === result['ActiveConnections'][i]['MAC'])) NetworksDataset[n] = Object.assign(NetworksDataset[n], { Status: 'Connected' }, result['ActiveConnections'][i]);
    }
  }
  if(NetworksTable !== null) NetworksTable.destroy(); 
  var Contents = $('#NetworksTable tbody');
  Contents.html('');
  for(n = 0; n < NetworksDataset.length; n++) {
    var Buttons = '';
    if(NetworksDataset[n].ConfigPath !== undefined) {
      Buttons += '<button class="btn btn-xs btn-default" onclick="NetworkEdit(' + n + ')" style="width:50%"><i class="fa fa-pencil" aria-hidden="true"></i> Edit</button>';
      Buttons += '<button class="btn btn-xs btn-default" onclick="NetworkForget(' + n + ')" style="width:50%"><i class="fa fa-times" aria-hidden="true"></i> Forget</button>';
    } else {
      Buttons += '<button class="btn btn-xs btn-default" onclick="NetworkConfigure(' + n + ')" style="width:100%"><i class="fa fa-plus" aria-hidden="true"></i> Configure</button>';
    }
    Contents.append('<tr class="' + ((NetworksDataset[n].Status === 'Connected') ? 'info' : '') + '">' +
      '<td>' + NetworksDataset[n].Status + '</td>' +
      '<td>' + NetworksDataset[n].SSID + '</td>' +
      '<td>' + NetworksDataset[n].MAC + '</td>' +
      '<td>' + NetworksDataset[n].Channel + '</td>' +
      '<td>' + NetworksDataset[n].Mode + '</td>' +
      '<td data-sort="' + NetworksDataset[n].Power + '"><div class="progress" style="margin-bottom:0px">' +
      '  <div class="progress-bar" role="progressbar" style="width:' + NetworksDataset[n].Power + '%"></div>' +
      '</div></td>' +
      '<td><div class="btn-group" style="width: 100%">' + Buttons + '</div></td></tr>');
  }
  var Buttons = '<div class="btn-group">' +
                '  <button class="btn btn-sm btn-default" onclick="AjaxGetNetworks()"><i class="fa fa-refresh"></i> Refresh</button>' +
                '  <button class="btn btn-sm btn-default" onclick="NetworkAdd()"><i class="fa fa-plus-square-o" aria-hidden="true"></i> Add</button>' +
                '</div>';
  NetworksTable = $('#NetworksTable').DataTable({ responsive: false, paging: false, dom: '<"top datatable-toolbar"i><"top float-right"f><"clear">t<"clear">', language: { info: Buttons + ' _MAX_ networks found', infoEmpty: Buttons + 'No networks in range'},
                                                  columns: [ { width: '105px', orderable: false },
                                                             { orderable: false }, 
                                                             { width: '105px', orderable: false },
                                                             { width: '20px', orderable: false, className: 'dataTables_empty' },
                                                             { width: '20px', orderable: false },
                                                             { width: '40px', orderable: false },
                                                             { width: '200px', orderable: false, searchable: false }
                                                           ],
                                                  order: [5, 'desc']
  });
             
           }.bind(this),
           complete: function() {
             this.ShowSpinner = false;
           }.bind(this)
        });
      }
      
    },
    components: { vuetable, Spinner }
  }
</script>