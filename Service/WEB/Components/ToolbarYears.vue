<template>
  <div class="btn-group">
    <div class="input-group input-group-xs">
      <span class="input-group-addon"><img :src="require('Imgs/outlook_calendar_work_week.png')"></span>
      <div class="input-group-btn">
        <button class="btn btn-default dropdown-toggle" type="button" data-toggle="dropdown"><span>{{ (value == null) ? 'No data' : value.toString() }}</span><span class="caret"></span></button>
        <ul class="dropdown-menu pull-right">
          <li v-for="year in Years" :key="year" v-bind:class="{ active: (year == value) }"><a @click="$emit('input', parseInt(year))">{{ year }}</a></li>
        </ul>
      </div>
    </div>
  </div>
</template> 

<script>
  import moment from 'moment';
  export default {
    props: { storage: { type: String, default: null}, value: { type: Number, default: null }, aggreg: { type: String, default: '15M' }, analyzer: { type: Object } },
    data() {
      return { Years: [] };
    },
    created: function() {
      if(this.analyzer !== null) this.GetYears();
    },
    watch: {
      analyzer: function(val) {
        this.Years = [];
        this.$emit('input', null);
        if(val !== null) this.GetYears();
      },
      value: function(val) {
        if((this.storage != null) && (val != null)) localStorage.setItem(this.storage, val);
      }
    },
    methods: {
      GetYears: function() {
        var Years = [];
        this.$http.post("getSeriesRange", { Aggreg: this.aggreg, Analyzer: this.analyzer.Analyzer }).then( 
          (result) => {
            var StartYear = moment(result.data.From).year();
            var EndYear = moment(result.data.To).year();
            for(var year = StartYear; year <= EndYear; year++) Years.push(year);
            this.Years = Years.reverse();
            var Stored = null;
            if(this.storage != null) {
              Stored = parseInt(localStorage.getItem(this.storage));
              if(isNaN(Stored) == true) Stored = null;
            }
            if(Years.length > 0) {
              if((Stored >= StartYear) && (Stored <= EndYear)) {
                this.$emit('input', Stored);
              } else {
                this.$emit('input', parseInt(Years[0]));
              }
            } else {
              this.$emit('input', null);
            }
          }
        ); 
      }
    }
  }
</script>

<style scoped>
  .btn-group ul {
    max-height: 142px;
    overflow-y: auto;
  }
</style>