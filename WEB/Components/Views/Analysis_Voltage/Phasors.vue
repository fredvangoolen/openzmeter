<template>
              <div id="PhasorPanel">
              <div class="row">
                <div class="col-md-4 col-lg-3 col-xl-2">
                  <div id="PhasorChart" class="chart big_chart"></div>
                </div>
                <div class="col-md-8 col-lg-9 col-xl-10">
                  <h4>Voltage and current phase angle</h4>
                  <table id="PhasorsTable" class="table small table-striped table-responsive">
                    <thead>
                      <tr>
                        <th>Phase</th><th>Channel</th><th>Angle</th><th>Value</th>
                      </tr>
                    </thead>
                    <tbody>
                    </tbody>
                  </table>
                </div>
              </div>
            </div>
</template>

function PhasorResizeEvent() {
  var Width = $('#PhasorChart').parent().width();
  if(PhasorChart !== null) PhasorChart.resize(Width, Width);
}




/* Grafico de fasores */
var PhasorChart = null;
var PhasorTable = null;
function UpdatePhasorChart(result) {
  var Vectors = [];
  var MaxCurrent = 0;
  var MaxVoltage = 0;
  for(i = 0; i < result['Voltage'].length; i++) {
    Vectors.push([2 * i, 120 * i, result.Voltage[i]]);
    Vectors.push([2 * i + 1, 120 * i - result.Phi[i], result.Current[i]]);
    MaxVoltage = Math.max(MaxVoltage, result.Voltage[i]);
    MaxCurrent = Math.max(MaxCurrent, result.Current[i]);
  }
  for(i = 0; i < Vectors.length; i += 2) {
    Vectors[i][2] = Vectors[i][2] / MaxVoltage;
    Vectors[i + 1][2] = Vectors[i + 1][2] / MaxCurrent;
  }
  var Options = { 
    strokeWidth: 0.5, gridLineColor: "#BBB", gridLinePattern: [4, 4], highlightCircleSize: 0,  interactionModel: { }, legend: 'never', valueRange: [-115, 115], dateWindow: [-115, 115], plotter: polarLinePlotter,
    labels: ['Index', 'Phase', 'Magnitude'], colors: ['#d9534f', '#428bca'], xAxisHeight: 0.1, xLabelHeight: 0.1, plugins: [ new Dygraph.Plugins.PolarCrosshair( { } ) ],
    axes: {
      y: { drawAxis: true, drawGrid: false, axisLabelWidth: 0.0, axisLineColor: "#FFF",
           ticker: function(min, max, pixels, opts, dygraph, vals) {
             var ticks = [];
             var Width = document.getElementById("PhasorChart").clientWidth / 2 - 1;
             for(var i = 1; i <= 4; i++) {
               ticks.push({ v: i * 25, label: '<div style="font-size: 11px; background: rgba(255, 255, 255, 0.8); margin-left: ' + Width + 'px; text-align: left; width: 44px; line-height: 12px">' + Round2(i / 4 * MaxVoltage) + '<em>V</em></div>' });
             }
             return ticks;
           },            
         },
      x: { drawAxis: true, drawGrid: false, axisLabelWidth: 0, axisLineColor: "#FFF",
           ticker: function(min, max, pixels, opts, dygraph, vals) {
             var ticks = [];
             var Height = document.getElementById("PhasorChart").clientHeight / 2 + 2;
             for(var i = 1; i <= 4; i++) {
               ticks.push({ v: -i * 25, label: '<div style="font-size: 11px; background: rgba(255, 255, 255, 0.8); margin-left: -20px; margin-top: ' + -Height + 'px; text-align: center; width: 40px; line-height: 12px">' + Round2(i / 4 * MaxCurrent) + '<em>A</em></div>' });
             }
             return ticks;
           }
         }
      }
    };
  if(PhasorChart === null) {
    PhasorChart = new Dygraph(document.getElementById("PhasorChart"), Vectors, Options);
    $(window).bind('resize', PhasorResizeEvent);
    PhasorChart.updateOptions({ file: Vectors });
    PhasorChart.updateOptions(Options);
  }
  if(PhasorTable === null) {
    PhasorTable = $('#PhasorsTable').DataTable({ responsive: true, paging: false, info: false, searching: false,
                                                 columnDefs: [ { targets: [0, 1, 2, 3], searchable: false } ],                            
    });
  }
  PhasorTable.clear();
  for(i = 0; i < result.Voltage.length; i++) {
    var PhaseName = "L";
    if(result.Voltage.length > 1) {
      if(i === 0) PhaseName = "R";
      if(i === 1) PhaseName = "S";
      if(i === 2) PhaseName = "T";
    }
    PhasorTable.row.add( [PhaseName, "Voltage", Round2(i * 120) + 'º', Round2(result.Voltage[i]) + '<em> V</em>'] ).draw(false);
    PhasorTable.row.add( [PhaseName, "Current", Round2(i * 120 + -result.Phi[i]) + 'º', Round2(result.Current[i]) + '<em> A</em>'] ).draw();
  }
  PhasorResizeEvent();
}
