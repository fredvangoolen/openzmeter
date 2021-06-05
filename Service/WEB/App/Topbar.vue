<template>
  <div>
    <div class="navbar-header">
      <button type="button" class="navbar-toggle" @mouseenter="$emit('expandmenu')" @click="$emit('expandmenu')"><img :src="require('Imgs/application_menu.png')"></button>
      <div class="navbar-brand">
        <img src="MainLogo.png">
      </div>
    </div>
    <div class="indicator-bar" v-if="analyzer != null" @click="Grouped = !Grouped">
      <topbar-indicator :analyzer="analyzer" serie="Voltage" :grouped="Grouped" :value="Transfer1.Data"></topbar-indicator>
      <topbar-indicator :analyzer="analyzer" serie="Current" :grouped="Grouped" :value="Transfer1.Data"></topbar-indicator>
      <topbar-indicator :analyzer="analyzer" serie="Active_Power" :grouped="Grouped" :value="Transfer1.Data"></topbar-indicator>
      <topbar-indicator :analyzer="analyzer" serie="Frequency" :value="Transfer1.Data" v-if="analyzer.NominalFrequency > 0.0"></topbar-indicator>
      <topbar-indicator :analyzer="analyzer" serie="Active_Energy" :grouped="Grouped" :value="EnergyUsage"></topbar-indicator>
    </div>
  </div>
</template>

<script>
  import TopbarIndicator from './TopbarIndicator.vue';
  export default {
    props: { analyzer: Object, expanded: Boolean, status: Object },
    data () {
      return { Grouped: false,
               Transfer1: this.NowTransfer(['Voltage', 'Current', 'Active_Power', 'Frequency']),
               Transfer2: this.SeriesTransfer(['Active_Energy'])
      };
    },
    created: function() {
      this.Transfer1.Update(this.analyzer);
      this.Transfer2.Update(this.analyzer, '15M', 'Day');
    },
    beforeDestroy: function() {
      this.Transfer1.Destroy();
      this.Transfer2.Destroy();
    },
    watch: {
      analyzer: function() {
        this.$nextTick(() => {
          this.Transfer1.Update(this.analyzer);
          this.Transfer2.Update(this.analyzer, '15M', 'Day');
        });
      }
    },
    computed: {
      EnergyUsage: function() {
        var Energy = new Array(this.analyzer.PhaseNames.length).fill(0);
        if(this.Transfer2.Data == null) return { Active_Energy: Energy };
        var EnergyCol = this.Transfer2.Data.Headers.indexOf('Active_Energy');
        for(var n = 0; n < this.Transfer2.Data.Rows.length; n++) {
          for(var p = 0; p < this.Transfer2.Data.Rows[n][EnergyCol].length; p++) Energy[p] = Energy[p] + this.Transfer2.Data.Rows[n][EnergyCol][p];
        }
        return { Active_Energy: Energy };
      }
    },
    components: { TopbarIndicator }
  }
</script>

<style scoped>
  .navbar-header {
    position: relative;
    background-color: #474544;
    width: 100%;
    z-index: 95;
    height: 56px;
    float: inherit;
  }
  .navbar-toggle {
    padding: 6px 5px;
    left: 0px;
    position: absolute;
    margin: 3px;
    color: #FFF;
    border: 1px solid #333;
    background-color: transparent;
    display: block;
    z-index: 100;
  }
  .navbar-toggle img {
    margin: 2px 0px;
  }
  .navbar-brand {
    padding: 0px 6px;
    margin-left: 48px;
  }
  @media(min-width: 1280px) {
    .navbar-toggle {
      display: none;
    }
    .navbar-brand {
      margin-left: 0px;
    }
  }
  .navbar-brand > img {
    margin-top: 14px;
    height: 28px;
    max-width: 428px;
  }
  .navbar-brand > div {
    font-size: 10px;
    text-align: center;
    color: #FFF;
    line-height: 16px;
    display: inline-block;
  }
  .navbar-brand > div > img {
    margin: 0px 2px;
  }
  .sys-stats {
    font-size: 10px;
    text-align: center;
    color: #FFF;
    width: 100%;
    line-height: 16px;
  }
  .sys-stats img {
    margin-right: 1px;
  }
  .indicator-bar {
    cursor: pointer;
    background-image: linear-gradient(to bottom,#f3f3f3,#e2e2e2);
    text-align: center;
    font-size: 14px;
    color: #FFF;
    font-weight: 700;
    padding-top: 2px;
    border-bottom: 1px solid #c9c9c9;
    position: relative;
    z-index: 90;
    top: 0px;
  }
  @media(min-width:840px) {
    .indicator-bar {
      background-image: none;
      position: absolute;
      right: 2px;
      text-align: right;
      z-index: 95;
      border: none;
    }
  }
  @media(min-width:1280px) {
    .indicator-bar {
      right: 2px;
    }
  }
</style>