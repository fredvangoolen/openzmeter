<template>
            <div class="panel panel-other">
            <div class="panel-heading">
              <div class="huge-small header-text">Tariffs</div>
              <div class="header-buttons">
                <div class="btn-group">
                  <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Save changes" id="btnAddTariff"><img src="Imgs/plus.png"> Add</button>
                </div>
              </div>
            </div>
            <div class="panel-body">
              <table id="TariffsTable" class="table small table-striped table-responsive">
                <thead><tr><th style="width: 20px">Country</th><th>Name</th><th style="width: 120px">Start date</th><th style="width: 120px">End date</th><th style="width: 80px"></th></tr></thead>
                <tbody></tbody>
              </table>
            </div>
          </div>
</template>
