<template>
  <div class="btn-group">
    <div class="input-group input-group-xs">
      <span class="input-group-addon"><img :src="require('Imgs/time_go.png')"></span>
      <div class="input-group-btn">
        <button class="btn btn-default dropdown-toggle" type="button" data-toggle="dropdown">{{ SelectedText }}<span class="caret"></span></button>
        <ul class="dropdown-menu pull-right">
          <li :class="{active: (value == 'LastMinute')}"><a @click="$emit('input', 'LastMinute')">Last minute</a></li>
          <li :class="{active: (value == 'Last5Minutes')}"><a @click="$emit('input', 'Last5Minutes')">Last 5 minutes</a></li>
          <li :class="{active: (value == 'Last10Minutes')}"><a @click="$emit('input', 'Last10Minutes')">Last 10 minutes</a></li>
          <li :class="{active: (value == 'Last15Minutes')}"><a @click="$emit('input', 'Last15Minutes')">Last 15 minutes</a></li>
          <li :class="{active: (value == 'Last30Minutes')}"><a @click="$emit('input', 'Last30Minutes')">Last 30 minutes</a></li>
          <li :class="{active: (value == 'LastHour')}"><a @click="$emit('input', 'LastHour')">Last hour</a></li>
        </ul>
      </div>
    </div>
  </div>
</template>

<script>
  import moment from 'moment';
  export default {
    props: { storage: String, value: String, analyzer: Object },
    data: function () {
      return { Value: null, Range: null };
    },
    watch: {
      value: function(val) {
        if(this.storage != null) localStorage.setItem(this.storage, JSON.stringify(val));
        this.Range = this.analyzer.CalcRange(val);
      }
    },
    created: function () {
      if(this.storage != null) {
        var Stored;
        try {
          Stored = JSON.parse(localStorage.getItem(this.storage));
        } catch(e) {
          Stored = 'LastMinute';
        }
        if((Stored != 'LastMinute') && (Stored != 'Last5Minutes') && (Stored != 'Last10Minutes') && (Stored != 'Last15Minutes') && (Stored != 'Last30Minutes') && (Stored != 'LastHour')) {
          Stored = 'LastMinute';
        }
        if(this.value != Stored) this.$emit('input', Stored);
      } else if(this.value == null) {
        this.$emit('input', 'LastMinute');
      }
    },
    computed: {
      SelectedText: function() {
        if(this.value == 'LastMinute') return 'Last minute';
        if(this.value == 'Last5Minutes') return 'Last 5 minutes';
        if(this.value == 'Last10Minutes') return 'Last 10 minutes';
        if(this.value == 'Last15Minutes') return 'Last 15 minutes';
        if(this.value == 'Last30Minutes') return 'Last 30 minutes';
        if(this.value == 'LastHour') return 'Last hour';
        return 'Custom';
      }
    }
  }
</script>

<style scoped>
  .input-group-btn .narrow {
    padding: 1px 0px;
  }
</style>