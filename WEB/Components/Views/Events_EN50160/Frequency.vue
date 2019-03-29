<template>
                      <div class="panel panel-default" style="margin: 5px">
                      <div class="panel-heading">
                        Grid frequency
                        <div id="FreqWarning" class="title-warning hidden"><i class="fa fa-warning" style="color: #d9534f"></i> Exceded</div>
                        <div class="pull-right small" id="GridFrequencyLegend"></div>
                      </div>
                      <div id="FrequencyChart" style="margin: 5px; min-height: 240px"></div>
                      <div class="panel-footer">
                        <div class="text-center">99.5% of the year between <span id="GridNominalFreq">?</span>±1%Hz with no events allowed outside 47 to 52Hz</div>
                        <div class="progress" style="margin-top:5px; margin-bottom:0px; position: relative; background-color: #FFF">
                          <div class="progress-bar" id="FreqInRange" role="progressbar" style="width:0%; background-color: #5cb85c; position: absolute"></div>
                          <span id="FreqInRangeText" style="position: absolute; left: 0px; width: 100%; position: absolute; text-align: center"></span>
                        </div>
                        <div style="float: left; font-size: 10px">90%</div><div style="float:right; font-size: 10px">100%</div>
                        <div class="clearfix"></div>
                      </div>
                    </div>
</template>
