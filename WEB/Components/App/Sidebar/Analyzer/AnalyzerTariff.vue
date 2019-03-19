<template>
  <div class="modal-content" v-bind:class="{ hide: !Show }">
    <div class="modal-header">
      <div class="pull-left"><h4 class="modal-title">Analyzers &raquo; Tariffs &raquo; Edit</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" v-bind:class="{ disabled: (Error != '') }" @click="Save"><img :src="require('Imgs/diskette.png')" alt="Save tariff"></button>
        <button type="button" class="btn btn-default btn-xs" v-bind:class="{ disabled: (Error != '') }" @click="Periods">Periods</button>
        <button type="button" class="btn btn-default btn-xs" v-bind:class="{ disabled: (Error != '') }" @click="Taxs">Taxs</button>
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
        this.Tariff.Template = Object.assign({}, this.Initial.Template);
        this.Tariff.Name = this.Initial.Name;
      },
      SetTariffs: function() {
        $(this.$refs.Base).bsSuggest( { hideOnSelect: true, effectiveFields: ['name', 'description'], showHeader: false, ignorecase: true, indexId: 0, indexKey: 1, data: { value: this.Tariffs } } );
        $(this.$refs.Base).on('onSetSelectValue', this.SelectBase.bind(this));
        $(this.$refs.Base).on('onUnsetSelectValue', this.DeselectBase.bind(this));
        if(this.Initial !== null) this.LoadCurrent();
        this.Validate();
        this.ShowSpinner = false;
      },
      Save: function() {
//        alert("guardar no terminado!!");
//        TariffDefinition.Name = $('#inputTariffName').val();
//  TariffDefinition.Details.Description = $('#inputTariffComments').val();
//  if(TariffKey !== null) TariffDefinition.RateID = TariffKey;
        $.ajax({ url: "setTariffs", timeout: 20000, type: "POST", dataType: "text", contentType: 'application/json', data: JSON.stringify( Object.assign(this.Tariff, { RateID: this.RateID, Analyzer: this.Serial }) ), 
          success: function() {
            this.Close();
            this.$parent.GetTariffs();
          }.bind(this),
          error: function() {
            this.Error = 'Failed to save tariff to analyzer!';
            setTimeout(function() { this.Error = ''; }.bind(this), 1000); 
          }.bind(this)
        });
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
                    this.Initial = result;
                    this.Tariff = Object.assign({}, result);
                    this.$refs.StartDate.SetDate(moment(this.Tariff.StartDate));
                    this.$refs.EndDate.SetDate(moment(this.Tariff.EndDate));
                    this.Tariffs.unshift({ id: 'CURRENT', name: '(*) ' + result.Name, description: 'From ' + moment(result.StartDate).format('DD/MM/YYYY') + ' to ' + moment(result.EndDate).format('DD/MM/YYYY')});
                    this.SetTariffs();
                  }.bind(this)
                });
              } else {
                this.Initial.Template = null;
                this.Tariff = { Template: { Description: '' }};
                this.$refs.StartDate.Disable();
                this.$refs.EndDate.Disable();
                this.SetTariffs();
              }
            }.bind(this)
          });
        });
      },
      Validate: function() {
        this.Error = '';
        if(this.Tariff.StartDate < this.Tariff.Template.Seasons[0].StartDate) {
          this.Error = 'Date ranges is outside selected tariff ranges';
          return;
        }
        if(this.Tariff.EndDate > this.Tariff.Template.Seasons.slice(-1)[0].EndDate) {
          this.Error = 'Date ranges is outside selected tariff ranges';
          return;
        }
      },
      SetStartDate: function(date) {
        this.Tariff.StartDate = date.startOf('day').valueOf();
        this.Validate();
      },
      SetEndDate: function(date) {
        this.Tariff.EndDate = date.endOf('day').valueOf();
        this.Validate();
      },
      Back: function() {
        this.Show = false;
        this.$parent.$refs.Tariffs.Load(this.Serial);
      },
      SelectBase: function(e, keywork) {
        this.$refs.StartDate.Enable();
        this.$refs.EndDate.Enable();
        if(keywork.id === 'CURRENT') {
          this.LoadCurrent();
          this.Validate();
        } else {
          this.ShowSpinner = true;
          $.ajax({ url: "getTariffs", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ RateID: keywork.id }), 
            success: function(result) {
              this.Tariff.Template = result.Details;
              this.Tariff.Name = result.Name;
              this.Validate();
              this.ShowSpinner = false;
            }.bind(this)
          });
        }
      },
      DeselectBase: function(e) {
        this.$refs.StartDate.Disable();
        this.$refs.EndDate.Disable();
        this.Error = 'Select a base tariff from list';
      },
      Periods: function() {
        this.Show = false;
        this.$parent.$refs.Periods.Load(this.Tariff);
      },
      Taxs: function() {
        this.Show = false;
        this.$parent.$refs.Taxs.Load(this.Tariff);
      }
    },
    created: function() {
      this.Serial = null;
      this.Tariffs = [];
      this.Initial = null;
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