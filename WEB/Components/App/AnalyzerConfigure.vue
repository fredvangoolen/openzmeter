<template>
  <div class="modal-content">
    <div class="modal-header">
      <div class="pull-left"><h4 class="modal-title">Analyzers &raquo; Configuration</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" id="btnAnalyzersConfigSave"><img src="Imgs/diskette.png" alt="Save"></button>
        <button type="button" class="btn btn-default btn-xs" id="btnAnalyzersConfigBack"><img src="Imgs/arrow_undo.png" alt="Discard"></button>
      </div>
    </div>
    <div class="modal-body">
      <div class="row" id="divAnalyzersConfig">
        <div class="col-xs-12">
          <label class="control-label">Serial number</label>
          <input type="text" class="form-control" id="inputAnalyzerID" disabled>
        </div>
        <div class="col-xs-12">
          <label class="control-label">Display name</label>
          <input type="text" class="form-control" id="inputAnalyzerName">
        </div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">Voltage</label>
          <div class="input-group">
            <input type="text" class="form-control" id="inputAnalyzerVoltage">
            <span class="input-group-addon" id="basic-addon1">V</span>
          </div>
        </div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">Frequency</label>
          <div class="input-group">
            <input type="text" class="form-control" id="inputAnalyzerFrequency">
            <span class="input-group-addon" id="basic-addon1">Hz</span>
          </div>
        </div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">CUPS</label>
          <input type="text" class="form-control" id="inputAnalyzerCUPS">
        </div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">Postal code</label>
          <input type="text" class="form-control" id="inputAnalyzerZIP">
        </div>
        <div class="col-xs-12 col-sm-12">
          <label class="control-label">Location</label>
          <div class="input-group">
            <span class="input-group-addon">Lat</span>
            <input type="text" class="form-control" id="inputAnalyzerLat">
            <span class="input-group-addon">Long</span>
            <input type="text" class="form-control" id="inputAnalyzerLong">
            <span class="input-group-btn"><button class="btn btn-default"><img src="Imgs/globe_africa.png"></button></span>
          </div>
        </div>
      </div>
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