<template>
                      <div class="panel panel-default" style="margin: 5px">
                      <div class="panel-heading">
                        Harmonic distorsion
                        <div id="THDWarning" class="title-warning hidden"><i class="fa fa-warning" style="color: #d9534f"></i> Exceded</div>
                        <div class="pull-right small" id="THDLegend"></div>
                      </div>
                      <div id="THDChart" style="margin: 5px; min-height: 240px"></div>
                      <div class="panel-footer">
                        <div class="text-center">100% of the year under 8%</div>
                        <div class="progress" style="margin-top:5px; margin-bottom:0px; position: relative; background-color: #FFF">
                          <div class="progress-bar" id="THDInRange" role="progressbar" style="width:0%; background-color: #5cb85c; position: absolute"></div>
                          <span id="THDInRangeText" style="position: absolute; left: 0px; width: 100%; position: absolute; text-align: center"></span>
                        </div>
                        <div style="float: left; font-size: 10px">90%</div><div style="float:right; font-size: 10px">100%</div>
                        <div class="clearfix"></div>
                      </div>
                    </div>
</template>

<script>


</script>
