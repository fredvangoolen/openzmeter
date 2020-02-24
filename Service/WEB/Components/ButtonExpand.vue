<template>
<<<<<<< HEAD
  <button type="button" class="btn btn-default btn-xs" @click="$emit('maximize')" data-toggle="tooltip" title="Maximize graph">
    <img v-if="!ShowMaximized" :src="require('Imgs/arrow_out.png')">
    <img v-if="ShowMaximized" :src="require('Imgs/arrow_in.png')">
    <div class="modal" v-if="ShowMaximized" tabindex="1" role="dialog" data-backdrop="static" ref="MaximizeDialog" data-keyboard="false">
      <div class="modal-dialog full-screen-dialog">
        <div class="modal-content" style="height: 100%">
          <div class="modal-header">
            <div class="pull-left"><h4 class="modal-title">{{ Title }}</h4></div>
            <div class="pull-right clearfix" ref="Toolbar"></div>
          </div>
          <div class="modal-body" ref="Graph"></div>
        </div>
      </div>
    </div>
=======
  <button type="button" class="btn btn-default btn-xs" @click="$emit('input', !value)" data-toggle="tooltip" :title="value?'Minimize graph' : 'Maximize graph'">
    <img v-if="!value" :src="require('Imgs/arrow_out.png')">
    <img v-if="value" :src="require('Imgs/arrow_in.png')">
>>>>>>> eda5f001a831b33a1f525854c29e15e34550c1e4
  </button>
</template>

