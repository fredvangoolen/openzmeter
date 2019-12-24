<template>
  <div class="modal" tabindex="1" role="dialog" data-backdrop="false" data-keyboard="false" v-if="ShowDialog">
    <div class="modal-backdrop in"></div>
    <div class="modal-dialog">
      <edit-main ref="Main" v-on:Close="Close"></edit-main>
      <edit-periods ref="Periods"></edit-periods>
      <edit-seasons ref="Seasons"></edit-seasons>
      <edit-season ref="Season"></edit-season>
      <edit-holidays ref="Holidays"></edit-holidays>
      <edit-holiday ref="Holiday"></edit-holiday>
    </div>
  </div>
</template>

<script>
  import EditMain from './TariffsEditMain.vue';
  import EditPeriods from './TariffEditPeriods.vue';
  import EditSeasons from './TariffEditSeasons.vue';
  import EditSeason from './TariffEditSeason.vue';
  import EditHolidays from './TariffEditHolidays.vue';
  import EditHoliday from './TariffEditHoliday.vue';
  import moment from 'moment';
  export default {
    data: function() {
      return { ShowDialog: false, Tariff: {} };
    },
    methods: {
      Show: function(tariff) {
        this.$set(this, 'Tariff', tariff);
        this.ShowDialog = true;
        this.$nextTick(() => {
          $(this.$el).modal('show');
          this.$refs.Main.Load(this.Tariff);
        });
      },
      Save: function() {
        $.ajax({ url: (this.Tariff.TariffID === undefined) ? 'addTariff' : 'setTariff', timeout: 20000, type: 'POST', dataType: 'text', contentType: 'application/json', data: JSON.stringify(this.Tariff), 
          success: function() {
            this.Close();
            this.$parent.GetTariffs();
          }.bind(this),
          error: function() {
            this.$refs.Main.Error = 'Failed to save tariff!';
            setTimeout(function() { this.$refs.Main.Error = ''; }.bind(this), 1000); 
          }.bind(this)
        });
      },
      Check: function(tariff) {
        $('#divTariffSuccess, #divTariffError').addClass('hide');
        $('#btnTariffSave').addClass('disabled');
        var FindSeason = function(date) {
          for(var i = 0; i < tariff.Template.Seasons.length; i++) { if(tariff.Template.Seasons[i].StartDate === date) return i; }
          return null;
        };
        for(var i = 0; i < tariff.Template.Seasons.length; i++) {
          for(var h = 0; h < 24; h++) {
            if((tariff.Template.Seasons[i].DayPeriods[h] < 0) || (tariff.Template.Seasons[i].DayPeriods[h] >= tariff.Template.Periods.length)) return 'Periods in season "' + tariff.Template.Seasons[i].Name + '" out of range!';
          }
        }
        var Used = tariff.Template.Seasons.length;
        var Date = tariff.StartDate;
        while(Used > 0) {
          var Find = FindSeason(Date);
          if(Find === null) break;
          Date = tariff.Template.Seasons[Find].EndDate;
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
    components: { EditMain, EditPeriods, EditSeasons, EditSeason, EditHolidays, EditHoliday }
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