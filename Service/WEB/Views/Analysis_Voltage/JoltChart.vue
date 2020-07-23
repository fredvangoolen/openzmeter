<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">Jolt</div>
    </div>
    <div class="col-lg-8 col-md-12" style="padding: 50px;">
      <div id="graphJolt"></div>
    </div>
    <div class="col-lg-4 col-md-12" style="padding: 50px;">
      <h3>Jolt Information</h3>
      <p>Start Time: {{startTime_str}}</p>
      <p>End Time: {{endTime_str}}</p>
    </div>
  </div>
</template>

<script>
import moment from "moment";
import Plotly from "plotly.js-dist";
import * as oZm from "JS/openZmeter.js";
export default {
  props: { analyzer: Object, range: [String, Array] },
  created: function() {
    this.UpdateRange();
    this.timer = setInterval(this.UpdateJoltInformation, 5000);

    this.UpdateJoltData();
  },
  beforeDestroy() {
    clearInterval(this.timer);
  },
  methods: {
    UpdateRange: function() {
      if (this.range != null) {
        if (typeof this.range == "string") {
          // Parse predefined range string
          switch (this.range) {
            case "LastHours":
              this.startTime = moment().subtract(2, "hour");
              this.endTime = moment();
              break;
            case "LastDay":
              this.startTime = moment().subtract(1, "day");
              this.endTime = moment();
              break;
            case "LastWeek":
              this.startTime = moment().subtract(1, "week");
              this.endTime = moment();
              break;
            case "LastMonth":
              this.startTime = moment().subtract(1, "month");
              this.endTime = moment();
              break;
            case "LastYear":
              this.startTime = moment().subtract(1, "year");
              this.endTime = moment();
              break;
            case "Day":
              this.startTime = moment().startOf("day");
              this.endTime = moment();
              break;
            case "Week":
              this.startTime = moment().startOf("week");
              this.endTime = moment();
              break;
            case "Month":
              this.startTime = moment().startOf("month");
              this.endTime = moment();
              break;
            case "Year":
              this.startTime = moment().startOf("year");
              this.endTime = moment();
              break;
            default:
              // Unexpected string; cannot determine range to use.
              this.startTime = null;
              this.endTime = null;
              break;
          }
        } else if (Array.isArray(this.range)) {
          if (this.range.length == 2) {
            // Extract [start, end] epoch timestamps from the array
            this.startTime = moment(this.range[0]);
            this.endTime = moment(this.range[1]);
          } else {
            this.startTime = null;
            this.endTime = null;
          }
        }
      } else {
        this.startTime = null;
        this.endTime = null;
      }
    },
    UpdateJoltData: function() {
      if (this.startTime != null && this.endTime != null) {
        this.$http
          .post("getJoltData", {
            StartTime: this.startTime.valueOf(),
            EndTime: this.endTime.valueOf(),
            Analyzer: this.analyzer.Analyzer
          })
          .then(
            result => {
              this.UpdateChart();
            },
            fail => {
              alert("Jolt data samples are not avaliable right now.");
            }
          );
      } else {
        this.joltData = null;
      }
    },
    UpdateChart: function() {},
    UpdateJoltInformation() {
      if (this.startTime != null) {
        this.startTime_str =
          this.startTime.format("LLLL") + " (" + this.startTime.fromNow() + ")";
      } else {
        this.startTime_str = "";
      }

      if (this.endTime != null) {
        this.endTime_str =
          this.endTime.format("LLLL") + " (" + this.endTime.fromNow() + ")";
      } else {
        this.endTime_str = "";
      }
    }
  },
  data() {
    return {
      timer: null,
      startTime: null,
      endTime: null,
      startTime_str: "",
      endTime_str: "",
      joltData: null
    };
  },
  watch: {
    range: function() {
      this.UpdateRange();
      this.UpdateJoltData();
    },
    startTime: function() {
      this.UpdateJoltInformation();
    },
    endTime: function() {
      this.UpdateJoltInformation();
    }
  },
  computed: {}
};
</script>

<style scoped>
.header-text {
  font-size: 14px;
}
</style>