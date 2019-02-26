<template>
          <div class="modal-content hide" id="divTariffHolidays">
          <div class="modal-header">
            <div class="pull-left"><h4 class="modal-title">Tariffs template &raquo; Holidays</h4></div>
            <div class="pull-right btn-group">
              <button type="button" class="btn btn-default btn-xs" id="btnTariffHolidaysAdd"><img src="Imgs/plus.png" alt="New holiday"></button>
              <button type="button" class="btn btn-default btn-xs" id="btnTariffHolidaysBack"><img src="Imgs/arrow_undo.png" alt="Go back"></button>
            </div>
          </div>
          <div class="modal-body">
            <div class="row">
              <div class="col-xs-12">
                <table class="table small table-striped table-responsive">
                  <thead><tr><th style="width: 40px">Driver</th><th>Description</th><th style="width: 250px">Serial</th><th style="width: 20px">Analyzers</th><th style="width: 90px">Status</th><th style="width: 180px"></th></tr></thead>
                  <tbody></tbody>
                </table>
              </div>
            </div>
          </div>
        </div> 
</template>

<script>
//  if(HolidaysTable === null) HolidaysTable = $('#HolidaysTable').DataTable({ responsive: false, paging: false, scrollCollapse: true, scrollY: '224px', dom: '<"clear">t<"clear">', columnDefs: [ { targets: [0, 1, 2], orderable: true }, { targets: [1, 2], className: 'text-center' }, { targets: [3], orderable: false } ], order: [1, 'asc']});

  </script>