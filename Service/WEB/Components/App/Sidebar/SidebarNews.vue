<template>
  <div>
    <div class="sidenav-version"><div>Build: {{ BuildDate }}</div>
      <button type="button" class="btn btn-default btn-xs pull-right" v-on:click="show" :disabled="News.length == 0"><img :src="require('Imgs/newspaper.png')"></button>
    </div>
    <div class="modal" tabindex="1" role="dialog" data-backdrop="false" v-if="ShowDialog" data-keyboard="false" ref="dialog">
      <div class="modal-backdrop in"></div>
      <div class="modal-dialog">
        <div class="modal-content">
          <div class="modal-header">
            <div class="pull-left"><h4 class="modal-title">{{ Title }}</h4></div>
            <div class="pull-right btn-group">
              <h5 class="modal-title"><span class="label label-success">{{NewsPos+1}}/{{News.length}}</span></h5>
              <div class="pull-right btn-group">
                <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Previous" :disabled="NewsPos == 0" v-on:click="Prev"><img :src="require('Imgs/resultset_previous.png')"></button>
                <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Next" :disabled="NewsPos == (News.length - 1)" v-on:click="Next"><img :src="require('Imgs/resultset_next.png')"></button>
                <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Discard" v-on:click="Back"><img :src="require('Imgs/arrow_undo.png')"></button>
              </div>
            </div>
          </div>
          <div class="modal-body">
            <spinner v-if="ShowSpinner"></spinner>
            <div v-html="Contents"></div>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
  import Spinner from 'Components/Spinner.vue';
  export default {
    data() {
      return { ShowSpinner: true, BuildDate: 'Wait...', ShowDialog: false, Title: '', Contents: '', News: [], NewsPos: -1};
    },
    mounted: function() {
      $.ajax({ url: "getNews", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ }), 
        success: function(result) {
          this.News = result.News;
          this.BuildDate = result.Version;
          if(isNaN(parseInt(localStorage.getItem('LastLogin'))) === true) localStorage.setItem('LastLogin', 0);
          var LastLogin = parseInt(localStorage.getItem('LastLogin'));
          for(var n = 0; n < this.News.length; n++) {
            if(this.News[n] > LastLogin) {
              this.NewsPos = n;
              this.show();
              break;
            }
          }
          if(this.NewsPos === -1) this.NewsPos = this.News.length - 1;
        }.bind(this)
      });
    },
    destroyed: function() {
      this.$root.$el.removeChild(this.$refs.dialog);
    },
    methods: {
      FetchNews: function() {
        this.ShowSpinner = true;
        $.ajax({ url: "getNews", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ NewsTime: this.News[this.NewsPos] }),
          success: function(result) {
            this.Title = result.Header;
            this.Contents = result.Contents;
            this.ShowSpinner = false;
            if(result.NewsTime > parseInt(localStorage.getItem('LastLogin'))) localStorage.setItem('LastLogin', result.NewsTime);
          }.bind(this)
        });
      },
      show: function(time) {
        this.ShowDialog = true;
        if(this.News.length === 0) return;
        this.FetchNews();
        this.$nextTick(function() {
          this.$root.$el.append(this.$refs.dialog);
          $(this.$refs.dialog).modal('show');
        });
      },
      Prev: function() {
        if(this.NewsPos > 0) this.NewsPos--;
        this.FetchNews();
      },
      Next: function() {
        if(this.NewsPos < (this.News.length - 1)) this.NewsPos++;
        this.FetchNews();
      },
      Back: function() {
        $(this.$refs.dialog).modal('hide');
        this.ShowDialog = false;
      },
    },
    components: {Spinner}
  }
</script>

<style scoped>
  .modal-body {
    min-height: 200px;
  }
  .sidenav-version {
    color: white;
    width: 100%;
    padding: 0px 10px;
    font-size: 12px;
  }
  .sidenav-version > div {
    width: 210px;
    line-height: 26px;
    font-size: 14px;
    display: inline-block;
  }
  .sidenav-version button {
    padding: 4px;
    background-color: transparent;
    border: 1px solid #333;
  }
  .sidenav-version > span {
    vertical-align: middle;
  }

</style>
