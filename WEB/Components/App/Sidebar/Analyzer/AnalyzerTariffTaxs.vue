<template>
  <div class="modal-content" v-if="Show">
    <div class="modal-header">
      <div class="pull-left"><h4 class="modal-title">Analyzers &raquo; Tariffs &raquo; Edit &raquo; Taxs</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" @click="Add"><img :src="require('Imgs/plus.png')" alt="Add taxs"></button>
        <button type="button" class="btn btn-default btn-xs" @click="Back"><img :src="require('Imgs/arrow_undo.png')" alt="Back"></button>
      </div>
    </div>
    <div class="modal-body">
      <div class="row">
        <div class="col-xs-12">
          <table>
            <tr><td><strong>Description</strong></td><td class="text-right"><strong>Value</strong></td></tr>
            <tr v-for="(tax, idx) in Tariff.Config.Taxs">
                <td><div class="input-group input-group-xs">
                  <span class="input-group-btn"><button class="btn btn-default" type="button" @click="Delete(idx)"><img :src="require('Imgs/cross.png')"></button></span>
                  <input type="text" class="form-control" @change="NameChanged($event, idx)" :value="tax.Name">
                </div></td>
                <td><div class="input-group input-group-xs">
                  <input type="text" class="form-control text-right" @change="ValueChanged($event, idx)" :value="tax.Value"> 
                  <span class="input-group-addon">{{ Types[tax.Type].Units }}</span>
                  <div class="input-group-btn">
                    <button type="button" class="btn btn-default dropdown-toggle" data-toggle="dropdown">{{ Types[tax.Type].Text }} <span class="caret"></span></button>
                    <ul class="dropdown-menu dropdown-menu-right">
                      <li v-for="(item, index) in Types"><a @click="TypeChanged(idx, index)">{{ item.Text }}</a></li>
                    </ul>
                  </div>
                  
                </div></td>
              </tr>
          </table>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
  export default {
    data: function() {
      return { Show: false };
    },
    methods: {
      Load: function(tariff) {
        this.Tariff = tariff;
        this.Show = true;
      },
      Back: function() {
        this.Show = false;
        this.$parent.$refs.Tariff.Show = true;
      },
      Add: function() {
        this.Tariff.Config.Taxs.push({Name: 'Unnamed tax', Type: 'PERCENT_TOTAL', Value: 0.0});
      },
      Delete: function(idx) {
        this.Tariff.Config.Taxs.splice(idx, 1);
      },
      TypeChanged: function(idx, index) {
        this.Tariff.Config.Taxs[idx].Type = index;
        this.Tariff = Object.assign({}, this.Tariff);
      },
      ValueChanged(value, idx) {
        var Value = parseFloat(value.target.value);
        if(isNaN(Value) === true) Value = 0.0;
        this.Tariff.Config.Taxs[idx].Value = Value;
        this.Tariff = Object.assign({}, this.Tariff);
      },
      NameChanged(value, idx) {
        this.Tariff.Config.Taxs[idx].Name = value.target.value;
        this.Tariff = Object.assign({}, this.Tariff);
      }

    },
    created: function() {
      this.Types = {
        'PERCENT_TOTAL': { Text: 'Total', Units: '%'},
        'PERCENT_ENERGY': { Text: 'Energy', Units: '%'},
        'PERCENT_POWER': { Text: 'Power', Units: '%'},
        'PERCENT_ENERGY_POWER': { Text: 'Energy + Power', Units: '%'},
        'FIXED_QUANTITY': { Text: 'Fixed', Units: '€/Bill cycle'}
      };
    }
  }
</script>

<style scoped>
  table, .input-group {
    width: 100%;
  }
  table tr td {
    padding: 1px 3px;
    width: 50%;
  }
  .input-group-addon {
    border-left: none;
  }
  .input-group-addon:nth-child(2) {
    width: 70px;
  }
  .dropdown-toggle {
    width: 120px;
  }
</style>