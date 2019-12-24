<template>
  <span>
    <div class="btn-group">
      <div class="input-group input-group-xs">
        <span class="input-group-addon"><img :src="require('Imgs/clock_15.png')"></span>
        <div class="input-group-btn">
          <button class="btn btn-default dropdown-toggle" type="button" data-toggle="dropdown">{{ Selected.name }}<span class="caret"></span></button>
          <ul class="dropdown-menu pull-left">
            <li v-for="(option, idx) in Options" v-bind:class="{active: (value == option.value)}" :key="idx"><a v-on:click="$emit('input', option.value)">{{ option.name }}</a></li>
          </ul>
        </div>
      </div>
    </div>
    <div class="modal" tabindex="-1" role="dialog" data-backdrop="false" ref="ErrorDialog" v-if="showDialog">
      <div class="modal-backdrop in"></div>
      <div class="modal-dialog">
        <div class="modal-content">
          <div class="modal-header">
            <h4 class="modal-title">Aggregation change</h4>
            <div class="pull-right">
              <button type="button" class="btn btn-default btn-xs" @click="showDialog = false"><img :src="require('Imgs/arrow_undo.png')"></button>
            </div>
          </div>
          <div class="modal-body">
            <p>Requested aggregation returns too much points.<br/>Zoom in and try again.</p><p>Aggregation period has been changed to the lowest allowed.</p>
          </div>
        </div>
      </div>
    </div>
  </span>
</template>

<script>
  export default {
    props: { value: {type: String, default: '1H' }, options: { type: Array, default: [] }, storage: { type: String, default: null } },
    data() {
      return { Options: [], showDialog: false, Selected: {} };
    },
    created: function() {
      if(this.options.includes('3S'))  this.Options.push({value: '3S',  name: '3 seconds'});
      if(this.options.includes('1M'))  this.Options.push({value: '1M',  name: '1 minute'});
      if(this.options.includes('10M')) this.Options.push({value: '10M', name: '10 minutes'});
      if(this.options.includes('15M')) this.Options.push({value: '15M', name: '15 minutes'});
      if(this.options.includes('1H'))  this.Options.push({value: '1H',  name: '1 hour'});
      if(this.storage != null) {
        var Stored = localStorage.getItem(this.storage);
        if(this.options.includes(Stored)) {
          this.$emit('input', Stored);
        } else {
          this.$emit('input', this.Options[0].value);
        }
      }
    },
    watch: {
      value: function(val) {
        if((this.storage != null) && (val != null)) localStorage.setItem(this.storage, val);
        for(var idx in this.Options) {
          if(this.Options[idx].value !== val) continue;
          this.Selected = this.Options[idx];
          return;
        }
      }
    },
    methods: {
//      AggregClick(aggreg, event) {
//        event.preventDefault();
//        this.Selected = aggreg;
//        this.SelectedText = $(event.target).text();
//        localStorage.setItem(this.variable, aggreg);
//        this.$emit('AggregationChanged', this.Selected);
//      },
      AggregationFail() {
        this.showDialog = true;
        this.$nextTick(function() { $(this.$refs.ErrorDialog).modal('show'); });
        var i = this.options.indexOf(this.Selected);
        if(this.options[i + 1] !== undefined) this.Selected = this.options[i + 1];
        this.SetAggregation();        
      },
//      SetAggregation() {
//        if(this.options.includes(this.Selected) !== true) this.Selected = this.options[0];
//        if(this.Selected === '3S') {
//          this.SelectedText = '3 seconds';
//        } else if(this.Selected === '1M') {
//          this.SelectedText = '1 minute';
//        } else if(this.Selected === '10M') {
//          this.SelectedText = '10 minutes';
//        } else if(this.Selected === '15M') {
//          this.SelectedText = '15 minutes';
//        } else if(this.Selected === '1H') {
//          this.SelectedText = '1 hour';
//        } else {
//          this.Selected = '1H';
//          this.SelectedText = '1 hour';
//        }
//        localStorage.setItem(this.variable, this.Selected);
//        this.$emit('AggregationChanged', this.Selected);
//      }
    }
  }
</script>

<style>
  .dropdown-menu {
    z-index: 10000;
  }
  .modal-dialog {
    z-index: 1100;
    color: #333;
    font-size: 14px;
    text-align: left;
    margin-top: 70px;
  }
  .modal-title {
    float: left;
  }
</style>
