<template>
  <div :class="color">
    <div>
      <div>
        <span v-for="val in value" v-html="val"></span>
      </div>
      <div>{{ prefix }}{{ units }}</div>
    </div>
  </div>
</template>
 
<script>
  import * as oZm from 'JS/openZmeter.js'
  export default {
    props: ['color', 'units', 'decimals'],
    data: function() {
      return { value: ['Offline'], prefix: '', Decimals: 2 };
    },
    created: function() {
      if(this.decimals !== undefined) this.Decimals = this.decimals;
      this.Group = false;
    },
    methods: {
      setValue: function(vals) {
        if(vals === null) {
          this.value = ['Offline'];
        } else {
          if(vals.length == 1) this.Group = false;
          if(this.Group == true) {
            var Aggreg = 0.0;
            var AggregType;
            if((this.units == 'V') || (this.units == 'A')) {
              var PhasesCount = oZm.Series['VOLTAGE'].Names(this.$root.SelectedAnalyzer).length;
              for(var n = 0; n < PhasesCount; n++) Aggreg += Math.pow(vals[n], 2.0);
              AggregType = "RMS";
              Aggreg = Math.sqrt(Aggreg / PhasesCount);
            } else if(this.units == 'W') {
              AggregType = "SUM";
              for(var val in vals) Aggreg += vals[val];
            }
            this.prefix = oZm.GetBestUnits(Aggreg);
            Aggreg = oZm.ScaleToUnits(Aggreg, this.prefix);
            this.value = [oZm.RoundHTML(Aggreg, this.Decimals), "&nbsp;", '<div style="text-align:center">Phases<br>' + AggregType + '</div>'];
          } else {
            this.prefix = oZm.GetBestUnits(oZm.ArrayAbsMax(vals));
            vals = oZm.ScaleArrayToUnits(vals, this.prefix);
            this.value = [];
            for(var i = 0; i < vals.length; i++) this.value.push(oZm.RoundHTML(vals[i], this.Decimals));
          }
          this.Group = !this.Group;
        }
      }
    }
  }
</script>
 
<style>
  .indicator-bar {
    background-image: linear-gradient(to bottom,#f3f3f3,#e2e2e2);
    text-align: center;
    font-size: 14px;
    color: #FFF;
    font-weight: 700;
    padding: 2px;
    border-bottom: 1px solid #c9c9c9;
    position: relative;
    z-index: 95;
    font-size: 18x;
    top: 0px;
  }
  @media(min-width:640px) {
    .indicator-bar {
      background-image: none;
      position: absolute;
      right: 48px;
      text-align: right;
      border: none;
    }
  }
  @media(min-width:1280px) {
    .indicator-bar {
      right: 2px;
    }
  } 
  @media(min-width:768px) {
    .indicator-bar > div > div > div:first-child {
      font-size: 12px;
    }
  }
  .indicator-bar > div {
    border-radius: 4px;
    display: inline-block;
    height: 50px;
  }
  .indicator-bar > div > div {
    display: table;
    height: 48px;
  }
  .indicator-bar > div > div > div {
    display: table-cell;
    vertical-align: middle;
    padding: 0 4px;
    text-align: center;
  }
  .indicator-bar > div > div > div:first-child {
    text-align: right;
    background-color: #fff;
    color: #333;
    border-radius: 4px;
    font-size: 11px;
    font-weight: normal;
    line-height: 10px;
    padding: 2px;
    min-width: 44px;
  }
  .indicator-bar > div > div > div > span {
    display: block;
  }
</style>