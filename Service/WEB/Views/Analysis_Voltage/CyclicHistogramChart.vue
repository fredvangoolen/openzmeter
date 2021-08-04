<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">Cyclic Histogram</div>
    </div>
    <div class="col-lg-8 col-md-12" style="padding: 50px">
      <div id="graphCyclicHistogram"></div>
    </div>
    <div class="col-lg-4 col-md-12" style="padding: 50px">
      <h3>Cyclic Histogram Information</h3>
      <p>Capture Start Time: {{ starttime_str }} ({{ starttime_ago_str }})</p>
      <p>Capture End Time: {{ endtime_str }} ({{ endtime_ago_str }})</p>
      <p v-if="id != null">
        Event Start Time: {{ event_starttime_str }} ({{
          event_starttime_ago_str
        }})
      </p>
      <button
        type="button"
        class="btn btn-primary"
        @click="UpdateCyclicHistogramData"
      >
        Refresh Chart
      </button>
      <button
        type="button"
        class="btn btn-danger"
        @click="ClearCyclicHistogramData"
      >
        Clear Chart
      </button>
    </div>
  </div>
</template>

<script>
import moment from "moment";
import Plotly from "plotly.js-dist";
import * as oZm from "JS/openZmeter.js";

const arrayMinMax = (arr) =>
  arr.reduce(
    ([min, max], val) => [Math.min(min, val), Math.max(max, val)],
    [Number.POSITIVE_INFINITY, Number.NEGATIVE_INFINITY]
  );

