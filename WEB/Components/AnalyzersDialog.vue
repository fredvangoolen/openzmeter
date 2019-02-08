<template>
  <div class="modal" tabindex="1" role="dialog" data-backdrop='static'>
    <div class="modal-dialog">
      <div class="modal-content">
        <div class="modal-header">
          <div class="pull-left"><h4 class="modal-title">Analyzers</h4></div>
          <div class="pull-right">
            <button type="button" class="btn btn-default btn-xs" v-on:click="Close"><img src="Imgs/arrow_undo.png"></button>
          </div>
        </div>
        <div class="modal-body">
          <table class="table small table-striped table-responsive">
            <thead><tr><th style="width:20px">Status</th><th>Name</th><th style="width:60px">Type</th><th style="width:80px"></th></tr></thead>
            <tbody>
            </tbody>
          </table>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
module.exports = {
  props: ['interval'],
  data: function() {
          return {
            Analyzers: [],
            TableAnalyzers: null
          }
        },
  methods: {
   /* AddAnalyzers = function(result) {
      
      for(var i = 0; i < Self.AvaliableAnalyzers.length; i++) {
        var RowText = '<tr' + ((i === Self.CurrentAnalyzerIndex) ? ' class="info"' : '') + '>' +
                      '<td>' + ((result[i].Online === true) ? 'ONLINE' : 'OFFLINE') + '</td>' +
                      '<td>' + result[i].Name + '</td>' +
                      '<td>' + result[i].Mode + '</td>' +
                      '<td><div class="btn-group" data-analyzer="' + result[i].ID + '">' +
                      '<button type="button" class="btn btn-success btn-xs' + ((i === Self.CurrentAnalyzerIndex) ? ' disabled' : '') + '">Select</button><button type="button" class="btn btn-default dropdown-toggle btn-xs" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false"><span class="caret"></span></button>' +
                      '<ul class="dropdown-menu pull-right"><li><a href="#">Configure</a></li><li><a href="#">ACL</a></li><li><a href="#">Tariff</a></li><li><a href="#">Alerts</a></li><li role="separator" class="divider"></li><li><a href="#">Maintenance</a></li></ul>' +
                      '</div></td>' +
                      '</tr>';
        Self.TableAnalyzers.row.add($(RowText)).draw();
      }
      if(Self.AvaliableAnalyzers[Self.CurrentAnalyzerIndex] === undefined) {
        if(Self.AvaliableAnalyzers.length !== 0) $('#dlgAnalyzers').modal('show');
        $('#NodeTime').text('Not connected!');
      } else {
        $('#labelAnalyzerName').text(Self.AvaliableAnalyzers[Self.CurrentAnalyzerIndex].Name);
        $('#AnalyzerMenu').removeClass('hide');
        $('#AnalyzerNavHeader').removeClass('hide');
        $('.indicator-bar').removeClass('hide');
        Self.GetIndicators();
        Self.GetIndicatorsB();
        Self.GetSysStats();
      }
    },*/
    Show: function(result) {
      if(result.length === 0) return;
      this.Analyzers = result;
      if(this.TableAnalyzers === null) this.TableAnalyzers = $(this.$el).find('table').DataTable({ responsive: false, paging: false, scrollCollapse: true, dom: '<"top float-left"i><"top float-right"f><"clear">t<"clear">p', language: { info: "_MAX_ avaliable devices", infoEmpty: "No avaliable devices" }, columnDefs: [ { targets: [0], orderable: false, searchable: false }, { targets: [3], orderable: false, searchable: false }, { targets: [0, 2], className: 'text-center' } ], order: [1, 'desc'] });
      for(var i = 0; i < result.length; i++) {
        var RowText = '<tr' + ((i === 0) ? ' class="info"' : '') + '>' +
                      '<td>' + ((result[i].Online === true) ? 'ONLINE' : 'OFFLINE') + '</td>' +
                      '<td>' + result[i].Name + '</td>' +
                      '<td>' + result[i].Mode + '</td>' +
                      '<td><div class="btn-group" data-analyzer="' + result[i].ID + '">' +
                      '<button type="button" class="btn btn-success btn-xs' + ((i === 0) ? ' disabled' : '') + '">Select</button><button type="button" class="btn btn-default dropdown-toggle btn-xs" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false"><span class="caret"></span></button>' +
                      '<ul class="dropdown-menu pull-right"><li><a href="#">Configure</a></li><li><a href="#">ACL</a></li><li><a href="#">Tariff</a></li><li><a href="#">Alerts</a></li><li role="separator" class="divider"></li><li><a href="#">Maintenance</a></li></ul>' +
                      '</div></td>' +
                      '</tr>';
        this.TableAnalyzers.row.add($(RowText)).draw();
      }
    
    
    $(this.$el).modal('show');
    },
    Close: function() {
      $(this.$el).modal('hide');
    }
  }
}
//# sourceURL=Components/AnalyzerDialog.vue
</script>
 
<style>
  table {
    width: 100%;
  }
</style>