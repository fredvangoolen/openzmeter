<template>
  <dlg ref="Dialog">
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">Analyzers &raquo; Alarms &raquo; Edit</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" v-bind:class="{ disabled: (Error != '') }" v-on:click="Save"><img :src="require('Imgs/diskette.png')" alt="Save alarm"></button>
        <button type="button" class="btn btn-default btn-xs" v-bind:class="{ active: View=='Main' }" v-on:click="View='Main'">Main</button>
        <button type="button" class="btn btn-default btn-xs" v-bind:class="{ active: View=='Expression' }" v-on:click="View='Expression'; Evaluate()">Expression</button>
        <button type="button" class="btn btn-default btn-xs" v-on:click="Back"><img :src="require('Imgs/arrow_undo.png')" alt="Back"></button>
      </div>
    </template>
    <div class="alert alert-danger text-center" style="margin-bottom: 5px; padding: 5px" v-if="Error != ''">{{ Error }}</div>
    <div class="row" v-bind:style="{ display: (View=='Main') ? 'block' : 'none' }">
      <div class="col-xs-12">
        <label class="control-label">Name</label>
        <input type="text" class="form-control form-control-sm" v-model="Alarm.Name"/>
      </div>
      <div class="col-xs-12">
        <label class="control-label">Description</label>
        <textarea class="form-control" rows="2" v-model="Alarm.Description" style="resize: none"></textarea>
      </div>
      <div class="col-xs-12">
        <label class="control-label">Alarm message</label>
        <textarea class="form-control" rows="2" v-model="Alarm.Message" style="resize: none"></textarea>
      </div>
      <div class="col-xs-6">
        <label class="control-label">Priority</label>
        <option-picker :options="Priorities" :selected="Priorities[Alarm.Priority]" :big="true"></option-picker>
      </div>
      <div class="col-xs-6">
        <label class="control-label">Aggregation</label>
        <option-picker :options="['3 seconds', '1 minute', '10 minutes', '15 minutes', '1 hour']" selected="3 seconds" v-on:change="AggregationChanged" :big="true"></option-picker>
      </div>
      <div class="col-xs-6">
        <label class="control-label">Occurrences</label>
        <input-spin :min="1" :value="Alarm.Samples" :increment="1" :decimals="0"></input-spin>
      </div>
      <div class="col-xs-6">
        <label class="control-label">Window</label>
        <input-spin :min="1" :value="Alarm.Window" :increment="1" :decimals="0"></input-spin>
      </div>
    </div>
    <div class="row" v-bind:style="{ display: (View=='Expression') ? 'block' : 'none' }">
      <div class="col-xs-12">
        <label class="control-label">Expression</label>
        <div class="input-group">
          <span class="input-group-addon">
            <img v-if="EvalResult===true" :src="require('Imgs/exclamation.png')" data-toggle="tooltip" title="Alarm expression is valid and evaluate as triggered"/>
            <img v-if="EvalResult===false" :src="require('Imgs/tick.png')" data-toggle="tooltip" title="Alarm expression is valid and evaluate as not triggered!"/>
            <img v-if="EvalResult===null" :src="require('Imgs/cross.png')" data-toggle="tooltip" title="Alarm expression is invalid"/>
          </span>
          <input type="text" class="form-control" v-model="Alarm.Expression">
          <span class="input-group-btn">
            <button class="btn btn-default" data-toggle="tooltip" title="Evaluate" type="button" v-on:click="Evaluate"><img :src="require('Imgs/math_functions.png')"></button>
          </span>
        </div>
      </div>
      <div class="col-xs-12">
        <div style="width:100%; height: 200px; overflow-y: auto">
          <vuetable ref="table" :fields="columns" :api-mode="false" class="table small table-striped">
            <template slot="actions" slot-scope="props">
              <div class="btn-group">
                <button type="button" class="btn btn-primary btn-xs" v-on:click="Inser(props.rowData)">Insert</button>
              </div>
            </template>
          </vuetable>
        </div>
      </div>
    </div>
  </dlg>
</template>