export default {
  props: { analyzer: Object, id: "string" },
  created: function () {
    this.event_id = this.$route.params.id;
    this.timer = setInterval(this.UpdateCyclicHistogramTimes, 5000);
    this.UpdateCyclicHistogramData();
  },
  beforeDestroy() {
    Plotly.purge(this.graphCyclicHistogram);
    clearInterval(this.timer);
  },
  methods: {
    UpdateCyclicHistogramData() {
      fetch("getCyclicHistogramConfig", {
        method: "POST",
        body: JSON.stringify({
          ID: this.event_id,
          Analyzer: this.analyzer.Analyzer,
        }),
        headers: {
          Accept: "application/json",
          "Content-Type": "application/json",
        },
      }).then((config_result) => {
        if (config_result.status == 200) {
          config_result.json().then((config) => {
            this.maxAmplitude = config["MaxAmplitude"];
            this.binCount = config["BinCount"];

            if (config["Enabled"]) {
              fetch("getCyclicHistogramData", {
                method: "POST",
                body: JSON.stringify({
                  ID: this.event_id,
                  Analyzer: this.analyzer.Analyzer,
                }),
                headers: {
                  Accept: "application/json",
                  "Content-Type": "application/json",
                },
              }).then((result) => {
                if (result.status == 200) {
                  result.json().then((data) => {
                    this.cyclicHistogramData = data.points;

                    this.starttime = moment(data.starttime);
                    this.starttime_str = this.starttime.format(
                      "dddd, MMM D, YYYY hh:mm:ss.SSS"
                    );

                    this.endtime = moment(data.endtime);
                    this.endtime_str = this.endtime.format(
                      "dddd, MMM D, YYYY hh:mm:ss.SSS"
                    );

                    if (this.event_id != null) {
                      this.event_starttime = moment(this.event_id);
                      this.event_starttime_str = this.event_starttime.format(
                        "dddd, MMM D, YYYY hh:mm:ss.SSS"
                      );
                    }
                  });
                } else {
                  this.cyclicHistogramData = [];
                  alert(
                    "Cyclic Histogram samples are not avaliable right now."
                  );
                }
              });
            }
          });
        }
      });
    },
    UpdateCyclicHistogramChart() {
      this.graphCyclicHistogram = document.getElementById(
        "graphCyclicHistogram"
      );

      let traces = [];

      const z_min_max = arrayMinMax(this.cyclicHistogramData.map((x) => x[3]));

      const z_min = z_min_max[0];
      const z_max = z_min_max[1];

      // Build the color scale
      function get_colorscale_normalized_value(k, N) {
        return (k / 4) * (1 - 3600 / N) + 3600 / N;
      }

      let colorscale = null;
      if (z_max > 3600) {
        colorscale = [
          [0, "rgb(0,34,234)"], // 1 cycles (static region)
          [10 / z_max, "rgb(0,140,89)"], // 10 cycles (static region)
          [30 / z_max, "rgb(28,210,0)"], // 30 cycles (static region)
          [180 / z_max, "rgb(136,231,0)"], // 180 cycles (static region)
          [3600 / z_max, "rgb(255,252,0)"], // 3600 cycles (static region)
          [get_colorscale_normalized_value(1, z_max), "rgb(255,200,0)"],
          [get_colorscale_normalized_value(2, z_max), "rgb(255,155,0)"],
          [get_colorscale_normalized_value(3, z_max), "rgb(255,96,0)"],
          [1, "rgb(255,0,0)"],
        ];
      } else {
        colorscale = [
          [0, "rgb(0,34,234)"],
          [0.125, "rgb(0,140,89)"],
          [0.25, "rgb(28,210,0)"],
          [0.375, "rgb(136,231,0)"],
          [0.5, "rgb(255,252,0)"],
          [0.625, "rgb(255,200,0)"],
          [0.75, "rgb(255,155,0)"],
          [0.875, "rgb(255,96,0)"],
          [1, "rgb(255,0,0)"],
        ];
      }

      let phases = [0, 1, 2];

      phases.forEach((phase) => {
        let points_filtered = this.cyclicHistogramData.filter(
          (x) => x[0] == phase
        );
        let phase_number = phase + 1;

        let new_trace = {
          x: points_filtered.map((x) => x[1]),
          y: points_filtered.map((x) => x[2]),
          z: points_filtered.map((x) => x[3]),
          name: "Phase " + phase_number,
          type: "heatmap",
          coloraxis: "coloraxis",
          showlegend: true,
          zmin: z_min,
          zmax: z_max,
          connectgaps: false,
          hoverongaps: false,
        };

        if (phase != 0) {
          new_trace["visible"] = "legendonly";
        }

        traces.push(new_trace);
      });

      let layout = {
        height: 600,
        title: { text: "Cyclic Histogram" },
        xaxis: { title: "Sample Number" },
        yaxis: { title: "Voltage Bin Number", range: [0, this.binCount] },
        legend: {
          orientation: "h",
          x: 0.1,
          y: 1.2,
        },
        margin: { t: 0 },
        coloraxis: { colorscale: colorscale },
        plot_bgcolor: "#000000",
        paper_bgcolor: "#F1F1F1",
      };
      let config = { responsive: true };
      Plotly.react(this.graphCyclicHistogram, traces, layout, config);
    },
    UpdateCyclicHistogramTimes() {
      if (this.starttime != null) {
        this.starttime_ago_str = this.starttime.fromNow();
      } else {
        this.starttime_ago_str = "";
      }

      if (this.endtime != null) {
        this.endtime_ago_str = this.endtime.fromNow();
      } else {
        this.endtime_ago_str = "";
      }

      if (this.event_id != null) {
        this.event_starttime_ago_str = this.event_starttime.fromNow();
      }
    },
    ClearCyclicHistogramData() {
      if (confirm("Are you sure you want to clear Cyclic Histogram data?")) {
        fetch("clearCyclicHistogramData", {
          method: "POST",
          body: JSON.stringify({
            ID: this.event_id,
            Analyzer: this.analyzer.Analyzer,
          }),
          headers: {
            Accept: "application/json",
            "Content-Type": "application/json",
          },
        }).then((result) => {
          if (result.status == 200) {
          } else {
          }
        });
      }
    },
  },
  data() {
    return {
      cyclicHistogramData: [],
      graphCyclicHistogram: null,
      event_id: null,
      starttime: "",
      endtime: "",
      event_starttime: "",
      starttime_str: "",
      endtime_str: "",
      event_starttime_str: "",
      starttime_ago_str: "",
      endtime_ago_str: "",
      event_starttime_ago_str: "",
      timer_momentjs: "",
      timer: null,
    };
  },
  watch: {
    cyclicHistogramData: function () {
      this.UpdateCyclicHistogramTimes();
      this.UpdateCyclicHistogramChart();
    },
  },
  computed: {},
};
</script>

<style scoped>
.header-text {
  font-size: 14px;
}
</style>