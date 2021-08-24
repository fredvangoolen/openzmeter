<template>
  <div class="row">
    <div v-if="(analyzer!=null)&&(id!=null)">
      <p>Event ID: {{id}}</p>
      <cyclic-histogram-chart :id="id" :analyzer="analyzer"></cyclic-histogram-chart>
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
import CyclicHistogramChart from "../Analysis_CyclicHistogram/CyclicHistogramChart.vue";
export default {
  props: { id: "string", analyzer: Object },
  created: function() {
    //let event_id = this.$route.params.id;
  },
  data: function() {
    return {};
  },
  methods: {},
  components: { CyclicHistogramChart }
};
</script>