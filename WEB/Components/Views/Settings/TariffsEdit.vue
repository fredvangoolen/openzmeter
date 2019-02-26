<template>
  <div class="modal" tabindex="-1" role="dialog" data-backdrop='false' v-if="ShowDialog">
    <div class="modal-backdrop in"></div>
    <div class="modal-dialog">
      <edit-main ref="Main" v-on:Close="Close"></edit-main>
      <edit-periods ref="Periods"></edit-periods>
      <edit-seasons ref="Seasons"></edit-seasons>
      <edit-holidays ref="Holidays"></edit-holidays>
    </div>
  </div>
</template>

<script>
  import EditMain from './TariffsEditMain.vue';
  import EditPeriods from './TariffEditPeriods.vue';
  import EditSeasons from './TariffEditSeasons.vue';
  import EditHolidays from './TariffEditHolidays.vue';
  export default {
    data: function() {
      return { ShowDialog: false, Tariff: {}, TariffID: null };
    },
    methods: {
      Show: function(id) {
        this.ShowDialog = true;
        this.TariffID = id;  
        this.$nextTick(function() {  
          if(id === null) {
            this.Tariff = { 
              Country: null, 
              StartDate: new Date(new Date().getFullYear(), 0, 1, 0, 0, 0, 0).getTime(), 
              EndDate: new Date(new Date().getFullYear(), 11, 31, 23, 59, 59, 999).getTime(), 
              Name: 'No nane', 
              Details: {
                Description: 'Add tariff description here', 
                Seasons: [], 
                Periods: [ { "ReactiveMode": null, "PowerMode": null } ], 
                Holidays: [] 
              } 
            };
            this.$refs.Main.Load(this.Tariff);
            $(this.$el).modal('show');
          } else {
            $.ajax({ url: "getTariffs", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ RateID: id }),
              success: function(result) {
                this.Tariff = result;
                this.$refs.Main.Load(this.Tariff);
                $(this.$el).modal('show');
              }.bind(this)
            });
          }          
        });
      },
      Save: function() {
        alert("guardar no terminado!!");
//        TariffDefinition.Name = $('#inputTariffName').val();
//  TariffDefinition.Details.Description = $('#inputTariffComments').val();
//  if(TariffKey !== null) TariffDefinition.RateID = TariffKey;
//  $.ajax({ url: "setTariffs", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify(TariffDefinition), 
//    success: function() { $('#dlgTariff').modal('hide'); },
//    error: function() { $('#divTariffError').text('Failed to save tariff!').removeClass('hide'); $('#btnTariffSave').addClass('disabled'); setTimeout(CheckTariff, 2000); }
//  });
        
      },
      Check: function(tariff) {
        $('#divTariffSuccess, #divTariffError').addClass('hide');
        $('#btnTariffSave').addClass('disabled');
        var FindSeason = function(date) {
          for(var i = 0; i < tariff.Details.Seasons.length; i++) { if(tariff.Details.Seasons[i].StartDate === date) return i; }
          return null;
        };
        for(var i = 0; i < tariff.Details.Seasons.length; i++) {
          for(var h = 0; h < 24; h++) {
            if((tariff.Details.Seasons[i].DayPeriods[h] < 1) || (tariff.Details.Seasons[i].DayPeriods[h] > tariff.Details.Periods.length)) return 'Periods in season "' + tariff.Details.Seasons[i].Name + '" out of range!';
          }
        }
        var Used = tariff.Details.Seasons.length;
        var Date = tariff.StartDate;
        while(Used > 0) {
          var Find = FindSeason(Date);
          if(Find === null) break;
          Date = tariff.Details.Seasons[Find].EndDate;
          Used = Used - 1;
          if(Used > 0) Date = Date + 1;
        }
        if((Used > 0) || (Date !== tariff.EndDate)) return 'Defined tariff period is not correctly covered by defined seasons!';
        this.Tariff = tariff;
        return '';
      },
      Close: function() {
        $(this.$el).modal('hide');
        this.ShowDialog = false;
      }
    },
    mounted: function() {
      this.$root.$el.append(this.$el);
    },
    destroyed: function() {
      this.$root.$el.removeChild(this.$el);
    },
    components: { EditMain, EditPeriods, EditSeasons, EditHolidays }
  }
</script>

<style scoped>
  .modal-dialog {
    z-index: 1100;
    color: #333;
    font-size: 14px;
    margin-top: 70px;
  }
</style> 
 