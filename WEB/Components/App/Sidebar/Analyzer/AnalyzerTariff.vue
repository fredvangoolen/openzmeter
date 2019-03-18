<template>
  <div class="modal-content" v-if="Show">
    <div class="modal-header">
      <div class="pull-left"><h4 class="modal-title">Analyzers &raquo; Tariffs &raquo; Edit</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" @click="Save"><img :src="require('Imgs/diskette.png')" alt="Save tariff"></button>
        <button type="button" class="btn btn-default btn-xs" @click="Save">Periods</button>
        <button type="button" class="btn btn-default btn-xs" @click="Save">Taxs</button>
        <button type="button" class="btn btn-default btn-xs" @click="Back"><img :src="require('Imgs/arrow_undo.png')" alt="Discard"></button>
      </div>
    </div>
    <div class="modal-body">
      <div class="alert alert-danger text-center hide" id="divAnalyzersTariffError" style="margin-bottom: 5px; padding: 5px;"></div>
      <div class="alert alert-success text-center hide" id="divAnalyzersTariffSuccess" style="margin-bottom: 5px; padding: 5px;"></div>
      <div class="row">
        <div class="col-xs-12">
          <label class="control-label">Base Tariff</label>
          <div class="input-group">
            <input type="text" class="form-control" ref="Base">
            <div class="input-group-btn">
              <button type="button" class="btn btn-default dropdown-toggle" data-toggle=""><span class="caret"></span></button>
              <ul class="dropdown-menu dropdown-menu-right" role="menu"></ul>
            </div>
          </div>
        </div>
        <div class="col-xs-12"><label class="control-label">Description</label><textarea class="form-control" rows="3" ref="Description" style="resize: none" disabled>{{ Description }}</textarea></div>
        <div class="col-xs-12 col-sm-6"><label class="control-label">From</label><date-picker ref="StartDate" :options="{ format: 'DD/MM/YYYY', viewMode: 'months' }" v-on:DateChanged="SetStartDate"></date-picker></div>
        <div class="col-xs-12 col-sm-6"><label class="control-label">To</label><date-picker ref="EndDate" :options="{ format: 'DD/MM/YYYY', viewMode: 'months' }" v-on:DateChanged="SetEndDate"></date-picker></div>
      </div>              
    </div>
  </div>
</template>

<script>
  import bsSuggest from 'JS/bootstrap-suggest.js';
  import moment from 'JS/moment.js';
  import DatePicker from 'Components/DatePicker.vue';
  export default {
    data() {
      return { Show: false, Description: '', BaseTariff: null };
    },
    methods: {
      Load: function(serial, id) {
        this.Serial = serial;
        this.RateID = id;
        this.Show = true;
        var Data = [];
        this.$nextTick(function() {
          $.ajax({ url: "getTariffs", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ }), 
            success: function(result) {
              this.BaseTariffs = result;
              for(var i = 0; i < result.length; i++) Data.push({ id: result[i].RateID, name: result[i].Name, description: 'From ' + moment(result[i].StartDate).format('DD/MM/YYYY') + ' to ' + moment(result[i].EndDate).format('DD/MM/YYYY')}); 
              $(this.$refs.Base).bsSuggest( { hideOnSelect: true, effectiveFields: ['name', 'description'], showHeader: false, ignorecase: true, indexId: 0, indexKey: 1, data: { value: Data } } );
              $(this.$refs.Base).on('onSetSelectValue', this.SelectBase.bind(this));
              $(this.$refs.Base).on('onUnsetSelectValue', this.DeselectBase.bind(this));
            }.bind(this)
          });
        });
      },
      SetStartDate: function() {
        
      },
      SetEndDate: function() {
        
      },
      Save: function() {
        
      },
      Back: function() {
        this.Show = false;
        this.$parent.$refs.Tariffs.Load(this.Serial);
      },
      SelectBase: function(e, keywork) {
        $.ajax({ url: "getTariffs", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ RateID: keywork.id }), 
          success: function(result) {
            this.BaseTariff = result;
            this.Description = this.BaseTariff.Details.Description;
            this.$refs.StartDate.SetDate(moment(this.BaseTariff.StartDate));
            this.$refs.EndDate.SetDate(moment(this.BaseTariff.EndDate));
          }.bind(this)
//hago la consulta para obtener periodos, y fechas de inicio y fin
    //Cargo los datepickers con las fechas        
//            Self.OnEditTariff = Self.OnEditTariffCurrent;
//    if(id !== 'CURRENT') $.ajax({ url: "getTariffs", timeout: 20000, async: false, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ RateID: parseInt(id) }), success: function(result) { Self.OnEditTariff = result; } });
//    Self.OnEditTariff.RateID = (Self.OnEditTariffCurrent !== null) ? Self.OnEditTariffCurrent.RateID : null;
//    $('#inputAnalyzerTariffStartDate').data("DateTimePicker").enable();
//    $('#inputAnalyzerTariffEndDate').data("DateTimePicker").enable();
//    $('#inputAnalyzerTariffStartDate').data("DateTimePicker").date(moment(Self.OnEditTariff.StartDate));
//    $('#inputAnalyzerTariffEndDate').data("DateTimePicker").date(moment(Self.OnEditTariff.EndDate));
//    

//    Self.TariffValidate();
        });
        
      },
      DeselectBase: function(e) {
        alert("unselect");    
      }
    },
    created: function() {
      this.Serial = null;
    },
    destroyed: function() {
      $(this.$refs.Base).off('onSetSelectValue', this.SelectBase.bind(this));
      $(this.$refs.Base).off('onUnsetSelectValue', this.DeselectBase.bind(this));
      $(this.$refs.Base).bsSuggest("destroy");
    },
    components: { DatePicker }
  }
</script>

