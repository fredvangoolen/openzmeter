<template>
  <div id="wrapper">
    <div class="row">
      <div class="col-lg-12">
        <div class="panel panel-other">
          <div class="panel-heading">
            <span class="header-text">EN50160</span>
            <div class="header-buttons clearfix">
              <toolbar-weeks variable="Week" v-on:PeriodChanged="WeekChange"></toolbar-weeks>
              <toolbar-years v-on:YearChanged="YearChanged"></toolbar-years>
            </div>
          </div>
          <div class="panel-body">
            <div class="row">
              <div class="col col-lg-3 col-md-6 col-xs-12">
                <div class="panel panel-lightblue-info text-center" style="position: relative">
                  <p><span class="huge-small">Voltage swell or dips</span><br/>Less than 1 min and -15%/+10% nominal voltage</p>
                      <div class="progress" style="margin-bottom:0px; position: relative">
                        <div class="progress-bar" id="DipEvents" role="progressbar" style="width:0%; background-color: #5cb85c; position: absolute"></div>
                        <span id="DipEventsText" style="color: #333; position: absolute; left: 0px; width: 100%; text-align: center"></span>
                      </div>
                      <div style="font-size: 10px">10 to 1000 allowed per year</div>
                </div>
              </div>
              <div class="col col-lg-3 col-md-6 col-xs-12">
                    <div class="panel panel-lightblue-info text-center">
                      <p><span class="huge-small">Short interruptions</span><br/>Less than 3 min and under 1% nominal voltage</p>
                      <div class="progress" style="margin-bottom:0px; position: relative">
                        <div class="progress-bar" id="ShortEvents" role="progressbar" style="width:0%; background-color: #5cb85c; position: absolute"></div>
                        <span id="ShortEventsText" style="color: #333; position: absolute; left: 0px; width: 100%; text-align: center"></span>
                      </div>
                      <div style="font-size: 10px">10 to 100 allowed per year</div>
                    </div>
              </div>                  
                  <div class="col col-lg-3 col-md-6 col-xs-12">
                    <div class="panel panel-lightblue-info text-center">
                      <p><span class="huge-small">Long interruptions</span><br/>More than 3 min and under 1% nominal voltage</p>
                      <div class="progress" style="margin-bottom:0px; position: relative">
                        <div class="progress-bar" id="LongEvents" role="progressbar" style="width:0%; background-color: #5cb85c; position: absolute"></div>
                        <span id="LongEventsText" style="color: #333; position: absolute; left: 0px; width: 100%; text-align: center"></span>
                      </div>
                      <div style="font-size: 10px">10 to 50 allowed per year</div>
                    </div>
                  </div>
                  <div class="col col-lg-3 col-md-6 col-xs-12">
                    <div class="panel panel-lightblue-info text-center">
                      <p><span class="huge-small">Harmonics</span><br/>Harmonics in valid ranges as detailed in EN50160</p>
                      <div class="progress" style="margin-bottom:0px; position: relative">
                        <div class="progress-bar" id="HarmonicsInRange" role="progressbar" style="width:0%; background-color: #5cb85c; position: absolute"></div>
                        <span id="HarmonicsInRangeText" style="color: #333; position: absolute; left: 0px; width: 100%; text-align: center"></span>
                      </div>
                      <div style="float: left; font-size: 10px">90%</div><div style="float:right; font-size: 10px">100%</div>
                      <div class="clearfix"></div>
                    </div>
                  </div>
                </div>
              </div>
            </div>
          </div>

    </div>
    <div class="row">
      <div class="col col-lg-4 col-md-12"><frequency ref="frequency"></frequency></div>
      <div class="col col-lg-4 col-md-12"><voltage ref="voltage"></voltage></div>
      <div class="col col-lg-4 col-md-12"><thd ref="thd"></thd></div>
    </div>
  </div>
</template>

<script>
  import ToolbarYears from 'Components/ToolbarYears.vue';
  import ToolbarWeeks from 'Components/ToolbarWeeks.vue';
  import ProgressBar from 'Components/ProgressBar.vue';
  export default {
    data: function() {
      return { };
    },
    components: { ToolbarYears, ProgressBar, ToolbarWeeks }
}
</script>

<style scoped>
  .header-text {
    font-size: 24px;
    line-height: 16px;
  }
</style>
