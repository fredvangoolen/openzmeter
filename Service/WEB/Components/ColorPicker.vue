<template>
  <div class="btn-group">
    <div class="input-group input-group-xs">
      <span class="form-control"><div class="colorbox" v-bind:style="{backgroundColor: Selected}"></div></span>
      <div class="input-group-btn">
        <button class="btn btn-default dropdown-toggle" type="button" data-toggle="dropdown"><span class="caret"></span></button>
        <ul class="dropdown-menu pull-right" v-bind:style="{ columns: columns }">
          <li v-for="color in colors" v-bind:class="{ active: (Selected == color)}"><a @click="ColorClick(color, $event)"><div class="colorli" v-bind:style="{backgroundColor: color}"></div></a></li>
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
    margin-top: 2px;
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