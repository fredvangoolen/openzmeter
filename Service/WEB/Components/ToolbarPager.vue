<template>
  <div class="btn-group">
    <div class="input-group input-group-xs">
      <div class="input-group-btn">
        <button class="btn btn-default narrow" type="button" @click="Previous"><img :src="require('Imgs/resultset_previous.png')"></button>
      </div>
      <div class="input-group-btn">
        <button class="btn btn-default dropdown-toggle" type="button" data-toggle="dropdown">{{ SelectedText }}<span class="caret"></span></button>
        <ul class="dropdown-menu pull-right" ref="periodTags">
          <li :class="{active: (value == 'LastHours')}"><a @click="$emit('input', 'LastHours')">Last 2 hour</a></li>
          <li :class="{active: (value == 'LastDay')}"><a @click="$emit('input', 'LastDay')">Last 24 hours</a></li>
          <li :class="{active: (value == 'LastWeek')}"><a @click="$emit('input', 'LastWeek')">Last 7 days</a></li>
          <li :class="{active: (value == 'LastMonth')}"><a @click="$emit('input', 'LastMonth')">Last 30 days</a></li>
          <li :class="{active: (value == 'LastYear')}"><a @click="$emit('input', 'LastYear')">Last 365 days</a></li>
          <li role="separator" class="divider"></li>
          <li :class="{active: (value == 'Day')}"><a @click="$emit('input', 'Day')">Today</a></li>
          <li :class="{active: (value == 'Week')}"><a @click="$emit('input', 'Week')">This week</a></li>
          <li :class="{active: (value == 'Month')}"><a @click="$emit('input', 'Month')">This month</a></li>
          <li :class="{active: (value == 'Year')}"><a @click="$emit('input', 'Year')">This year</a></li>
          <li role="separator" class="divider"></li>
          <li :class="{active: (Array.isArray(value))}"><a @click="EditPeriod()">Custom</a></li>
        </ul>
      </div>
      <div class="input-group-btn">
        <button class="btn btn-default narrow" type="button" @click="Next"><img :src="require('Imgs/resultset_next.png')"></button>
      </div>
    </div>
    <dlg ref="Dialog" v-if="ShowDialog">
      <template v-slot:header>
        <div class="pull-left"><h4 class="modal-title">Data range</h4></div>
        <div class="pull-right btn-group">
          <button type="button" class="btn btn-default btn-xs" @click="Apply"><img :src="require('Imgs/tick.png')" alt="Apply"></button>
          <button type="button" class="btn btn-default btn-xs" @click="ShowDialog = false"><img :src="require('Imgs/arrow_undo.png')" alt="Discard"></button>
        </div>
      </template>
      <div class="row">
        <div class="col-xs-12 col-sm-6"><label class="control-label">From</label><date-picker ref="StartDate" :options="{ format: 'DD/MM/YYYY - HH:mm:ss', viewMode: 'days' }" v-on:change="SetStartDate"></date-picker></div>
        <div class="col-xs-12 col-sm-6"><label class="control-label">To</label><date-picker ref="EndDate" :options="{ format: 'DD/MM/YYYY - HH:mm:ss', viewMode: 'days' }" v-on:change="SetEndDate"></date-picker></div>
      </div>
    </Dlg>
  </div>
</template>

<script>
  export default {
    props: { pageSize: Number, total: Number },
    data: function() {
      return { SelectedText: 'From 1 to 49 of 257'};
    }
  }
</script>

<style scoped>
  .input-group-btn .narrow {
    padding: 1px 0px;
  }
  .dropdown-toggle {
    border-radius: 0px;
  }  
</style>