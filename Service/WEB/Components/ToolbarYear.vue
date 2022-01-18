<template>
  <q-btn stretch flat dense size="sm">
    <div>
      <div class="block">Year</div>
      <div class="select-value">{{ Selected }}</div>
    </div>
    <q-menu auto-close anchor="bottom middle" self="top middle">
      <q-list dense>
        <q-item clickable v-for="(option, idx) in Options" :active="modelValue == option.value" :key="idx" @click="Select(option.value)">{{ option.name }}</q-item>
      </q-list>
    </q-menu>
  </q-btn>
</template>

<script>
  export default {
    props: { aggreg: String, modelValue: Number, storage: String },
    data() {
      return { };
    },
    computed: {
      Selected: function() {
        for(var idx in this.Options) {
          if(this.Options[idx].value == this.modelValue) return this.Options[idx].name;
        }
        return '?';
      },
      Options: function() {
        let Range = null;
        let Options = [];
        let Values = [];
        if(this.aggreg == '15M') Range = this.Analyzer.Ranges.Aggreg15M;
        if(this.aggreg == '1H') Range = this.Analyzer.Ranges.Aggreg1H;
        if((Range == null) || (this.Analyzer.Analyzer == null)) return Options;
        let StartYear = this.$dayjs.tz(Range.From, this.Analyzer.Timezone).year();
        let EndYear = this.$dayjs.tz(Range.To, this.Analyzer.Timezone).year();
        let Found = false;
        for(let year = StartYear; year <= EndYear; year++) {
          Options.push({ value: year, name: String(year) });
          Values.push(year);
          if(this.modelValue == year) Found = true;
        }
        Options.reverse();
        this.Select(this.GetConfig(this.storage, Found ? this.modelValue : Values[0], Values));
        return Options;
      }
    },
    watch: {
      modelValue: function() {
        if((this.storage != null) && (this.modelValue != null)) localStorage.setItem(this.storage, this.modelValue);
      }
    },
    methods: {
      Select: function(val) {
        if(this.modelValue != val) this.$emit('update:modelValue', val);
        console.log(val);
      }
    }
  }
</script>

<style scoped>
  .select-value {
    font-weight: bold;
    border-radius: 2px;
    background: rgba(255, 255, 255, 0.2);
    padding: 2px 3px 3px 4px;
    margin: 1px;
  }
</style>