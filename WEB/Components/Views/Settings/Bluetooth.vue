<template>
  <div class="panel panel-other">
    <div class="panel-heading">
      <div class="huge-small header-text">Bluetooth</div>
      <div class="header-buttons">
        <div class="btn-group">
          <button type="button" class="btn btn-default btn-xs disabled" data-toggle="tooltip" title="Save changes"><img :src="require('Imgs/diskette.png')"> Save</button>
        </div>
      </div>
    </div>
    <div class="panel-body">
      <div class="row">
        <div class="col-xs-12">
          <div class="input-group">
            <span class="input-group-addon input-group-addon-toogle">
              <input type="checkbox" data-toggle="toggle" id="checkNTP" data-size="small" data-onstyle="success">
            </span>
            <input type="text" class="form-control" id="inputNTPServer">
          </div>
        </div>
      </div>
    </div>
  </div>
</template>