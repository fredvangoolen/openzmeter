<template>
  <div class="btn-group">
    <div class="input-group input-group-xs">
      <span class="input-group-addon"><img :src="require('Imgs/calendar_view_day.png')"></span>
      <div class="input-group-btn">
        <button class="btn btn-default dropdown-toggle" :class="{disabled: (weeks.length == 0)}" type="button" data-toggle="dropdown"><span>{{ value }}</span><span class="caret"></span></button>
        <ul class="dropdown-menu pull-right">
          <li v-for="(week, id) in weeks" :class="{ active: (id == value) }" :key="id"><a @click="$emit('input', id)">
            <img v-if="week.Warning==true" :src="require('Imgs/exclamation.png')" data-toggle="tooltip" title="Limit exceded">
            <div v-if="week.Warning==false"></div>
            {{ id }}</a></li>
        </ul>
      </div>
    </div>
  </div>
</template>

<script>
  import moment from 'moment';
  export default {
    props: { weeks: Object, value: String },
    data() {
      return { };
    },
    methods: {
      WeekClick: function(week, event) {
        event.preventDefault();
        this.$emit('input', week);
      }
    }
  }
</script>

<style scoped>
  li > a > img {
    height: 12px;
    width: 12px;
    margin-right: 2px;
    vertical-align: baseline;
  }
  li > a > div {
    width: 16px;
    display: inline-block;
  }
  .btn-group ul {
    max-height: 142px;
    overflow-y: auto;
  }
</style>
