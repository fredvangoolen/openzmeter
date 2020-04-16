<template>
  <div class="input-math" spellcheck="false">
    <textarea ref="inputbox" type="text"></textarea>
  </div>
</template>

<script>
  import 'highlight-within-textarea/jquery.highlight-within-textarea.js';
  import 'highlight-within-textarea/jquery.highlight-within-textarea.css';

  export default {
    props: {  },
    data: function() {
      return { };
    },
    mounted: function() {
    $(this.$refs.inputbox).highlightWithinTextarea({
      highlight: [
       { highlight: RegExp('([0-9]*[.])?[0-9]+'), className: 'numbers' },
       { highlight: ['Voltage', 'Current'], className: 'variable' },
       { highlight: ['=', '&&', '||', '<=', '>=', '!=', '==', '>', '<', '+', '-', '*', '/', '^', '?', ':'], className: 'operator' },
       { highlight: ['(', ')', 'sin', 'cos', 'tan', 'asin', 'acos', 'atan', 'sinh', 'cosh', 'tanh', 'asinh', 'acosh', 'atanh', 'log2', 'log10', 'log', 'ln', 'exp', 'sqrt', 'sign', 'rint', 'abs', 'min', 'max', 'sum', 'avg'], className: 'function' },

      ]
});
    }


  }
</script>

<style scoped>
  .input-math >>> .hwt-container {
    margin: -5px;
    padding: 5px;
    width: 100%;
    box-sizing: content-box;
  }
  .input-math >>> .hwt-input {
    resize: none;
    height: 100px;
    caret-color: #555;
    color: rgba(0, 0, 0, 0);
  }
  .input-math >>> .hwt-highlights {
    color: #ddd !important;
  }
  .input-math >>> .hwt-backdrop {
    margin: 5px;
  }
  .input-math >>> .hwt-content {
    display: block;
    width: 100%;
    padding: 6px 12px;
    font-size: 14px;
    line-height: 18px;
    background-color: #fff;
    background-image: none;
    border: 1px solid #ccc;
    border-radius: 4px;
    -webkit-box-shadow: inset 0 1px 1px rgba(0,0,0,.075);
    box-shadow: inset 0 1px 1px rgba(0,0,0,.075);
    -webkit-transition: border-color ease-in-out .15s,-webkit-box-shadow ease-in-out .15s;
    -o-transition: border-color ease-in-out .15s,box-shadow ease-in-out .15s;
    transition: border-color ease-in-out .15s,box-shadow ease-in-out .15s;
    font-family: monospace;
  }
  .input-math >>> .hwt-content:focus {
    border-color: #66afe9;
    outline: 0;
    -webkit-box-shadow: inset 0 1px 1px rgba(0, 0, 0, .075), 0 0 8px rgba(102, 175, 233, 0.6);
    box-shadow: inset 0 1px 1px rgba(0, 0, 0, .075), 0 0 8px rgba(102, 175, 233, 0.6);
  }
  .input-math >>> mark {
    font-family: monospace;
    font-size: 14px;
    background: #FFF;
  }
  .input-math >>> mark.error {
    text-decoration-line: underline;
    text-decoration-style: dotted;
    text-decoration-color: #d9534f;
  }
  .input-math >>> mark.variable {
    color: #e6550d;
  }
  .input-math >>> mark.operator {
    color: #333;
  }
  .input-math >>> mark.function {
    color: #5799d0;
    font-style: italic;
  }
  .input-math >>> mark.numbers {
    color: #31a354;
  }
</style>