<template>
  <div class="panel panel-current" id="CurrentPanel">
            <div class="panel-heading">
              <div class="huge header-text">Current</div>
              <div class="header-buttons"  id="toolbarCurrent">
                <div class="btn-group" data-load-block="ScrollableAggreg_Block"></div>
                <span>&nbsp;</span>
                <div class="btn-group" data-load-block="ScrollableTime_Block"></div>
                <span>&nbsp;</span>
                <div class="btn-group" id="groupCurrentPhases" data-load-block="PhaseI_Block"></div>
                <span>&nbsp;</span>
                <div class="btn-group" data-load-block="GraphToolsPlay_Block"></div>
              </div>
              <span class="clearfix"></span>
            </div>
            <div class="panel-body">
              <spinner v-bind:class="{ 'hide': !ShowSpinner }"></spinner>
              <div class="row">
                <div class="col-md-6 col-lg-4 col-xl-3">
                  <div class="subpanel">
                    <div class="huge-small" id="labelCurrentAvgPrimary">Loading data...</div>
                    <div class="huge-small" id="labelCurrentAvgPrimaryValue">?</div>
                    <div><span id="labelCurrentAvgSecondary">Loading data...</span>&nbsp;<span id="labelCurrentAvgSecondaryValue"></span></div>
                  </div>
                  <div class="subpanel">
                    <div class="huge-small" id="labelCurrentMaxPrimary">Loading data...</div>
                    <div class="huge-small" id="labelCurrentMaxPrimaryValue">?</div>
                    <div><span id="labelCurrentMaxSecondary">Loading data...</span>&nbsp;<span id="labelCurrentMaxSecondaryValue"></span></div>
                  </div>
                  <div class="subpanel">
                    <div class="huge-small" id="labelCurrentMinPrimary">Loading data...</div>
                    <div class="huge-small" id="labelCurrentMinPrimaryValue">?</div>
                    <div><span id="labelCurrentMinSecondary">Loading data...</span>&nbsp;<span id="labelCurrentMinSecondaryValue"></span></div>
                  </div>
                </div>
                <div class="col-md-6 col-lg-8 col-xl-9">
                  <div>
                    <div id="CurrentChart" class="chart big_chart"></div>
                  </div>
                </div>
              </div>
            </div>
          </div>
        </div>

</template>
