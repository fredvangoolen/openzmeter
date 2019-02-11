<template>
  <div class="modal-content">
    <!--div id="divAnalyzersTariffSpinner" class="spinnerdiv"></div-->
    <div class="modal-header">
      <div class="pull-left"><h4 class="modal-title">Analyzers &raquo; Tariffs</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" id="btnAnalyzersTariffsAdd"><img src="Imgs/plus.png" alt="New tariff"></button>
        <button type="button" class="btn btn-default btn-xs" id="btnAnalyzersTariffsBack"><img src="Imgs/arrow_undo.png" alt="Discard"></button>
      </div>
    </div>
    <div class="modal-body">
      <table id="tblAnalyzersTariff" class="table small table-striped table-responsive" style="width: 100%">
        <thead>
          <tr>
            <th>Name</th><th>Module</th><th>Start date</th><th></th>
          </tr>
        </thead>
        <tbody>
        </tbody>
      </table>                  
    </div>
  </div>
</template>

<script>
  export default {
      data() {
        return {};
      } 
  }
</script>
  
<style>

</style>