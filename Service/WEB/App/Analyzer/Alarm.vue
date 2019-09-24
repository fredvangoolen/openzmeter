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
        <input type="text" name="name" class="form-control form-control-sm"/>
      </div>
      <div class="col-xs-6">
        <label class="control-label">Priority</label>
        <option-picker :options="['LOW', 'NORMAL', 'HIGH']" :selected="NORMAL" :big="true"></option-picker>
      </div>
      <div class="col-xs-6">
        <label class="control-label">Aggregation</label>
        <option-picker :options="['3 seconds', '1 minute', '10 minutes', '15 minutes', '1 hour']" :big="true"></option-picker>
      </div>
      <div class="col-xs-6">
        <label class="control-label">Occurrences</label>
        <input-spin :min="1" :value="Alarm.Samples" :increment="1" :decimals="0"></input-spin>
      </div>
      <div class="col-6">
        <label class="control-label">Window</label>
        <input-spin :min="1" :value="Alarm.Window" :increment="1" :decimals="0"></input-spin>
      </div>
    </div>
    <div ref="blockly" id="blockly"></div>
  </dlg>

</template>

<script>
  import Blockly from 'blockly';
  import Dlg from 'Components/Dialog.vue';
  import InputSpin from 'Components/InputSpin.vue';
  import OptionPicker from 'Components/OptionPicker.vue';
//    import {Alarm} from "./models/Alarm";


  export default {
    workspace: undefined,

    data() {
      return {
        visible: true,
        Alarm: {},
        aggregation: {value: '3S', text: '3 seconds'}
      }
    },

    created() {
//      this.load();
    },

    methods: {
      Show() {
        this.$refs.Dialog.Show();
        this.$nextTick(() => {
          this.loadBlockly();
        });
      },

      setAggregation(aggregation, event) {
        event.preventDefault();
        this.aggregation = {value: aggregation, text: $(event.target).text()};
      },

      back() {
        this.$refs.Dialog.Close();
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

  input {
    color: black;
  }

  #blockly {

    width: 100%;
    height: 300px;


  }
</style>