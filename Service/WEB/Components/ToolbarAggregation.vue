<template>
  <span>
    <div class="btn-group">
      <div class="input-group input-group-xs">
        <span class="input-group-addon"><img :src="require('Imgs/clock_15.png')"></span>
        <div class="input-group-btn">
          <button class="btn btn-default dropdown-toggle" type="button" data-toggle="dropdown">{{ Selected }}<span class="caret"></span></button>
          <ul class="dropdown-menu pull-left">
            <li v-for="(option, idx) in Options" :class="{active: (value == option.value)}" :key="idx"><a v-on:click="$emit('input', option.value)">{{ option.name }}</a></li>
          </ul>
        </div>
      </div>
    </div>
    <dlg v-if="showDialog">
      <template v-slot:header>
        <h4 class="modal-title">Aggregation change</h4>
        <div class="pull-right">
          <button type="button" class="btn btn-default btn-xs" @click="showDialog=false"><img :src="require('Imgs/arrow_undo.png')"></button>
        </div>
      </template>
      <p>Requested aggregation returns too much points.<br/>Zoom in and try again.</p><p>Aggregation period has been changed to the lowest allowed.</p>
    </dlg>
  </span>
</template>

<script>
  export default {
    props: { value: String, options: Array, storage: String },
    data() {
      return { Options: [], showDialog: false };
    },
    created: function() {
      if(this.options.includes('3S'))  this.Options.push({value: '3S',  name: '3 seconds'});
      if(this.options.includes('1M'))  this.Options.push({value: '1M',  name: '1 minute'});
      if(this.options.includes('10M')) this.Options.push({value: '10M', name: '10 minutes'});
      if(this.options.includes('15M')) this.Options.push({value: '15M', name: '15 minutes'});
      if(this.options.includes('1H'))  this.Options.push({value: '1H',  name: '1 hour'});
      if(this.options.includes('1D'))  this.Options.push({value: '1D',  name: '1 day'});
      if(this.storage != null) {
        var Stored = localStorage.getItem(this.storage);
        if(this.options.includes(Stored)) {
          this.$emit('input', Stored);
        } else {
          this.$emit('input', this.Options[0].value);
        }
      }
    },
    computed: {
      Selected: function() {
        for(var idx in this.Options) {
          if(this.Options[idx].value == this.value) return this.Options[idx].name;
        }
      }
    },
    watch: {
      value: function(val, prev) {
        if((val == null) && (prev != null)) {
          this.showDialog = true;
          var i = this.options.indexOf(prev);
          if(this.options[i + 1] != null) this.$emit('input', this.options[i + 1]);
        }
        if((this.storage != null) && (val != null)) localStorage.setItem(this.storage, val);
      }
    }
  }
</script>

<style>
  .modal-title {
    float: left;
  }
</style>
