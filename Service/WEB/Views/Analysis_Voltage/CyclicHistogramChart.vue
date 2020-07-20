<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">Cyclic Histogram</div>
    </div>
    <div class="col-lg-4">
      <h3>Phase 1</h3>
      <div id="graphPhase1"></div>
    </div>
    <div class="col-lg-4">
      <h3>Phase 2</h3>
      <div id="graphPhase2"></div>
    </div>
    <div class="col-lg-4">
      <h3>Phase 3</h3>
      <div id="graphPhase3"></div>
    </div>
  </div>
</template>

<script>
import moment from "moment";
import Plotly from "plotly.js-dist";
import * as oZm from "JS/openZmeter.js";
export default {
  props: { analyzer: Object, id: "int" },
  created: function() {
    let event_id = this.$route.params.id;
    this.$http
      .post("getCyclicHistogram", {
        StartTime: event_id,
        Analyzer: this.analyzer.Analyzer
      })
      .then(
        result => {
          let data = result.body;
          let points = data.points;

          let points_phase_a = points.filter(x => x.phase == 0);
          let points_phase_b = points.filter(x => x.phase == 1);
          let points_phase_c = points.filter(x => x.phase == 2);

          let x_map_fn = function(point) {
            return point.sample;
          };

          let y_map_fn = function(point) {
            return point.voltage_bin;
          };

          let z_map_fn = function(point) {
            return point.count;
          };

          let TESTER = document.getElementById("graphPhase1");
          Plotly.newPlot(
            TESTER,
            [
              {
                x: points_phase_a.map(x_map_fn),
                y: points_phase_a.map(y_map_fn),
                z: points_phase_a.map(z_map_fn),
                type: "heatmap",
                hoverongaps: false
              }
            ],
            {
              margin: { t: 0 }
            }
          );
        },
        fail => {
          this.ShowSpinner = false;
          alert("Cyclic Histogram samples are not avaliable right now.");
        }
      );
  },
  data() {
    return {};
  },
  computed: {}
};
</script>

<style scoped>
.header-text {
  font-size: 14px;
}
</style>