<template>
  

        <div class="modal-content hide" id="divTariffSeasons">
          <div class="modal-header">
            <div class="pull-left"><h4 class="modal-title">Tariffs template &raquo; Seasons</h4></div>
            <div class="pull-right btn-group">
              <button type="button" class="btn btn-default btn-xs" id="btnTariffSeasonsAdd"><img src="Imgs/plus.png" alt="New season"></button>
              <button type="button" class="btn btn-default btn-xs" id="btnTariffSeasonsBack"><img src="Imgs/arrow_undo.png" alt="Go back"></button>
            </div>
          </div>
          <div class="modal-body">
            <div class="alert alert-info text-center" style="margin-bottom: 5px; padding: 5px;">Define seasons of the year</div>
            <div class="row">
              <div class="col-xs-12">
                <table id="SeasonsTable" class="table small table-striped table-responsive" style="width: 100%">
                  <thead><tr><th>Name</th><th style="width:80px">From</th><th style="width:80px">To</th><th style="width:140px"></th></tr></thead>
                  <tbody></tbody>
                </table>
              </div>
            </div>
          </div>
        </div>




</template>

<script>
  //  if(SeasonsTable === null) SeasonsTable = $('#SeasonsTable').DataTable({ responsive: false, paging: false, scrollCollapse: true, scrollY: '224px', dom: '<"clear">t<"clear">', columnDefs: [ { targets: [0, 1, 2], orderable: true }, { targets: [1, 2], className: 'text-center' }, { targets: [3], orderable: false } ], order: [1, 'asc']});

  </script>