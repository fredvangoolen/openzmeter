<template>
  <div class="modal-content">
    <div class="modal-header">
      <div class="pull-left"><h4 class="modal-title">Analyzers</h4></div>
      <div class="pull-right">
        <button type="button" class="btn btn-default btn-xs" @click="$emit('close')"><img src="Imgs/arrow_undo.png"></button>
      </div>
    </div>
    <div class="modal-body">
      <table ref="table" class="table small table-striped table-responsive" style="width: 100%">
        <thead><tr><th style="width:20px">Status</th><th>Name</th><th style="width:60px">Type</th><th style="width:112px"></th></tr></thead>
        <tbody>
        </tbody>
      </table>
    </div>
  </div>
</template>

<script>
//  import Datatable from 'vue2-datatable-component';
  import DataTables from 'JS/dataTables.min.js';
  export default {
    props: ['params'],
    data() {
      return { Table: null
      };
  },
    methods: {
      SetData: function(data) {
        var Current = localStorage.getItem("SelAnalyzer");
        this.Table.clear();
        for(var i = 0; i < data.length; i++) {
          var RowText = '<tr' + ((Current === data[0].ID) ? ' class="info"' : '') + '>' +
                        '  <td>' + ((data[i].Online === true) ? 'ONLINE' : 'OFFLINE') + '</td>' +
                        '  <td>' + data[i].Name + '</td>' +
                        '  <td>' + data[i].Mode + '</td>' +
                        '  <td><div class="btn-group" data-analyzer="' + data[i].ID + '">' +
                        '    <button type="button" class="btn btn-success btn-xs' + ((Current === data[0].ID) ? ' disabled' : '') + '" data-action="Select">Select</button><button type="button" class="btn btn-default dropdown-toggle btn-xs" data-toggle="dropdown"><span class="caret"></span></button>' +
                        '    <ul class="dropdown-menu pull-right">' +
                        '      <li><a href="#" data-action="Configure">Configure</a></li>' +
                        '      <li><a href="#" data-action="ACL">ACL</a></li>' +
                        '      <li><a href="#" data-action="Tariffs"]>Tariff</a></li>' +
                        '      <li><a href="#" data-action="Alerts">Alerts</a></li>' +
                        '      <li role="separator" class="divider"></li>' +
                        '      <li><a href="#" data-action="Maintenance">Maintenance</a></li>' +
                        '    </ul></div>' +
                        '  </td>' +
                        '</tr>';
          this.Table.row.add($(RowText)).draw();
        }
        this.Table.columns.adjust();
      },
      SelectClick: function(evt) {
        alert("bliu");
      },
      ConfigureClick: function(evt) {
        alert("Configure not yet implemented");
      },
      ACLClick: function(evt) {
        alert("ACL not yet implemented");
      },
      TariffsClick: function(evt) {
        this.$parent.show('AnalyzerTariffs', null);
      },
      AlertsClick: function(evt) {
        alert("Alerts not yet implemented");
      },
      MaintenanceClick: function(evt) {
        alert("Maintenance not yet implemented");
      }
    },
    mounted: function() {
      this.Table = $(this.$refs.table).DataTable({ responsive: false, paging: false, scrollCollapse: true, dom: '<"top float-left"i><"top float-right"f><"clear">t<"clear">p', language: { info: "_MAX_ avaliable devices", infoEmpty: "No avaliable devices" }, columnDefs: [ { targets: [0], orderable: false, searchable: false }, { targets: [3], orderable: false, searchable: false }, { targets: [0, 2], className: 'text-center' } ], order: [1, 'desc'] });
      this.SetData(this.params);
      $(this.$refs.table).on('click', '[data-action="Select"]',      this.SelectClick);
      $(this.$refs.table).on('click', '[data-action="Configure"]',   this.ConfigureClick);  
      $(this.$refs.table).on('click', '[data-action="ACL"]',         this.ACLClick);
      $(this.$refs.table).on('click', '[data-action="Tariffs"]',     this.TariffsClick);
      $(this.$refs.table).on('click', '[data-action="Alerts"]',      this.AlertsClick);
      $(this.$refs.table).on('click', '[data-action="Maintenance"]', this.MaintenanceClick);
    },
    components: {  }
  }
</script>

<style>

</style>