<script>
//  import {Quasar, QBtn} from 'quasar';
  import vuetable from 'vuetable-2';
  import 'CSS/table-responsive.css';
  import Dlg from 'Components/Dialog.vue';
  import InputSpin from 'Components/InputSpin.vue';
  import OptionPicker from 'Components/OptionPicker.vue';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    data: function() {
      return { Analyzer: null, Alarm: {Aggreg: '3S'}, Error: '', EvalResult: null, View: 'Main', Priorities: ['LOW', 'NORMAL', 'HIGH'],
        columns: [
          { name: 'Name', title: 'Variable' },
          { name: 'Value', title: 'Value', callback: this.ValueFormat },
          { name: 'Units', title: 'Units' },
          { name: '__slot:actions', title: '' }
        ]
      };
    },
    methods: {
      AggregationChanged: function(val) {
        if(val == '3 seconds') {
          this.Alarm.Aggreg = '3S';
        }
      },
      ValueFormat: function(val) {
        return oZm.Round2Plain(val, 2);
      },
      Evaluate: function() {
        if(this.Alarm.Expression == undefined) this.Alarm.Expression = "";
        $.ajax({url: 'testAlarm', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json', data: JSON.stringify({ Analyzer: this.Analyzer.Analyzer, Aggreg: this.Alarm.Aggreg, Expression: this.Alarm.Expression }),
          success: (result) => {
            if(result.Error != undefined) {
              this.EvalResult = null;
              this.Error = result.Error;
            } else if(result.Result === 0.0) {
              this.EvalResult = false;
              this.Error = "";
            } else {
              this.EvalResult = true;
              this.Error = "";
            }
            this.$refs.table.setData({ data: result.Variables });
          },
          error: () => {
            this.FilterValues(null);              
          }
        });
      },
      FilterValues: function(result) {
        this.Values = {};
         if(result == null) return;    
        if(result.hasOwnProperty('Voltage') && (result.Voltage.length > 0)) this.Values.Voltage = (result.Voltage.slice(-1)[0].length == 1) ? result.Voltage.slice(-1)[0][0] : result.Voltage.slice(-1)[0];
        if(result.hasOwnProperty('Current') && (result.Current.length > 0)) this.Values.Current = result.Current.slice(-1)[0];
      },
      Validate: function(val) {
         try {
//        var Parser = math.parse(val.target.value);
//        Parser.compile();
//        this.Error = 'Evaluated to: ' + Parser.evaluate(this.Values);
      } catch (err) {
        this.Error = err.toString();
      }
//        window.MathJax = {
//				"fast-preview": {
//       					disabled: true
//				},
//				AuthorInit: function() {
//					MathJax.Hub.Register.StartupHook('End', function() {
//						MathJax.Hub.processSectionDelay = 0
//						var demoSource = document.getElementById('demoSource')
//						var demoRendering = document.getElementById('demoRendering')
//						var math = MathJax.Hub.getAllJax('demoRendering')[0]
//						demoSource.addEventListener('input', function() {
//							MathJax.Hub.Queue(['Text', math, demoSource.value])
//						})
//					})
//				}
//			}
	
        
        
      },
      Show: function(analyzer) {
        this.Analyzer = analyzer;
        this.$refs.Dialog.Show();
        this.$nextTick(() => {
//          this.loadBlockly();
        });
      },

      setAggregation(aggregation, event) {
        event.preventDefault();
        this.aggregation = {value: aggregation, text: $(event.target).text()};
      },
      Back() {
        this.$refs.Dialog.Close();
        this.$parent.$refs.Alarms.Show();
      },

      Save: function() {

        let code = Blockly.JavaScript.workspaceToCode(this.workspace);
        code = code.replace(/;/g, "").replace(/Math./g, "");
        alert(code);
      },

      loadBlockly() {

        $(document).off('focusin.modal');

        let toolbox = this.generateToolbox();

        this.workspace = Blockly.inject('blockly', {
          toolbox: toolbox,
          zoom: {startScale: 0.8}
        });
      },

      generateToolbox() {

        let toolbox = `<xml xmlns="https://developers.google.com/blockly/xml" id="toolbox" style="display: none">
                                  <block type="math_number">
                                    <field name="NUM">0</field>
                                  </block>
                                  <block type="logic_compare">
                                    <field name="OP">EQ</field>
                                  </block>
                                  <block type="math_trig">
                                    <field name="OP">SIN</field>
                                    <value name="NUM">
                                      <shadow type="math_number">
                                        <field name="NUM">45</field>
                                      </shadow>
                                    </value>
                                  </block>
                                  <block type="math_arithmetic">
                                    <field name="OP">ADD</field>
                                    <value name="A">
                                      <shadow type="math_number">
                                        <field name="NUM">1</field>
                                      </shadow>
                                    </value>
                                    <value name="B">
                                      <shadow type="math_number">
                                        <field name="NUM">1</field>
                                      </shadow>
                                    </value>
                                  </block>`;

        let blocks = {
          Voltage: "Voltage",
          Current: "Current",
          Frequency: "Frequency",
          Reactive_Power: "Reactive Power",
          Active_Power: "Active_Power"
        };


        let index = 0;
        Object.keys(blocks).forEach(k => {
          index++;
          let c = index * 20 + 5;
          Blockly.Blocks[k] = {
            init: function () {
              this.appendDummyInput()
                      .appendField(blocks[k]);
              this.setInputsInline(true);
              this.setOutput(true, "Number");
              this.setColour(c);
              this.setTooltip(blocks[k]);
              this.setHelpUrl(blocks[k]);
            }
          };
          Blockly.JavaScript[k] = function (block) {
            return [`${k}`, Blockly.JavaScript.ORDER_NONE];
          };

          toolbox += `<block type="${k}"></block>`;

        });


        return toolbox + '</xml>';
      }


    },
    components: { Dlg, InputSpin, OptionPicker, vuetable }
  }
</script>

<style scoped>
  .dropdown-menu {
    width: 240px;
  }
  .dropdown-submenu {
    position:relative;
  }
  .dropdown-submenu > .dropdown-menu { 
    top : 0;
    left: 100%;
    margin-top: -6px;
    margin-left: -1px;
    border-radius: 0 6px 6px 6px;
  }
  .dropdown-submenu:hover > .dropdown-menu{
    display:block;
  }
  .dropdown-submenu > a:after {
    display: block;
    content: " ";
    float: right;
    width:0;
    height:0;
    border-color: transparent;
    border-style: solid;
    border-width: 5px 0 5px 5px;
    border-left-color: #cccccc;
    margin-top: 5px;
    margin-right: -10px;
  }
  .dropdown-submenu:hover > a:after {
    border-left-color: #ffffff;
  }
  .dropdown-submenu.pull-left {
    float:none;
  }
  .dropdown-submenu.pull-left > .dropdown-menu { 
    left: -100%;
    margin-left:10px;
    border-radius:6px 0 6px 6px;
  }
  .menu-value {
    color: #666;
  }
  @media screen and (min-width: 600px) {
    .table >>> th:nth-child(2) {
      width: 100px;
      text-align: right;
    }
    .table >>> td:nth-child(2) {
      text-align: right;
    }
    .table >>> th:nth-child(3) {
      width: 20px;
    }
    .table >>> th:nth-child(4) {
      width: 30px;
    }
  }  
</style>