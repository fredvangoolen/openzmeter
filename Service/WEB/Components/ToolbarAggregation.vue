<template>
  <span>
    <div class="btn-group">
      <div class="input-group input-group-xs">
        <span class="input-group-addon"><img :src="require('Imgs/clock_15.png')"></span>
        <div class="input-group-btn">
          <button class="btn btn-default dropdown-toggle" type="button" data-toggle="dropdown">{{ SelectedText }}<span class="caret"></span></button>
          <ul class="dropdown-menu pull-right">
            <li v-if="options.includes('3S')" v-bind:class="{active: (Selected == '3S')}"><a @click="AggregClick('3S', $event)" href="#">3 seconds</a></li>
            <li v-if="options.includes('1M')" v-bind:class="{active: (Selected == '1M')}"><a @click="AggregClick('1M', $event)" href="#">1 minute</a></li>
            <li v-if="options.includes('10M')" v-bind:class="{active: (Selected == '10M')}"><a @click="AggregClick('10M', $event)" href="#">10 minutes</a></li>
            <li v-if="options.includes('15M')" v-bind:class="{active: (Selected == '15M')}"><a @click="AggregClick('15M', $event)" href="#">15 minutes</a></li>
            <li v-if="options.includes('1H')" v-bind:class="{active: (Selected == '1H')}"><a @click="AggregClick('1H', $event)" href="#">1 hour</a></li>
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
            <p>Requested aggregation returns to much points.<br/>Zoom in and try again.</p><p>Aggregation period has been changed to the lowest allowed.</p>
          </div>
        </div>
      </div>
    </div>
  </span>
</template>

<script>
  export default {
    props: { options: Array, variable: String },
    data() {
      return { showDialog: false, Selected: '1H', SelectedText: '1 hour' };
    },
    methods: {
      AggregClick(aggreg, event) {
        event.preventDefault();
        this.Selected = aggreg;
        this.SelectedText = $(event.target).text();
        localStorage.setItem(this.variable, aggreg);
        this.$emit('AggregationChanged', this.Selected);
      },
      AggregationFail() {
        this.showDialog = true;
        this.$nextTick(function() { $(this.$refs.ErrorDialog).modal('show'); });
        var i = this.options.indexOf(this.Selected);
        if(this.options[i + 1] !== undefined) this.Selected = this.options[i + 1];
        this.SetAggregation();        
      },
      SetAggregation() {
        if(this.options.includes(this.Selected) !== true) this.Selected = this.options[0];
        if(this.Selected === '3S') {
          this.SelectedText = '3 seconds';
        } else if(this.Selected === '1M') {
          this.SelectedText = '1 minute';
        } else if(this.Selected === '10M') {
          this.SelectedText = '10 minutes';
        } else if(this.Selected === '15M') {
          this.SelectedText = '15 minutes';
        } else if(this.Selected === '1H') {
          this.SelectedText = '1 hour';
        } else {
          this.Selected = '1H';
          this.SelectedText = '1 hour';
        }
        this.$emit('AggregationChanged', this.Selected);
      }
    },
    mounted: function() {
      this.Selected = localStorage.getItem(this.variable);
      this.SetAggregation();
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
