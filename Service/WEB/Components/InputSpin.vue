<template>
  <div class="input-group inputspin">
    <div class="input-group-btn" v-if="list != null">
      <button type="button" class="btn btn-default dropdown-toggle" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false"><span class="caret"></span></button>
      <ul class="dropdown-menu">
        <li v-for="item in list"><a v-on:click="SetValue(item.Value)">{{ item.Text }}</a></li>
      </ul>
    </div>
    <input type="text" class="form-control" :value="Format(Value)" v-on:change="Change">
    <span v-if="prepend" class="input-group-addon" id="basic-addon1">{{ prepend }}</span>
    <div class="input-group-btn-vertical">
      <button class="btn btn-default" type="button" v-on:click="Increment">&#9650;</button>
      <button class="btn btn-default" type="button" v-on:click="Decrement">&#9660;</button>
    </div>    
  </div>
</template>

<script>
  export default {
    props: {prepend: { type: String }, min: { type: Number, default: 0}, max: { type: Number, default: 100}, increment: { type: Number, default: 0.1 }, value: { type: Number, default: 50}, decimals: { type: Number, default: 2 }, list: { type: Array, default: null} },
    data: function() {
      return { Value: 50 };
    },
    created: function() {
      this.Value = this.value;
    },
    methods: {
      SetValue: function(val) {
        this.Value = parseFloat(val);
        if(this.Value > this.max) this.Value = this.max;
        this.$emit('change', this.Value);        
      },
      Increment: function(evt) {
        $(evt.target).blur();
        this.Value = parseFloat(this.Value) + this.increment;
        if(this.Value > this.max) this.Value = this.max;
        this.$emit('change', this.Value);
      },
      Decrement: function(evt) {
        $(evt.target).blur();
        this.Value = parseFloat(this.Value) - this.increment;
        if(this.Value < this.min) this.Value = this.min;
        this.$emit('change', this.Value);
      },
      Change: function(evt) {
        var Value = evt.target.value;
        if(isNaN(parseFloat(Value)) === true) Value = this.min;
        if(Value > this.max) Value = this.max;
        if(Value < this.min) Value = this.min;
        this.Value = Value;
        this.$emit('change', this.Value);
      },
      Format: function(val) {
        var Exponent = Math.pow(10, this.decimals);
        val = Math.round(val * Exponent);
        var Integer = Math.trunc(val / Exponent);
        var Fraction = Math.abs(val % Exponent);
        Fraction = Fraction.toString();
        while(Fraction.length < this.decimals) Fraction = '0' + Fraction;
        return Integer.toString() + '.' + Fraction;
      }
    }    
  }
</script>

<style scoped>
  .inputspin input {
    text-align: right;
  }
  .inputspin > span {
    border-left: none;
  }
  .input-group-btn-vertical {
    position: relative;
    white-space: nowrap;
    width: 2%;
    vertical-align: middle;
    display: table-cell;
  }
  .input-group-btn-vertical > .btn {
    display: block;
    float: none;
    width: 100%;
    max-width: 100%;
    padding: 2px 4px;
    margin-left: -1px;
    position: relative;
    border-radius: 0;
    font-size: 10px;
    line-height: 12px;
  }
  .input-group-btn-vertical > .btn:first-child {
    border-top-right-radius: 4px;
  }
  .input-group-btn-vertical > .btn:last-child {
    margin-top: -2px;
    border-bottom-right-radius: 4px;
  }
  .input-group-btn-vertical i {
    position: absolute;
    top: 0;
    left: 4px;
  }
</style>