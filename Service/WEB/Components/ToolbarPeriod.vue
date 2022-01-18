<template>
  <q-btn stretch flat dense size="sm">
    <div>
      <div class="block">Date range</div>
      <div class="block select-value">{{ Selected }}</div>
    </div>
    <q-menu anchor="bottom right" self="top right">
      <div class="row no-wrap q-pa-none">
        <q-date flat landscape range v-model="CustomRange">
          <div class="row items-center justify-end">
            <q-btn color="primary" label="Selected range" v-close-popup @click="SelectRange" :disable="CustomRange==null" />
          </div>
        </q-date>
        <q-separator vertical inset class="q-ma-none" />
        <div class="col">
          <q-list dense>
            <q-item clickable v-close-popup :active="modelValue == 'LastHours'" @click="Select('LastHours')">Last 2 hours</q-item>
            <q-item clickable v-close-popup :active="modelValue == 'LastDay'" @click="Select('LastDay')">Last 24 hours</q-item>
            <q-item clickable v-close-popup :active="modelValue == 'LastWeek'" @click="Select('LastWeek')">Last 7 days</q-item>
            <q-item clickable v-close-popup :active="modelValue == 'LastMonth'" @click="Select('LastMonth')">Last 30 days</q-item>
            <q-item clickable v-close-popup :active="modelValue == 'LastYear'" @click="Select('LastYear')">Last 365 days</q-item>
            <q-separator />
            <q-item clickable v-close-popup :active="modelValue == 'Day'" @click="Select('Day')">Today</q-item>
            <q-item clickable v-close-popup :active="modelValue == 'Week'" @click="Select('Week')">This week</q-item>
            <q-item clickable v-close-popup :active="modelValue == 'Month'" @click="Select('Month')">This month</q-item>
            <q-item clickable v-close-popup :active="modelValue == 'Quarter'" @click="Select('Quarter')">This quarter</q-item>
            <q-item clickable v-close-popup :active="modelValue == 'Year'" @click="Select('Year')">This year</q-item>
          </q-list>
        </div>
      </div>
    </q-menu>
  </q-btn>
</template>

<script>
  export default {
    props: { storage: String, modelValue: [String, Array] },
    data: function () {
      return { CustomRange: null };
    },
    computed: {
      Selected: function() {
        if(this.modelValue == 'LastHours') return 'Last 2 hours';
        if(this.modelValue == 'LastDay') return 'Last 24 hours';
        if(this.modelValue == 'LastDay') return 'Last 24 hours';
        if(this.modelValue == 'LastWeek') return 'Last 7 days';
        if(this.modelValue == 'LastMonth') return 'Last 30 days';
        if(this.modelValue == 'LastYear') return 'Last 365 days';
        if(this.modelValue == 'Day') return 'Today';
        if(this.modelValue == 'Week') return 'This week';
        if(this.modelValue == 'Month') return 'This month';
        if(this.modelValue == 'Year') return 'This year';
        return 'Custom';
      }
    },
    watch: {
      modelValue: function(val) {
        if((this.storage != null) && (this.modelValue != null)) localStorage.setItem(this.storage, JSON.stringify(val));
      }
    },
    created: function () {
      let Stored = 'Day'
      if(this.storage != null) {
        try {
          Stored = JSON.parse(localStorage.getItem(this.storage));
        } catch(e) {
          Stored = 'Day';
        }
        if(Array.isArray(Stored) == true) {
          if(Stored.length != 2) Stored = 'Day';
        } else if((Stored != 'LastHours') && (Stored != 'LastDay') && (Stored != 'LastDay') && (Stored != 'LastWeek') && (Stored != 'LastMonth') && (Stored != 'LastYear') && (Stored != 'Day') && (Stored != 'Week') && (Stored != 'Month') && (Stored != 'Year')) {
          Stored = 'Day';
        }
        if(JSON.stringify(this.value) != JSON.stringify(Stored)) this.$emit('update:modelValue', Stored);
      } else if(this.value == null) {
        this.$emit('update:modelValue', Stored);
      }
    },
    methods: {
      Select: function(val) {
        if(this.modelValue != val) this.$emit('update:modelValue', val);
      },
      SelectRange: function() {
        this.$emit('update:modelValue', [this.CustomRange.from, this.CustomRange.to]);
      }
    }
  }
</script>

<style scoped>
  .select-value {
    font-weight: bold;
    border-radius: 2px;
    background: rgba(255, 255, 255, 0.2);
    padding: 3px;
    margin: 0px;
  }
</style>