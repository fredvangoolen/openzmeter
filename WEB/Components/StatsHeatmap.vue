<template>
         <div id="wrapper">
      <div class="row">
        <div class="col-lg-12">
          <div class="panel panel-active-energy">
            <div id="StatsSpinner" class="spinnerdiv"></div>
            <div class="panel-heading">
              <span class="huge header-text">Energy Heatmap</span>
              <div class="header-buttons">
                <div style="display: inline-block; width: 140px" class="btn-group">
                  <div style="float: left" id="MapMin">?</div>
                  <div style="float: right" id="MapMax">?</div>
                  <div style="text-align: center"><em id="MapUnits"></em></div>
                  <div id="CalendarScale" style="height: 22px; width: 140px; display: flex; flex-direction:row; border: 1px solid #EEE"></div>
                </div>  
                <div class="btn-group" data-load-block="Years_Block"></div>
                <div class="btn-group" data-load-block="BlockSize_Block"></div>
                <div class="btn-group" data-load-block="PhaseV_Block" id="groupEnergyPhases"></div>
              </div>
              <span class="clearfix"> </span>
            </div>
                                 
            
            <div class="panel-body">
              <table id="HeatMap" style="width: 100%; font-size: 9px; text-align:center; table-layout: fixed">
                <thead style="font-weight: bold">
                  <tr>
                    <td style="width:16px">Day</td>
                    <td style="width:54px">Date</td>
                    <td colspan="6">00</td>
                    <td colspan="6">01</td>
                    <td colspan="6">02</td>
                    <td colspan="6">03</td>
                    <td colspan="6">04</td>
                    <td colspan="6">05</td>
                    <td colspan="6">06</td>
                    <td colspan="6">07</td>
                    <td colspan="6">08</td>
                    <td colspan="6">09</td>
                    <td colspan="6">10</td>
                    <td colspan="6">11</td>
                    <td colspan="6">12</td>
                    <td colspan="6">13</td>
                    <td colspan="6">14</td>
                    <td colspan="6">15</td>
                    <td colspan="6">16</td>
                    <td colspan="6">17</td>
                    <td colspan="6">18</td>
                    <td colspan="6">19</td>
                    <td colspan="6">20</td>
                    <td colspan="6">21</td>
                    <td colspan="6">22</td>
                    <td colspan="6">23</td>
                  </tr>
                </thead>
                <tbody>
                </tbody>
              </table>
            </div>
          </div>
        </div>
      </div>
         </div>
</template>

<script>
export default {
  name: 'Dashboard',
  data () {
    return {
      msg: 'Welcome to Your Vue.js App'
    }
  }
}
</script>

<style>
</style>
