function EnergyResizeEvent() {
  if(EnergyChart !== null) EnergyChart.resize('100%', $('#EnergyPanel > .panel-body > .row > div:first-child').height());
}

/* Contadores de consumo */
function Scrollable_UpdateCounters(times, renew) {
  if(renew === true) {
    $('#labelEnergyPrimary').text(times.PrimaryText);
    $('#labelBudgetPrimary').text(times.PrimaryText + ' budget');
    $('#labelEnergyPrimaryValue').text('Calculating...');
    $('#labelEnergyPrimaryUnits #labelEnergyPrimaryPhases, #labelBudgetUnits').text('');
    $('#labelEnergySecondary, #labelBudgetSecondary').text(times.SecondaryText);
    $('#labelEnergySecondaryValue, #labelBudgetSecondaryValue').text((times.SecondaryTo !== undefined) ? 'Calculating...' : '');
    if(times.SecondaryTo !== undefined) {
      Analyzer.GetSeriesAggreg(times.SecondaryPeriod, times.SecondaryTo, times.PreferedAggreg, 'sum(active_energy)', 
        function(result) {
          if(result['Samples'] !== 0) {
            var Sum = ArraySum(result['sum_active_energy']);
            var Units = GetBestUnits(Sum);
            var Text = Round2(ScaleToUnits(Sum, Units)) + '&nbsp;' + Units + 'Wh';
            if(result['sum_active_energy'].length > 1) Text += '(' + Array_Round2(ScaleArrayToUnits(result['sum_active_energy'], Units)) + ')';
            $('#labelEnergySecondaryValue').html(Text);
          } else {
            $('#labelEnergySecondaryValue').text('No data');
          }
          window.dispatchEvent(new Event('resize'));
        }
      );
      Analyzer.GetPrices(times.SecondaryPeriod, times.SecondaryTo, 0,
        function(result) {
          if(result['Sampletime'].length !== 0) {
            $('#labelBudgetUnits').text('€');
            var Budget = 0.0;
            for(var i = 0; i < result['Sampletime'].length; i++) Budget += result['active_energy'][i] * result['price'][i] / 1000.0;
            $('#labelBudgetSecondaryValue').html(Round2(Budget));
          } else {
            $('#labelBudgetSecondaryValue').text('No data');
          }
        }
      );
    }
  }
  Analyzer.GetSeriesAggreg(times.PrimaryPeriod, times.PrimaryTo, times.PreferedAggreg, 'sum(active_energy)', 
    function(result) {
      var Sum = ArraySum(result['sum_active_energy']);
      var Units = GetBestUnits(Sum);
      $('#labelEnergyPrimaryValue').html(Round2(ScaleToUnits(Sum, Units)));
      $('#labelEnergyPrimaryUnits').text(Units + 'Wh');
      if(result['sum_active_energy'].length > 1) $('#labelEnergyPrimaryPhases').html('By phases:&nbsp;' + Array_Round2(ScaleArrayToUnits(result['sum_active_energy'], Units)) + '&nbsp;' + Units + 'Wh');
      window.dispatchEvent(new Event('resize'));
    }
  );
  Analyzer.GetPrices(times.PrimaryPeriod, times.PrimaryTo, 0,
    function(result) {
      $('#labelBudgetUnits').text('€');
      var Budget = 0.0;
      for(var i = 0; i < result['Sampletime'].length; i++) Budget += result['active_energy'][i] * result['price'][i] / 1000.0;
      $('#labelBudgetPrimaryValue').html(Round2(Budget));
    }
  );
}
