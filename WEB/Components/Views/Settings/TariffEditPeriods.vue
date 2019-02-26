<template>
          <div class="modal-content hide" id="divTariffPeriods">
          <div class="modal-header">
            <div class="pull-left"><h4 class="modal-title">Tariffs template &raquo; Periods</h4></div>
            <div class="pull-right btn-group">
              <button type="button" class="btn btn-default btn-xs" id="btnTariffPeriodsDel"><img src="Imgs/cross.png" alt="Delete period"></button>
              <button type="button" class="btn btn-default btn-xs" id="btnTariffPeriodsAdd"><img src="Imgs/plus.png" alt="New period"></button>
              <button type="button" class="btn btn-default btn-xs" id="btnTariffPeriodsBack"><img src="Imgs/arrow_undo.png" alt="Go back"></button>
            </div>
          </div>
          <div class="modal-body">
            <div class="alert alert-info text-center" style="margin-bottom: 5px; padding: 5px;">Define the maximum periods of days and their pricing model</div>
            <div class="row">
              <div class="col-xs-12">
                <table class="table small table-striped table-responsive" id="PeriodsTable">
                  <thead><tr><th style="width: 20px">Name</th><th>Power model</th><th>Reactive model</th></tr></thead>
                  <tbody></tbody>
                </table>
              </div>
            </div>
          </div>
        </div> 
</template>

<script>
  
  export default {
      
      
  }
  
  //  if(PeriodsTable === null) PeriodsTable = $('#PeriodsTable').DataTable({ responsive: false, paging: false, scrollCollapse: true, scrollY: '224px', dom: '<"clear">t<"clear">', columnDefs: [ { targets: [0, 1, 2], orderable: false, className: 'text-center' } ] } );

</script>