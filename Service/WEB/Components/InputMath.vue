<template>
  <table class="vartable">
    <tr>
      <td spellcheck="false">
        <textarea class="hl_textarea" type="text" :value="value" @input="$emit('input', $event.target.value)"></textarea>
      </td>
    </tr>
    <tr>
      <td class="first-row">
        <div class="btn-group" v-if="this.$listeners.evaluate">
          <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Reload variables and evaluate" @click="$emit('evaluate')"><img :src="require('Imgs/evaluate_formula.png')"></button>
        </div>
        <option-adder :icon="require('Imgs/math_functions.png')" :options="Functions" @add="Insert"></option-adder>
        <option-adder :icon="require('Imgs/sql_join_outer_exclude.png')" :options="Operators" @add="Insert"></option-adder>
        <toolbar-pager class="pull-right" :pageSize="10" :input="vars" :output.sync="FilterVars" :filtercols="['Name', 'Description', 'Value', 'Units']"></toolbar-pager>
      </td>
    </tr>
    <tr>
      <td class="last-row">
        <vuetable :fields="Columns" :api-mode="false" :data="FilterVars" class="table small table-striped">
          <template slot="actions" slot-scope="props">
            <div class="btn-group">
              <button type="button" class="btn btn-default btn-xs" @click="Insert(props.rowData.Name)"><img :src="require('Imgs/plus.png')" alt="Add variable"></button>
            </div>
          </template>
        </vuetable>
      </td>
    </tr>
  </table>
</template>

<script>
  import 'highlight-within-textarea/jquery.highlight-within-textarea.js';
  import 'highlight-within-textarea/jquery.highlight-within-textarea.css';
  import vuetable from 'vuetable-2';
  import 'CSS/table-responsive.css';
  export default {
    props: { value: String, vars: Array },
    data: function() {
      return { FilterVars: [],
               Functions: ['sin(A)', 'cos(A)', 'tan(A)', 'asin(A)', 'acos(A)', 'atan(A)', 'sinh(A)', 'cosh(A)', 'tanh(A)', 'asinh(A)', 'acosh(A)', 'atanh(A)', 'log2(A)', 'log10(A)', 'log(A)', 'ln(A)', 'exp(A)', 'sqrt(A)', 'sign(A)', 'rint(A)', 'abs(A)', 'min(A, B, ...)', 'max(A, B, ...)', 'sum(A, B, ...)', 'avg(A, B, ...)'],
               Operators:['&&', '||', '<=', '>=', '!=', '==', '>', '<', '+', '-', '*', '/', '^', '(A==B) ? C : D'],
               Columns: [
                  { name: 'Name', title: 'Variable' },
                  { name: 'Description', title: 'Description' },
                  { name: 'Value', title: 'Value' },
                  { name: 'Units', title: 'Units' },
                  { name: '__slot:actions', title: '' }
                ]
      };
    },
    watch: {
      value: function() { this.$nextTick(() => { this.Refresh(); }); },
      vars: function() { this.Refresh(); }
    },
    mounted: function() {
      this.$emit('evaluate');
      this.Refresh();
    },
    beboreDestroy: function() {
      $('.hl_textarea').highlightWithinTextarea('destroy');
    },
    methods: {
      ValueFormat: function(val) {
        return this.Round(val, 2, false);
      },
      Insert: function(text) {
        var txt = $('.hl_textarea');
        var caretPos = txt[0].selectionStart;
        var textAreaTxt = txt.val();
        $(txt).focus();
        this.$emit('input', textAreaTxt.substring(0, caretPos) + text + textAreaTxt.substring(caretPos));
      },
      Refresh: function() {
        var  Vars = [];
        for(var n in this.vars) Vars.push(this.vars[n].Name);
        $('.hl_textarea').highlightWithinTextarea({
          highlight: [
            { highlight: RegExp('\\b' + Vars.join('|') + '\\b', 'g'), className: 'variable' },
            { highlight: ['=', '&&', '||', '<=', '>=', '!=', '==', '>', '<', '+', '-', '*', '/', '^', '?', ':'], className: 'operator' },
            { highlight: RegExp('\\(|\\)|\\b(sin|cos|tan|asin|acos|atan|sinh|cosh|tanh|asinh|acosh|atanh|log2|log10|log|ln|exp|sqrt|sign|rint|abs|min|max|sum|avg)\\b', 'g'), className: 'function' },
            { highlight: RegExp('\\b([0-9]*[.])?[0-9]+\\b', 'g'), className: 'numbers' }
          ]
        });
        $('.hl_textarea').focus();
      }
    },
    components: { vuetable }
  }
</script>

<style scoped>
  .vartable {
    width: 100%;
    border: 1px solid #ccc;
  }
  .vartable .first-row {
    background: #eee;
    padding: 2px;
    border: 1px solid #ccc;
  }
  .vartable .last-row {
    border: 1px solid #ccc;
    padding: 0px;
  }

  .vartable >>> .hwt-container {
    width: 100%;
    margin-bottom: -5px;
  }
  .vartable >>> .hwt-input {
    resize: none;
    height: 106px;
    caret-color: #555;
    color: rgba(0, 0, 0, 0);
  }
  .vartable >>> .hwt-highlights {
    color: #555 !important;
  }
  .vartable >>> .hwt-content {
    margin: 0px;
    display: block;
    width: 100%;
    padding: 6px;
    font-size: 14px;
    line-height: 18px;
    background-color: #fff;
    border: 0px solid #ccc;
    font-family: monospace;
    outline: 0;
  }
  .vartable >>> mark {
    font-family: monospace;
    font-size: 14px;
    background: #FFF;
  }
  .vartable >>> mark.variable {
    color: #d9534f;
  }
  .vartable >>> mark.operator {
    color: #f0ad4e;
  }
  .vartable >>> mark.function {
    color: #428bca;
    font-style: italic;
  }
  .vartable >>> mark.numbers {
    color: #5cb85c;
  }
  @media screen and (min-width: 600px) {
    .table >>> th:nth-child(3) {
      width: 100px;
      text-align: right;
    }
    .table >>> td:nth-child(3) {
      text-align: right;
    }
    .table >>> th:nth-child(4) {
      width: 20px;
    }
    .table >>> th:nth-child(5) {
      width: 30px;
    }
  }
</style>