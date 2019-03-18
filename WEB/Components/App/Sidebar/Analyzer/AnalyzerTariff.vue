<template>
  <div class="modal-content" v-if="Show">
    <div class="modal-header">
      <div class="pull-left"><h4 class="modal-title">Analyzers &raquo; Tariffs &raquo; Edit</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" v-bind:class="{ disabled: (Tariff == null) }" @click="Save"><img :src="require('Imgs/diskette.png')" alt="Save tariff"></button>
        <button type="button" class="btn btn-default btn-xs" v-bind:class="{ disabled: (Tariff == null) }" @click="Save">Periods</button>
        <button type="button" class="btn btn-default btn-xs" v-bind:class="{ disabled: (Tariff == null) }" @click="Save">Taxs</button>
        <button type="button" class="btn btn-default btn-xs" @click="Back"><img :src="require('Imgs/arrow_undo.png')" alt="Discard"></button>
      </div>
    </div>
    <div class="modal-body">
      <spinner v-if="ShowSpinner"></spinner>
      <div class="alert alert-danger text-center" style="margin-bottom: 5px; padding: 5px" v-if="Error != '' && !ShowSpinner">{{ Error }}</div>
      <div class="alert alert-success text-center" style="margin-bottom: 5px; padding: 5px" v-if="Error == '' && !ShowSpinner">{{ Tariff.Template.Description }}<br><strong>Tariff contains {{ Tariff.Template.Seasons.length }}  seasons, a total of {{ Tariff.Template.Periods.length }}  day periods and {{ Tariff.Template.Holidays.length }} holidays</strong></div>
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
  import Spinner from 'Components/Spinner.vue';
  export default {
    data() {
      return { Show: false, ShowSpinner: true, Tariff: null, Error: '' };
    },
    methods: {
      LoadCurrent: function() {
        $(this.$refs.Base).attr('data-id', 'CURRENT');
        $(this.$refs.Base).val(this.Tariffs[0].name);
        this.Description = this.InitialTemplate.Description;
      },
      SetTariffs: function() {
        $(this.$refs.Base).bsSuggest( { hideOnSelect: true, effectiveFields: ['name', 'description'], showHeader: false, ignorecase: true, indexId: 0, indexKey: 1, data: { value: this.Tariffs } } );
        $(this.$refs.Base).on('onSetSelectValue', this.SelectBase.bind(this));
        $(this.$refs.Base).on('onUnsetSelectValue', this.DeselectBase.bind(this));
        if(this.InitialTemplate !== null) this.LoadCurrent();
        this.Validate();
        this.ShowSpinner = false;
      },
      Load: function(serial, id) {
        this.Serial = serial;
        this.Show = true;
        this.ShowSpinner = true;
        this.Tariffs = [];
        this.$nextTick(function() {
          $.ajax({ url: "getTariffs", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ }), 
            success: function(result) {
              for(var i = 0; i < result.length; i++) this.Tariffs.push({ id: result[i].RateID, name: result[i].Name, description: 'From ' + moment(result[i].StartDate).format('DD/MM/YYYY') + ' to ' + moment(result[i].EndDate).format('DD/MM/YYYY')}); 
              if(id !== null) {
                $.ajax({ url: "getTariffs", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ Analyzer: this.Serial, RateID: id }), 
                  success: function(result) {
                    this.InitialTemplate = result.Template;
                    this.Tariff = result;
                    this.Tariffs.unshift({ id: 'CURRENT', name: '(*) ' + result.Name, description: 'From ' + moment(result.StartDate).format('DD/MM/YYYY') + ' to ' + moment(result.EndDate).format('DD/MM/YYYY')});
                    this.SetTariffs();
                  }.bind(this)
                });
              } else {
                this.InitialTemplate = null;
                this.Tariff = { Template: { Description: '' }};
                this.SetTariffs();
              }
            }.bind(this)
          });
        });
      },
      Validate: function() {
//    $('#divAnalyzersTariffSuccess, #divAnalyzersTariffError').addClass('hide');
//    $('#btnAnalyzersTariffSave').addClass('disabled');
//    if($('#inputAnalyzerTariffBase').attr('data-id') == '') {
//      $('#divAnalyzersTariffError').text('Choose a tariff base!').removeClass('hide');
//      return;
//    }
//    var Overlap = false;
//    for(var i = 0; i < Self.AvaliableTariffs.length; i++) {
//      if(Self.OnEditTariff.RateID === Self.AvaliableTariffs[i].RateID) continue;
//      if((Self.OnEditTariff.StartDate < Self.AvaliableTariffs[i].EndDate) && (Self.AvaliableTariffs[i].StartDate < Self.OnEditTariff.EndDate)) Overlap = true
//    }
//    if(Overlap) {
//      $('#divAnalyzersTariffError').text('Defined tariff period overlaps with other assigned tariffs').removeClass('hide');
//      return;
//    }
//    $('#divAnalyzersTariffSuccess').text('Tariff is correctly configured').removeClass('hide');
//    $('#btnAnalyzersTariffSave').removeClass('disabled');
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
        if(keywork.id === 'CURRENT') {
          this.LoadCurrent();
        } else {
          this.ShowSpinner = true;
          $.ajax({ url: "getTariffs", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ RateID: keywork.id }), 
            success: function(result) {
              this.Tariff.Template = result.Details;
              this.Tariff.Name = result.Name;
//              this.$refs.StartDate.SetDate(moment(this.BaseTariff.StartDate));
//              this.$refs.EndDate.SetDate(moment(this.BaseTariff.EndDate));
              this.ShowSpinner = false;
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
        }
        this.Validate();
      },
      DeselectBase: function(e) {
        alert("unselect");    
      }
    },
    created: function() {
      this.Serial = null;
      this.Tariffs = [];
      this.InitialTemplate = null;
    },
    destroyed: function() {
      $(this.$refs.Base).off('onSetSelectValue', this.SelectBase.bind(this));
      $(this.$refs.Base).off('onUnsetSelectValue', this.DeselectBase.bind(this));
      $(this.$refs.Base).bsSuggest("destroy");
    },
    components: { DatePicker, Spinner }
  }
</script>

<style scoped>
  .dropdown-menu >>> .jhover {
    background: #337ab7;
  }
  .dropdown-menu >>> .table {
    margin: 4px 0px 0px 0px !important;
  }
  .dropdown-menu >>> td {
    border: none;
  }
</style>