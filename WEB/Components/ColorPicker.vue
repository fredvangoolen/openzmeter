<template>
  <div class="btn-group">
    <div class="input-group input-group-xs">
      <span class="input-group-addon"><div class="colorbox" v-bind:style="{backgroundColor: Selected}"></div></span>
        <div class="input-group-btn">
          <button class="btn btn-default dropdown-toggle" type="button" data-toggle="dropdown"><span class="caret"></span></button>
          <ul class="dropdown-menu pull-right" v-bind:style="{ columns: columns }">
            <li v-for="color in colors"><a href="#"><div class="colorli" v-bind:style="{backgroundColor: color}" @click="ColorClick(color, $event)"></div></a></li>
          </ul>
        </div>
      </div>
    </div>
  
</template>

<script>
  export default {
    props: ['colors', 'selected', 'columns'],
    data() {
      return { Selected: '#FFFFFF' };
    },
    methods: {
      ColorClick(color, evt) {
        event.preventDefault();
        this.Selected = color;
        this.$emit('ColorChanged', color);
      }
    }, 
    created: function() {
      if(this.selected !== undefined) this.Selected = this.selected;
    }
  }
</script>

<style scoped>
  .colorbox {
    width: 30px;
    height: 14px;
    border: 1px solid #999;
  }
  ul {
    padding: 3px;
  }
  ul > li > a {
    padding: 3px 3px;
  }
  .colorli {
    width: 100%;
    height: 14px;
    border: 1px solid #999;
  }
</style>