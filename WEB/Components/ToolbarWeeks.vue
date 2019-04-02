<template>
  <div class="btn-group">
    <div class="input-group input-group-xs">
      <span class="input-group-addon"><img :src="require('Imgs/calendar_view_day.png')"></span>
      <div class="input-group-btn">
        <button class="btn btn-default dropdown-toggle" v-bind:class="{disabled: (weeks.length == 0)}" type="button" data-toggle="dropdown"><span>{{ SelectedText }}</span><span class="caret"></span></button>
        <ul class="dropdown-menu pull-right">
          <li v-for="(week, id) in weeks" v-bind:class="{ active: (week == SelectedText) }" v-if="week != undefined"><a @click="WeekClick(id, $event)">{{ week.Name }}</a></li>
        </ul>
      </div>
    </div>
  </div>
</template>

<script>
  import moment from 'JS/moment.js';
  export default {
    props: { weeks: Array },
    data() {
      return { SelectedText: 'All year' };
    },
    methods: {
      WeekClick: function(week, event) {
        event.preventDefault();
        this.SelectedText = this.weeks[week].Name;
        this.$emit('WeekChanged', week);
      }
    },
    watch: {
      weeks: function(val) {
        if(val.length === 0) return;
        this.SelectedText = val[0].Name;
        this.$emit('WeekChanged', 0);
      }
    }
  }
</script>

<style>
</style>
