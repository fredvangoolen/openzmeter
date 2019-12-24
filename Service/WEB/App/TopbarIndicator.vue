<template>
  <div class="indicator" v-bind:style="{ 'background-color': Color, 'border-color': Color }" style="border-width: 1px; border-style: solid">
    <div>
      <div>
        <span v-for="val in Value" v-html="val"></span>
      </div>
      <div>{{ Prefix }}{{ Units }}<br><span style="font-size: 10px; font-weight: normal; font-stretch: condensed">{{ AggregType }}</span></div>
    </div>
  </div>
</template>
 
<script>
  import * as oZm from 'JS/openZmeter.js'
  export default {
    props: ['value', 'serie', 'grouped'],
    data: function() {
      return { Color: oZm.Series[this.serie].Color, Value: ['Offline'], Units: oZm.Series[this.serie].Units, Prefix: '', AggregType: '' };
    },
    watch: {
      value: function() {
        this.SetValue();
      },
      grouped: function() {
        this.SetValue();
      }
    },
    methods: {
      SetValue: function() {
        if(this.value === null) {
          this.Value = ['Offline'];
          this.AggregType = '';
        } else if(Array.isArray(this.value) == false) {
          this.Prefix = oZm.GetBestUnits(Math.abs(this.value));
          this.Value = [oZm.RoundHTML(oZm.ScaleToUnits(this.value, this.Prefix), oZm.Series[this.serie].Decimals)];
          this.AggregType = '';          
        } else if(this.value.length == 1) {
          this.Prefix = oZm.GetBestUnits(Math.abs(this.value[0]));
          this.Value = [oZm.RoundHTML(oZm.ScaleToUnits(this.value[0], this.Prefix), oZm.Series[this.serie].Decimals)];
          this.AggregType = '';
        } else if(this.grouped == true) {
          var Aggreg = 0.0;
          this.AggregType = '';
          if((this.serie == 'VOLTAGE') || (this.serie == 'CURRENT')) {
            var PhasesCount = oZm.Series['VOLTAGE'].Names(this.$root.SelectedAnalyzer).length;
            for(var n = 0; n < PhasesCount; n++) Aggreg += Math.pow(this.value[n], 2.0);
            this.AggregType = "RMS";
            Aggreg = Math.sqrt(Aggreg / PhasesCount);
          } else if((this.serie == 'ACTIVE_POWER') || (this.serie == 'ACTIVE_ENERGY')) {
            this.AggregType = "SUM";
            for(var i in this.value) Aggreg += this.value[i];
          }
          this.Prefix = oZm.GetBestUnits(Aggreg);
          Aggreg = oZm.ScaleToUnits(Aggreg, this.Prefix);
          this.Value = ['<strong>' + oZm.RoundHTML(Aggreg, this.Decimals) + '</strong>'];
        } else {
          this.Prefix = oZm.GetBestUnits(oZm.ArrayAbsMax(this.value));
          this.AggregType = '';
          var Values = oZm.ScaleArrayToUnits(this.value, this.Prefix);
          this.Value = [];
          for(var i = 0; i < Values.length; i++) this.Value.push(oZm.RoundHTML(Values[i], oZm.Series[this.serie].Decimals));
        }
      }      
    }
  }
</script>
 
<style scoped>
  @media(min-width:768px) {
    .indicator > div > div:first-child {
      font-size: 12px;
    }
  }
  .indicator {
    border-radius: 4px;
    display: inline-block;
    height: 50px;
    margin: 0px;
  }
  .indicator > div {
    display: table;
    height: 48px;
  }
  .indicator > div > div {
    display: table-cell;
    vertical-align: middle;
    padding: 0 4px;
    text-align: center;
  }
  .indicator > div > div:first-child {
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
  .indicator > div > div > span {
    display: block;
  }
</style>