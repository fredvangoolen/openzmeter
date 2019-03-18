<template>
  <div class="modal-content" v-if="Show">
    <div class="modal-header">
      <div class="pull-left"><h4 class="modal-title">Analyzers &raquo; Tariffs &raquo; Edit &raquo; Periods</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" @click="Back"><img :src="require('Imgs/arrow_undo.png')" alt="Back"></button>
      </div>
    </div>
    <div class="modal-body">
      <div class="row">
        <div class="col-xs-12">
          <table>
            <tr><td></td><td><strong>Contracted power <em>(kW)</em></strong></td><td><strong>Power costs <em>(€/kW/day)</em></strong></td><td><strong>Power costs <em>(€/kWh/day)</em></strong></td></tr>
            <tr v-for="(period, idx) in Periods">
                <td><strong>P{{idx + 1}}</strong></td>
                <td><div class="input-group input-group-xs"><input type="text" class="form-control" value="0.0"></div></td>
                <td><div class="input-group input-group-xs"><input type="text" class="form-control" value="0.0"></div></td>
                <td><div class="input-group input-group-xs"><input type="text" class="form-control" value="0.0"></div></td>
              </tr>
          </table>
        </div>
      </div>
    </div>
            <div class="col-xs-12 hide">
              <label class="control-label">IVA</label>
              <div class="input-group">
                <input type="text" class="form-control">
                <span class="input-group-addon">%</span>
              </div>
            </div>
          </div>

        </div>
  </div>
  
</template>

<script>
  export default {
    data: function() {
      return { Show: false, Periods: [1,2,3] };
    },
    methods: {
      Load: function(periods) {
        
      },
      Back: function() {
        
      }
    }
  }
</script>

<style scoped>
  table, .input-group {
    width: 100%;
  }
  table tr td {
    padding: 1px 3px;
  }
</style>