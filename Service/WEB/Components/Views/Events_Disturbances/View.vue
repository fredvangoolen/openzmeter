<template>
  <div>
    <div class="row">
      <div class="col-lg-12">
        <div class="panel panel-other">
          <!--div id="DisturbancesSpinner" class="spinnerdiv"></div-->
          <div class="panel-heading">
            <span class="header-text">Disturbances</span>
            <!--div class="pull-right">
                  <span class="small"></span><br>
                  <div class="btn-group">
                    <button type="button" class="btn btn-default btn-xs" id="btnDisturbancesNow" data-toggle="tooltip" title="Move to most recent samples and update in real time"><i class="fa fa-play"></i></button>
                  </div>
                </div>
                <div class="pull-right">&nbsp;</div>
                <div class="pull-right">
                  <span class="small">Time scale:</span><br>
                  <div class="btn-group">
                    <button type="button" class="btn btn-default btn-xs" id="btnShowEventYear"  data-toggle="tooltip" title="Select exactly a year period from current time">Year</button>
                    <button type="button" class="btn btn-default btn-xs" id="btnShowEventMonth" data-toggle="tooltip" title="Select exactly a month period from current time">Month</button>
                    <button type="button" class="btn btn-default btn-xs" id="btnShowEventWeek"  data-toggle="tooltip" title="Select exactly a week period from current time">Week</button>
                    <button type="button" class="btn btn-default btn-xs" id="btnShowEventDay"   data-toggle="tooltip" title="Select exactly a day period from current time">Day</button>
                  </div>
                </div>
                <span class="clearfix"> </span-->
          </div>
          <div class="panel-body">
            <div id="disturbances_chart"></div>
            <br>
            <div class="panel panel-default">
              <div class="panel-heading">
                <i class="fa fa-table fa-fw"></i> Details
              </div>
              <div class="panel-body">
                <div class="row">
                  <div class="col col-md-5">
                    <table id="EventsTable" class="table small table-striped table-responsive" cellspacing="0" width="100%">
                      <thead>
                        <tr>
                          <th style="width: 130px">Date</th><th>Type</th><th>Duration <em>(s)</em></th><th>ΔU <em>(V)</em></th><th>ΔU <em>(%)</em></th><th style="width: 60px"></th>
                        </tr>
                      </thead>
                      <tbody>
                      </tbody>
                    </table>
                  </div>
                  <div class="col col-md-7">
                    <div id="EventSpinner" class="spinnerdiv"></div>
                    <div id="EventWaveformPanel">
                      <div id="EventWaveformMaximize">
                        <div style="padding:5px">
                          <div style="display:inline-block">&nbsp;</div>
                          <div id="EventChartLegend" style="float: right"></div>
                        </div>
                        <div id="EventChart" class="chart big_chart"></div>
                      </div>
                    </div>
                  </div>
                </div>
              </div>
            </div>
          </div>

        </div>
      </div>
    </div>
  </div>
</template>

<script>
  import vis from 'JS/vis.js';
  import 'CSS/vis.css'  
  export default {
    data: function() {
      return { }
    },
    created: function() {
      this.TimeLine = null;
    },
    mounted: function() {
  if(this.TimeLine == null) {
    this.TimeLine = new vis.Timeline(document.getElementById('disturbances_chart'), null, {
      stack: false,
      editable: false,
      margin: {
        item: 10, // minimal margin between items
        axis: 5   // minimal margin between items and the axis
      },
      orientation: 'top'
    });
    this.TimeLine.on('rangechanged', function(evt) {
      if(evt.byUser === false) return;
      clearInterval(DisturbancesTimer);
      $('#btnDisturbancesNow').removeClass('active');
      localStorage.setItem('GraphEnd', evt.end.getTime());
      localStorage.setItem('GraphPeriod', evt.end.getTime() - evt.start.getTime());
      LastEventStarttime = 0;
      AjaxGetEvents();
    });
    var Groups = new vis.DataSet([
      { id: 1, content: 'Dips'},
      { id: 2, content: 'Swells'},
      { id: 3, content: 'Interruptions'},
      { id: 4, content: 'RVC'},
      { id: 5, content: 'Glitchs'}
    ]);
    this.TimeLine.setGroups(Groups);
  }
      
    }
  }
</script>

<style scoped>
  .header-text {
    font-size: 24px;
    line-height: 16px;
  }
</style>
