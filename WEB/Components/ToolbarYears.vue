<template>
  <div class="btn-group">
    <div class="input-group input-group-xs">
      <span class="input-group-addon"><img src="Imgs/outlook_calendar_work_week.png"></span>
      <div class="input-group-btn">
        <button class="btn btn-default dropdown-toggle" type="button" data-toggle="dropdown"><span>{{ SelectedText }}</span><span class="caret"></span></button>
        <ul class="dropdown-menu pull-right">
          <li v-for="year in Years" v-bind:class="{ active: (year == SelectedText) }"><a @click="YearClick(year, $event)">{{ year }}</a></li>
        </ul>
      </div>
    </div>
  </div>
</template> 

<script>
  import moment from 'JS/moment.js';
  export default {
    data() {
      return { SelectedText: 'No data', Years: [] };
    },
    methods: {
      YearClick: function(year, event) {
        event.preventDefault();
        this.SelectedText = year;
        this.$emit('YearChanged', year);
      },
      AnalyzerChanged: function() {
        if(this.$root.SelectedAnalyzer === null) return;
        var Years = [];
        $.ajax({ url: "getSeriesRange", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ Aggreg: '3S', Analyzer: this.$root.SelectedAnalyzer.ID }), 
          success: function(result) {
            for(var year = new Date(result['From']).getFullYear(); year <= new Date(result['To']).getFullYear(); year++) Years.push(year);
            this.Years = Years;
            if(Years.length > 0) {
              this.SelectedText = Years[0];
              this.$emit('YearChanged', Years[0]);
            } else {
              this.SelectedText = 'No data';
              this.$emit('YearChanged', null);
            }
          }.bind(this)
        }); 
      }
    },
    created: function() {
      this.$root.$on('AnalyzerChanged', this.AnalyzerChanged);
    },
    mounted: function() {
      this.AnalyzerChanged();
    }
  }
</script>

<style>
</style>