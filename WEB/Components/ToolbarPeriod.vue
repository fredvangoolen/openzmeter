<template>
  <div class="btn-group">
    <div class="input-group input-group-xs">
      <span class="input-group-addon"><img src="Imgs/outlook_calendar_work_week.png"></span>
      <div class="input-group-btn">
        <button class="btn btn-default dropdown-toggle" type="button" data-toggle="dropdown">{{ SelectedText }}<span class="caret"></span></button>
        <ul class="dropdown-menu pull-right">
          <li v-bind:class="{active: (Selected == 86400000)}"><a @click="PeriodClick(86400000, $event)" href="#">Day</a></li>
          <li v-bind:class="{active: (Selected == 604800000)}"><a @click="PeriodClick(604800000, $event)" href="#">Week</a></li>
          <li v-bind:class="{active: (Selected == 2592000000)}"><a @click="PeriodClick(2592000000, $event)" href="#">Month</a></li>
          <li v-bind:class="{active: (Selected == 31536000000)}"><a @click="PeriodClick(31536000000, $event)" href="#">Year</a></li>
        </ul>
      </div>
    </div>
  </div>
</template> 

<script>
  export default {
    props: { variable: String },
    data() {
      return { Selected: 86400000, SelectedText: 'Day' };
    },
    methods: {
      PeriodClick(period, event) {
        event.preventDefault();
        this.Selected = period;
        this.SelectedText = $(event.target).text();
        localStorage.setItem(this.variable, period);
        this.$emit('PeriodChanged', this.Selected);
        console.log(period);
      },
      SetPeriod(period) {
        this.Selected = period;
        if(period === 86400000) {
          this.SelectedText = 'Day';
        } else if(period === 604800000) {
          this.SelectedText = 'Week';
        } else if(period === 2592000000) {
          this.SelectedText = 'Month';
        } else if(period === 31536000000) {
          this.SelectedText = 'Year';
        } else {
          this.SelectedText = 'Custom';
        }  
      }
    },
    mounted: function() {
      this.Selected = parseInt(localStorage.getItem(this.variable));
      if(isNaN(this.Selected)) this.Selected(86400000);
      this.SetPeriod(this.Selected);
      this.$emit('PeriodChanged', this.Selected);
    }
  }
</script>