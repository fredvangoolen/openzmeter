<template>
  <div class="btn-group">
    <div class="input-group input-group-xs">
      <div class="input-group-btn">
        <button class="btn btn-default narrow" type="button" @click="CalcView(-1)"><img :src="require('Imgs/resultset_previous.png')"></button>
      </div>
      <input type="text" class="form-control" placeholder="Filter..." v-model="Filter" :disabled="filtercols==null">
      <div class="input-group-btn">
        <button class="btn btn-default dropdown-toggle" type="button" data-toggle="dropdown"><span class="caret"></span></button>
        <ul class="dropdown-menu pull-right">
          <li class="dropdown-header text-center"> Filtered {{ Output.length }} of {{ input.length }} items<div v-if="DisplayRange!=null">{{ DisplayRange }}</div></li>
          <li v-for="(val, idx) in Pages" :key="idx" :class="{active: (val.value == Page)}"><a @click="Page = val.value; CalcView(0)">{{ val.name }}</a></li>
        </ul>
      </div>
      <div class="input-group-btn">
        <button class="btn btn-default narrow" type="button" @click="CalcView(+1)"><img :src="require('Imgs/resultset_next.png')"></button>
      </div>
    </div>
  </div>
</template>

<script>
  export default {
    props: { pageSize: Number, input: Array, output: Array, filtercols: Array },
    data: function() {
      return { Filter: '', Page: 0, Pages: [], Output: [] };
    },
    created: function() {
      this.FilterOutput();
    },
    watch: {
      Filter: function() { this.FilterOutput(); },
      input: function() { this.FilterOutput(); }
    },
    computed: {
      DisplayRange: function() {
        if(this.Output.length == 0) return null;
        var From = this.Page * this.pageSize + 1;
        var To = Math.min(this.Output.length, (this.Page + 1) * this.pageSize);
        return 'From ' + From + ' to ' + To;
      }
    },
    methods: {
      FilterOutput: function() {
        this.Pages = [];
        if(this.filtercols == null) {
          this.Output = this.input;
        } else {
          this.Output = [];
          this.Filter = this.Filter.toLowerCase();
          for(var row = 0; row < this.input.length; row++) {
            this.input[row].rowNum = row;
            for(var n = 0; n < this.filtercols.length; n++) {
              var Val = String(this.input[row][this.filtercols[n]]).toLowerCase();
              if(Val == undefined) continue;
              if(Val.includes(this.Filter)) {
                this.Output.push(this.input[row]);
                break;
              }
            }
          }
        }
        var Remain = this.Output.length;
        do {
          this.Pages.push({ name: 'Page ' + (this.Pages.length + 1), value: this.Pages.length });
          Remain = Remain - this.pageSize;
        } while(Remain > 0);
        this.CalcView(0);
      },
      CalcView: function(change) {
        this.Page = this.Page + change;
        if(this.Page < 0) this.Page = 0;
        if(this.Page > (this.Pages.length - 1)) this.Page = (this.Pages.length - 1);
        this.$emit('update:output', this.Output.slice(this.Page * this.pageSize, (this.Page + 1) * this.pageSize));
      }
    }
  }
</script>

<style scoped>
  .btn-group {
    width: 150px;
  }
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
  .dropdown-toggle {
    border-left: none;
  }
  .dropdown-toggle > span {
    margin-left: 1px;
  }
</style>