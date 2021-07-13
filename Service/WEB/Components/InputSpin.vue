<template>
  <div class="input-group inputspin small">
    <div class="input-group-btn" v-if="list != null">
      <button type="button" :disabled="disabled" class="btn btn-default dropdown-toggle btn-xs" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false"><span class="caret"></span></button>
      <ul class="dropdown-menu">
        <li v-for="item in list" :key="item.name"><a @click="$emit('input', item.value)">{{ item.name }}</a></li>
      </ul>
    </div>
    <div class="input-group-btn" v-if="$slots['buttons']">
      <slot name="buttons"></slot>
    </div>
    <input type="text" class="form-control" :disabled="disabled" :value="Format(value)" @change="Change($event.target.value)">
    <span v-if="prepend" class="input-group-addon">{{ prepend }}</span>
    <div class="input-group-btn-vertical">
      <button class="btn btn-default" type="button" :disabled="disabled || (value + increment) > max" @click="$emit('input', value + increment)"><div>&#9650;</div></button>
      <button class="btn btn-default" type="button" :disabled="disabled || (value - increment) < min" @click="$emit('input', value - increment)"><div>&#9660;</div></button>
    </div>
  </div>
</template>

<script>
  export default {
    props: { prepend: String, min: Number, max: Number, increment: Number, value: Number, decimals: Number, list: Array, disabled: Boolean },
    data: function() {
      return { };
    },
    methods: {
      Change: function(val) {
        var Value = parseFloat(eval(val));
        if(isNaN(Value) === true) Value = this.min;
        if(Value > this.max) Value = this.max;
        if(Value < this.min) Value = this.min;
        this.$emit('change', Value);
        this.$emit('input', Value);
      },
      Format: function(val) {
        var Exponent = Math.pow(10, this.decimals);
        val = Math.round(val * Exponent);
        var Integer = Math.trunc(val / Exponent);
        var Fraction = Math.abs(val % Exponent);
        Fraction = Fraction.toString();
        while(Fraction.length < this.decimals) Fraction = '0' + Fraction;
        if(this.decimals > 0) {
          return (((Integer == 0) && (val < 0.0)) ? '-' : '') + Integer.toString() + '.' + Fraction;
        } else {
          return (((Integer == 0) && (val < 0.0)) ? '-' : '') + Integer.toString();
        }
      }
    }
  }
</script>

<style scoped>
  .small .form-control, .small .input-group-addon {
    height: 22px;
    padding: 2px 5px;
    font-size: 12px;
  }
  .small .input-group-btn-vertical .btn-default {
    height: 12px;
  }
  .small .input-group-btn-vertical .btn-default > div {
    font-size: 10px;
    overflow: hidden;
    margin-top: -3px;
  }
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