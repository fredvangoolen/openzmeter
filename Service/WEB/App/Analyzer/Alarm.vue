<template>
  <dlg ref="Dialog">
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">Analyzers &raquo; Alarms &raquo; Edit</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" v-on:click="Back"><img :src="require('Imgs/arrow_undo.png')" alt="Back"></button>
        <button type="button" class="btn btn-default btn-xs" v-on:click="Save"><img :src="require('Imgs/tick.png')" alt="Back"></button>
      </div>
    </template>
    <div class="row">
      <div class="col-xs-12">
        <label class="control-label">Alarm name</label>
        <input type="text" class="form-control form-control-sm"/>
      </div>
      <div class="col-xs-6">
        <label class="control-label">Priority</label>
        <option-picker :options="['LOW', 'NORMAL', 'HIGH']" selected="NORMAL" :big="true"></option-picker>
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
      <div class="col-xs-12">
        <label class="control-label">Expresion builder</label>
        <div class="input-group">
          <div class="input-group-btn">
            <button type="button" class="btn btn-default dropdown-toggle" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false"><img :src="require('Imgs/math_functions.png')"><span class="caret"></span></button>
            <ul class="dropdown-menu" role="menu" aria-labelledby="dropdownMenu">
              <li class="dropdown-submenu">
                <a>Variables</a>
                  <ul class="dropdown-menu">
                    <li class="dropdown-submenu" v-if="Values.hasOwnProperty('Voltage') && Array.isArray(Values.Voltage)">
                      <a>Voltage</a>
                      <ul class="dropdown-menu">
                        <li><a>RMS<span class="float-right menu-value" v-html="EvaluateFormat(RMS(Values.Voltage), 2)"></span></a></li>
                        <li><a>AVG<span class="float-right menu-value" v-html="EvaluateFormat(AVG(Values.Voltage), 2)"></span></a></li>
                        <li role="separator" class="divider"></li>
                        <li v-for="(val, index) in Values.Voltage"><a>Voltage[{{ index }}]<span class="float-right menu-value" v-html="EvaluateFormat(val, 2)"></span></a></li>
                      </ul>
                    </li>
                    <li v-if="Values.hasOwnProperty('Voltage') && !Array.isArray(Values.Voltage)"><a>Voltage<span class="float-right menu-value" v-html="EvaluateFormat(Values.Voltage, 2)"></span></a></li>

                    <li class="dropdown-submenu" v-if="Values.hasOwnProperty('Current') && (Values.Current.length > 1)">
                      <a>Current</a>
                      <ul class="dropdown-menu">
                        <li><a>RMS<span class="float-right menu-value" v-html="EvaluateFormat(RMS(Values.Current), 2)"></span></a></li>
                        <li><a>AVG<span class="float-right menu-value" v-html="EvaluateFormat(AVG(Values.Current), 2)"></span></a></li>
                        <li role="separator" class="divider"></li>
                        <li v-for="(val, index) in Values.Current"><a>Voltage[{{ index }}]<span class="float-right menu-value" v-html="EvaluateFormat(val, 2)"></span></a></li>
                      </ul>
                    </li>
                    <li v-if="Values.hasOwnProperty('Current') && (Values.Current.length == 1)"><a>Current<span class="float-right menu-value" v-html="EvaluateFormat(Values.Current[0], 2)"></span></a></li>
                  </ul>
              </li>
              <li class="dropdown-submenu">
                <a href="#">Operators</a>
                  <ul class="dropdown-menu">
                    <li><a href="#">Power</a></li>
                    <li><a href="#">Square root</a></li>
                  </ul>
              </li>
            </ul>
          </div>
          <input type="text" class="form-control form-control-sm" v-on:keyup="Validate"/>
        </div>
      </div>
      <div class="col-xs-12">
        <div class="form-control" style="margin-top: 5px" ref="formula"></div>
      </div>
    </div>
    <div class="alert alert-danger text-center" style="margin-bottom: 5px; padding: 5px" v-if="Error != ''">{{ Error }}</div>
  </dlg>
</template>

<script>
//  import { create, all } from 'mathjs';
//  const math = create(all);
  import Dlg from 'Components/Dialog.vue';
  import InputSpin from 'Components/InputSpin.vue';
  import OptionPicker from 'Components/OptionPicker.vue';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    data: function() {
      return { Analyzer: null, Alarm: {Aggreg: '3S'}, Error: '', Values: {} };
    },
    methods: {
      RMS: function(values) {
        var Val = 0;
        for(var i = 0; i < values.length; i++) Val += Math.pow(values[i], 2.0);
        return Math.sqrt(Val / values.length);
      },
      AVG: function(values) {
        var Val = 0;
        for(var i = 0; i < values.length; i++) Val += values[i];
        return Val / values.length;
      },
      EvaluateFormat: function(expresion, decimals) {
        return oZm.RoundHTML(expresion, decimals);
      },
      AggregationChanged: function(val) {
        if(val == '3 seconds') {
          this.Alarm.Aggreg = '3S';
          $.ajax({url: 'getSeries', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json', data: JSON.stringify({Series: ['VOLTAGE', 'CURRENT', 'FREQUENCY', 'FLAG', 'ACTIVE_POWER', 'REACTIVE_POWER'], LastSample: 0, Analyzer: this.Analyzer.Analyzer, Aggreg: this.Alarm.Aggreg, To: 0, Period: 6000}),
            success: (result) => {
              this.FilterValues(result);
            },
            error: () => {
              this.FilterValues(null);              
            }
          });
        }
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
    components: { Dlg, InputSpin, OptionPicker}
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
</style>