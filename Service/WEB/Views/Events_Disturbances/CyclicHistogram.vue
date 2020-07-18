<template>
  <div class="row">
    <div v-if="(analyzer!=null)&&(id!=null)">
      <p>Event ID: {{id}}</p>
      <div id="tester" style="width:600px;height:250px;"></div>
    </div>
    <div v-else-if="analyzer==null">
      <div class="alert alert-danger text-center">
        <p>
          No selected analyzer or event. Go to
          <router-link to="/Dashboard">Dashboard</router-link>
        </p>
      </div>
    </div>
  </div>
</template>

<script>
import * as oZm from "JS/openZmeter.js";
import Plotly from 'plotly.js-dist';
export default {
  props: { id: "int", analyzer: Object },
  created: function() {
    let event_id = this.$route.params.id;
    if (event_id != null && this.analyzer != null) {
      this.$http
        .post("getCyclicHistogram", {
          StartTime: event_id,
          Analyzer: this.analyzer.Analyzer
        })
        .then(
          result => {
            let data = result.body;
            
            let TESTER = document.getElementById("tester");
            Plotly.newPlot(
              TESTER,
              [
                {
                  x: data.points.x,
                  y: data.points.y
                }
              ],
              {
                margin: { t: 0 }
              }
            );
          },
          fail => {
            this.ShowSpinner = false;
            alert(
              "Cyclic Histogram samples are not avaliable right now. The event may still be running or deleted."
            );
          }
        );
    }
  },
  data: function() {
    return {};
  },
  methods: {}
};
</script>