<template>
  <div class="indicator" v-bind:style="{ 'background-color': analyzer.Color[serie], 'border-color': analyzer.Color[serie] }">
    <div>
      <div>
        <span v-for="(val, idx) in Value.Value" v-html="val" :key="idx"></span>
      </div>
      <div>{{ Value.Units }}{{ analyzer.Unit[serie] }}<br><span style="font-size: 10px; font-weight: normal; font-stretch: condensed">{{ Value.Aggreg }}</span></div>
    </div>
  </div>
</template>
 
<script>
  export default {
    props: { analyzer: Object, value: Object, serie: String, grouped: Boolean },
    data: function() {
      return { };
    },
    computed: {
      Value: function() {
        var Val = { Value: ['Offline'], Aggreg: '', Units: '' };
        if((this.value == null) || (this.value[this.serie] == null)) {
        } else if(Array.isArray(this.value[this.serie]) == false) {
          Val.Units = this.BestUnits(Math.abs(this.value[this.serie]));
          Val.Value = [this.Round(this.ScaleUnits(this.value[this.serie], Val.Units), 3, true)];
        } else if(this.value[this.serie].length == 1) {
          Val.Units = this.BestUnits(Math.abs(this.value[this.serie][0]));
          Val.Value = [this.Round(this.ScaleUnits(this.value[this.serie][0], Val.Units), 2, true)];
        } else if(this.grouped == true) {
          var Aggreg = 0.0;
          if((this.serie == 'Voltage') || (this.serie == 'Current')) {
            Val.Aggreg = "RMS";
            for(var n = 0; n < this.analyzer.VoltageChannels; n++) Aggreg += Math.pow(this.value[serie][n], 2.0);
            Aggreg = Math.sqrt(Aggreg / this.analyzer.VoltageChannels);
          } else if((this.serie == 'Active_Power') || (this.serie == 'Active_Energy')) {
            Val.Aggreg = "SUM";
            for(var i in this.value[this.serie]) Aggreg += this.value[this.serie][i];
          }
          Val.Units = this.BestUnits(Aggreg);  
          Val.Value = ['<strong>' + this.Round(this.ScaleUnits(Aggreg, Val.Units), 2, true) + '</strong>'];
        } else {
          var Max = 0;
          for(var i in this.value[this.serie]) Max = Math.max(Max, Math.abs(this.value[this.serie][i]));
          Val.Units = this.BestUnits(Max);
          Val.Value = [];
          for(var i in this.value[this.serie]) Val.Value.push(this.Round(this.ScaleUnits(this.value[this.serie][i], Val.Units), 2, true));
        }
        return Val;
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
    border-width: 1px; 
    border-style: solid
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