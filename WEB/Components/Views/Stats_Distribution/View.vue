<template>
  <div id="wrapper">
    <div class="row">
      <div class="col-lg-12">
        <div class="panel panel-active-energy">
          <div class="panel-heading">
            <span class="header-text">Energy distribution</span>
            <div class="header-buttons clearfix">
              <color-bar :width="140" :height="10" ref="colorbar"></color-bar>
              <toolbar-years @YearChanged="YearChanged" aggreg="1H"></toolbar-years>
            </div>
          </div>
          <div class="panel-body">
            <spinner v-if="ShowSpinner"></spinner>
            <calendar ref="calendar" v-on:RangeChanged="RangeChanged"></calendar>
          </div>
        </div>
      </div>
      <div class="col-md-12 col-lg-6"><months ref="months"></months></div>
      <div class="col-md-12 col-lg-6"><day ref="day"></day></div>
      <div class="col-md-12 col-lg-6"></div>
      <!--div class="row">
                <div class="col-md-12">
                  <div class="panel panel-other"> 
                    <div class="panel-heading">
                      <span>By day</span>
                      <div style="float: right; width: 200px">
                        <div id="CalendarScale" style="height: 10px; width: 200px; display: flex; flex-direction:row; border: 1px solid #EEE"></div>
                        <div style="float: left; font-size: 80%" id="CalendarMin">?</div>
                        <div style="float: right; font-size: 80%" id="CalendarMax">?</div>
                        <div style="text-align: center; font-size: 80%"><em id="CalendarUnits"></em></div>
                      </div>
                    </div>
                    
                  </div>
              </div>
            </div>
          
          
          

                  
                  <div class="panel panel-other">
                    <div class="panel-heading">
                      <i class="fa fa-clock-o" aria-hidden="true"></i></i> By hour
                      <div style="float: right" id="HourUnits"></div>
                    </div>
                    <div class="panel-body">
                      <table id="HourTable" class="table small table-striped table-responsive">
                        <thead>
                          <tr>
                            <th style="width: 30px">Hour</th><th style="width: 100%"></th><th>Average</th><th>Accumulated</th>
                          </tr>
                        </thead>
                        <tbody>
                        </tbody>
                      </table>
                    </div>
                  </div>
                </div>
                <div class="col-md-12 col-lg-6">
                  <div class="panel panel-other">
                    <div class="panel-heading">
                      <i class="fa fa-bars" aria-hidden="true"></i> By week
                      <div class="pull-right" id="WeekUnits"></div>
                    </div>
                    <div class="panel-body">
                      <table id="WeekTable" class="table small table-responsive">
                        <thead>
                          <tr>
                            <th style="width: 30px">Week</th><th style="width: 100%"></th><th>Weekday</th><th>Weekend</th><th>Total</th>
                          </tr>
                        </thead>
                        <tbody>
                        </tbody>
                      </table>
                    </div>
                  </div>
                </div>
              </div>
            </div>
          </div>
        </div-->
      </div>
 </div>
</template>

<script>
  import Calendar from './Calendar.vue';
  import Months from './Months.vue';
  import Day from './Day.vue';
  import ToolbarYears from 'Components/ToolbarYears.vue';
  import ColorBar from 'Components/ColorBar.vue';
  import Spinner from 'Components/Spinner.vue';
  import moment from 'JS/moment.js';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    data: function() {
      return { ShowSpinner: true };
    },
    methods: {
      YearChanged: function(year) {
        this.ShowSpinner = true;
        if(this.$root.SelectedAnalyzer === null) return;
        var Analyzer = this.$root.SelectedAnalyzer;
        var Year = moment().year(year);
        $.ajax({ url: "getPrices", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify( { Period: Year.endOf('year').valueOf() - Year.startOf('year').valueOf(), To: Year.endOf('year').valueOf(), Aggreg: '1H', LastSample: 0, Analyzer: Analyzer.ID } ),
          success: function(result) {
//            UpdateWeeks(visibleSeries);
//    UpdateMonths(visibleSeries);
//    UpdateWeekDays(visibleSeries);
//    UpdateHours(visibleSeries);
    
    
    
    this.ShowSpinner = false;
            this.$refs.calendar.SetData(result);
          }.bind(this)
        });
      },
      RangeChanged(min, max, units) {
        this.$refs.colorbar.SetRange(oZm.Round2(min), oZm.Round2(max), units + 'Wh');
      }
    },
    components: { Spinner, ToolbarYears, Calendar, Months, Day, ColorBar }
  }
</script>

<style scoped>
  .header-text {
    font-size: 24px;
    line-height: 16px;
  }
</style>