<script>
  export default {
<<<<<<< HEAD
    props: { },
    data() {
      return { UpdateInProgress: false, Now: false, ShowMaximized: false, Pause: false };
    },
    methods: {
      PlayClick: function() {
        this.Pause = false;
        $(this.$el).children('button').blur();
        this.$emit('PlayStatus', false);
      },
      SetPlayStatus(stat) {
        this.Pause = stat;
      },
      PauseClick: function() {
        this.Pause = true;
        $(this.$el).children('button').blur();
        this.$emit('PlayStatus', true);
      },
      ResizeHandler: function() {
        $(this.Graph.graphDiv).parent().removeAttr('style');
        this.Graph.resize($(this.$refs.MaximizeDialog).width() - 50, $(this.$refs.MaximizeDialog).height() - 80);
      },
      MinimizeClick: function() {
        $(window).unbind('resize', this.ResizeHandler);
        $(this.PrevToolbar).append(this.Toolbar);
        $(this.PrevGraph).append(this.Graph.graphDiv);
        $(this.$refs.MaximizeDialog).modal('hide');
        this.ShowMaximized = false;
        this.$emit('MinimizeClick');
        window.dispatchEvent(new Event('resize'));
      },
      Maximize: function(title, graph, toolbar) {
        this.Title = title;
        this.ShowMaximized = true;
        this.PrevToolbar = toolbar;
        this.PrevGraph = $(graph.graphDiv).parent();
        this.Toolbar = $(toolbar).children();
        this.Graph = graph;
        this.GraphObj = graph;
        this.$nextTick(function() {
          $(this.$refs.MaximizeDialog).modal('show');
          this.$root.$el.append(this.$refs.MaximizeDialog);
          $(this.$refs.Toolbar).append(this.Toolbar);
          $(this.$refs.Graph).append(this.Graph.graphDiv);
          $(window).bind('resize', this.ResizeHandler);
          this.$nextTick(function() { window.dispatchEvent(new Event('resize')); });
        });
      },
      SaveCVS: function(filename, rows) {
        var processRow = function (row) {
          var finalVal = '';
          for(var j = 0; j < row.length; j++) {
            var innerValue = row[j] === null ? '' : row[j].toString();
            if(row[j] instanceof Date) innerValue = row[j].toLocaleString();
            var result = innerValue.replace(/"/g, '""');
            if(result.search(/("|;|\n)/g) >= 0) result = '"' + result + '"';
            if(j > 0) finalVal += ';';
            finalVal += result;
          }
          return finalVal + '\n';
        };
        var csvFile = '';
        for(var i = 0; i < rows.length; i++) csvFile += processRow(rows[i]);
        var blob = new Blob([csvFile], {type: 'text/csv;charset=utf-8;'});
        if(navigator.msSaveBlob) { // IE 10+
          navigator.msSaveBlob(blob, filename);
        } else {
          var link = document.createElement("a");
          if(link.download !== undefined) {
            var url = URL.createObjectURL(blob);
            link.setAttribute("href", url);
            link.setAttribute("download", filename);
            link.style.visibility = 'hidden';
            document.body.appendChild(link);
            link.click();
            document.body.removeChild(link);
          }
        }
      }
    },
    destroyed: function() {
      if(this.ShowMaximized) $(this.$root).removeChild(this.$refs.MaximizeDialog);
    },
    created: function() {
      this.Paused = this.paused;
      this.Graph = null;
      this.Toolbar = null;
      this.PrevToolbar = null;
      this.PrevGraph = null;
    }
  }
</script>
=======
    props: { value: Boolean },
    data() {
      return { };
    },
    methods: {
//      PlayClick: function() {
//        this.Pause = false;
//        $(this.$el).children('button').blur();
//        this.$emit('PlayStatus', false);
//      },
//      SetPlayStatus(stat) {
//        this.Pause = stat;
//      },
//      PauseClick: function() {
//        this.Pause = true;
//        $(this.$el).children('button').blur();
//        this.$emit('PlayStatus', true);
//      },
//      ResizeHandler: function() {
//        $(this.Graph.graphDiv).parent().removeAttr('style');
//        this.Graph.resize($(this.$refs.MaximizeDialog).width() - 50, $(this.$refs.MaximizeDialog).height() - 80);
//      },
//      MinimizeClick: function() {
//        $(window).unbind('resize', this.ResizeHandler);
//        $(this.PrevToolbar).append(this.Toolbar);
//        $(this.PrevGraph).append(this.Graph.graphDiv);
//        $(this.$refs.MaximizeDialog).modal('hide');
//        this.ShowMaximized = false;
//        this.$emit('MinimizeClick');
//        window.dispatchEvent(new Event('resize'));
//      },
//      Maximize: function(title, graph, toolbar) {
//        this.Title = title;
//        this.ShowMaximized = true;
//        this.PrevToolbar = toolbar;
//        this.PrevGraph = $(graph.graphDiv).parent();
//        this.Toolbar = $(toolbar).children();
//        this.Graph = graph;
//        this.GraphObj = graph;
//        this.$nextTick(function() {
//          $(this.$refs.MaximizeDialog).modal('show');
//          this.$root.$el.append(this.$refs.MaximizeDialog);
//          $(this.$refs.Toolbar).append(this.Toolbar);
//          $(this.$refs.Graph).append(this.Graph.graphDiv);
//          $(window).bind('resize', this.ResizeHandler);
//          this.$nextTick(function() { window.dispatchEvent(new Event('resize')); });
//        });
//      },
//      SaveCVS: function(filename, rows) {
//        var processRow = function (row) {
//          var finalVal = '';
//          for(var j = 0; j < row.length; j++) {
//            var innerValue = row[j] === null ? '' : row[j].toString();
//            if(row[j] instanceof Date) innerValue = row[j].toLocaleString();
//            var result = innerValue.replace(/"/g, '""');
//            if(result.search(/("|;|\n)/g) >= 0) result = '"' + result + '"';
//            if(j > 0) finalVal += ';';
//            finalVal += result;
//          }
//          return finalVal + '\n';
//        };
//        var csvFile = '';
//        for(var i = 0; i < rows.length; i++) csvFile += processRow(rows[i]);
//        var blob = new Blob([csvFile], {type: 'text/csv;charset=utf-8;'});
//        if(navigator.msSaveBlob) { // IE 10+
//          navigator.msSaveBlob(blob, filename);
//        } else {
//          var link = document.createElement("a");
//          if(link.download !== undefined) {
//            var url = URL.createObjectURL(blob);
//            link.setAttribute("href", url);
//            link.setAttribute("download", filename);
//            link.style.visibility = 'hidden';
//            document.body.appendChild(link);
//            link.click();
//            document.body.removeChild(link);
//          }
//        }
//      }
    },
//    destroyed: function() {
//      if(this.ShowMaximized) $(this.$root).removeChild(this.$refs.MaximizeDialog);
//    },
//    created: function() {
//      this.Paused = this.paused;
//      this.Graph = null;
//      this.Toolbar = null;
//      this.PrevToolbar = null;
//      this.PrevGraph = null;
//    }
  }
</script>

<style scoped>

</style>
>>>>>>> eda5f001a831b33a1f525854c29e15e34550c1e4
