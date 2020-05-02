<template>
  <div class="day-config">
    <table class="stripe-table">
      <tr>
        <td colspan="4">00</td>
        <td colspan="4">01</td>
        <td colspan="4">02</td>
        <td colspan="4">03</td>
        <td colspan="4">04</td>
        <td colspan="4">05</td>
        <td colspan="4">06</td>
        <td colspan="4">07</td>
        <td colspan="4">08</td>
        <td colspan="4">09</td>
        <td colspan="4">10</td>
        <td colspan="4">11</td>
      </tr>
      <tr>
        <td v-for="(s, idx) in value.slice(0, 48)" :style="{ background: colors[s] }" :key="idx" @click="SetQuarter(idx)" @mousemove="SetQuarter(idx, $event)" data-toggle="tooltip" :title="names[s]"></td>
      </tr>
    </table>
    <table class="stripe-table">
      <tr>
        <td colspan="4">12</td>
        <td colspan="4">13</td>
        <td colspan="4">14</td>
        <td colspan="4">15</td>
        <td colspan="4">16</td>
        <td colspan="4">17</td>
        <td colspan="4">18</td>
        <td colspan="4">19</td>
        <td colspan="4">20</td>
        <td colspan="4">21</td>
        <td colspan="4">22</td>
        <td colspan="4">23</td>
      </tr>
      <tr>
        <td v-for="(s, idx) in value.slice(48, 96)" :style="{ background: colors[s] }" :key="idx" @click="SetQuarter(48 + idx)" @mousemove="SetQuarter(48 + idx, $event)" data-toggle="tooltip" :title="names[s]"></td>
      </tr>
    </table>
  </div>
</template>

<script>
  export default {
    props: { value: Array, colors: Array, names: Array, selected: Number },
    data: function() {
      return { Selected: 0 };
    },
    methods: {
      SetQuarter: function(idx, evt) {
        if((evt != null) && (evt.buttons == 0)) return;
        var Periods = this.value.slice(0);
        Periods[idx] = this.selected;
        this.$emit('input', Periods);
      }
    }
  }
</script>

<style scoped>
  .day-config {
    margin: 4px;
  }
  .stripe-table {
    width: 100%;
    table-layout: fixed;
    padding: 2px 4px;
    font-size: 11px;
    margin-bottom: 4px;
  }
  .stripe-table tr td {
    height: 14px;
    text-align: center;
    border: 1px solid #ccc;
    cursor: pointer;
    user-select: none;
  }
  .stripe-table tr:first-child td {
    height: 14px;
    cursor: auto;
  }
</style>