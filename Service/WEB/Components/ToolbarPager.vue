<template>
  <div class="btn-group">
    <div class="input-group input-group-xs">
      <div class="input-group-btn">
        <button class="btn btn-default narrow" type="button" :disabled="value==0" @click="$emit('input', value-1)"><img :src="require('Imgs/resultset_previous.png')"></button>
      </div>
      <div class="input-group-btn">
        <button class="btn btn-default dropdown-toggle" type="button" data-toggle="dropdown">{{ SelectedText }}<span class="caret"></span></button>
        <ul class="dropdown-menu pull-right">
          <li v-for="(val, idx) in Pages" :key="idx" :class="{active: (val.value == value)}"><a @click="$emit('input', idx)">{{ val.name }}</a></li>
        </ul>
      </div>
      <div class="input-group-btn">
        <button class="btn btn-default narrow" type="button" :disabled="value==(Pages.length-1)" @click="$emit('input', value+1)"><img :src="require('Imgs/resultset_next.png')"></button>
      </div>
    </div>
  </div>
</template>

<script>
  export default {
    props: { pageSize: Number, total: Number, value: Number },
    data: function() {
      return { };
    },
    computed: {
      Pages: function() {
        var Page = [];
        var Remain = this.total;
        while(Remain > 0) {
          Page.push({ name: 'Page ' + (Page.length + 1), value: Page.length });
          Remain = Remain - this.pageSize;
        }
        return Page;
      },
      SelectedText: function() {
        var From = this.value * this.pageSize + 1;
        var To = Math.min(this.total, (this.value + 1) * this.pageSize);
        return 'From ' + From + ' to ' + To + ' of ' + this.total;
      }
    }
  }
</script>

<style scoped>
  .input-group-btn .narrow {
    padding: 1px 0px;
  }
  .dropdown-toggle {
    border-radius: 0px;
  }
  .btn-group ul {
    max-height: 142px;
    overflow-y: auto;
  }
</style>