<template>
  <div>
    <div class="sidenav-version"><div>Build: {{ BuildDate }}</div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" v-on:click="ShowUpdates" :disabled="Updates.length == 0"><img :src="require('Imgs/update.png')"></button>
        <button type="button" class="btn btn-default btn-xs" v-on:click="ShowNews" :disabled="News.length == 0"><img :src="require('Imgs/newspaper.png')"></button>
      </div>
    </div>
    <dlg ref="News">
      <template v-slot:header>
        <div class="pull-left"><h4 class="modal-title">{{ Title }}</h4></div>
        <div class="pull-right btn-group">
          <div class="pull-right btn-group">
            <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Previous" :disabled="NewsPos == 0" v-on:click="NewsPrev"><img :src="require('Imgs/resultset_previous.png')"></button>
            <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Next" :disabled="NewsPos == (News.length - 1)" v-on:click="NewsNext"><img :src="require('Imgs/resultset_next.png')"></button>
            <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Discard" v-on:click="NewsBack"><img :src="require('Imgs/arrow_undo.png')"></button>
          </div>
        </div>
        <div class="pull-right page-count">{{ NewsPos + 1}} of {{ News.length }}<br/>{{ NewsDate }}</div>
      </template>
      <div class="news-contents" v-html="Contents"></div>
    </dlg>
    <dlg ref="Updates">
      <template v-slot:header>
        <div class="pull-left"><h4 class="modal-title">System updates</h4></div>
        <div class="pull-right btn-group" v-if="!Upgrading">
          <button type="button" class="btn btn-default btn-xs btn-primary" data-toggle="tooltip" title="Save changes" v-on:click="Upgrade">Upgrade</button>
          <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Discard" v-on:click="UpdatesBack"><img :src="require('Imgs/arrow_undo.png')"></button>
        </div>
        <div class="pull-right page-count">Last check<br/>{{ UpdateDate }}</div>
      </template>
      <div v-if="Upgrading" class="alert alert-info text-center" style="margin-bottom: 5px; padding: 5px">Upgrade can take some time to complete.<br/>Reload page in a few minutes.</div>
      <vuetable v-if="!Upgrading" :data="Updates" :fields="Columns" :api-mode="false" class="table small table-striped">
        <template slot="actions" slot-scope="props">
          <div class="btn-group">
            <button type="button" class="btn btn-default btn-xs" v-on:click="Edit(props.rowData)">Edit</button>
            <button type="button" class="btn btn-danger btn-xs" v-on:click="Delete(props.rowData)">Delete</button>
          </div>
        </template>
      </vuetable>                  
    </dlg>
  </div>
</template>

<script>
  import Dlg from 'Components/Dialog.vue';
  import moment from 'JS/moment.js';
  import vuetable from 'vuetable-2';
  import 'CSS/table-responsive.css';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    data() {
      return { BuildDate: 'Wait...', ShowDialog: false, Title: '', Contents: '', NewsDate: '', News: [], NewsPos: -1, Updates: [], UpdateDate: '', Upgrading: false,
        Columns: [
          {name: 'Package', title: 'Package'},
          {name: 'From', title: 'From'},
          {name: 'To', title: 'To'}
        ]
      };
    },
    updated: function(){
      oZm.createCaptionsForResponsiveTable(this.$refs.table);
    },
    mounted: function() {
      $.ajax({ url: "getNews", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ }), 
        success: (result) => {
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
        }
      });
      $.ajax({ url: "getUpdates", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ }),
        success: (result) => {
          this.Updates = result.Updates;
          this.UpdateDate = moment(result.LastCheck).format('DD/MM/YYYY');
          if(this.Updates.length > 0) this.ShowUpdates();
        }
      });
    },
    methods: {
      FetchNews: function() {
        this.$refs.News.SpinnerState(true);
        $.ajax({ url: "getNews", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ NewsTime: this.News[this.NewsPos] }),
          success: function(result) {
            this.Title = result.Header;
            this.Contents = result.Contents;
            this.NewsDate = moment(this.News[this.NewsPos]).format('DD/MM/YYYY');
            this.$refs.News.SpinnerState(false);
            if(result.NewsTime > parseInt(localStorage.getItem('LastLogin'))) localStorage.setItem('LastLogin', result.NewsTime);
          }.bind(this)
        });
      },
      ShowNews: function(time) {
        if(this.News.length === 0) return;    
        this.$refs.News.Show();
        this.FetchNews();
      },
      NewsPrev: function() {
        if(this.NewsPos > 0) this.NewsPos--;
        this.FetchNews();
      },
      NewsNext: function() {
        if(this.NewsPos < (this.News.length - 1)) this.NewsPos++;
        this.FetchNews();
      },
      NewsBack: function() {
        this.$refs.News.Close();
      },
      ShowUpdates: function() {
        this.$refs.Updates.Show();
      },
      UpdatesBack: function() {
        this.$refs.Updates.Close();
      },
      Upgrade: function() {
        this.Upgrading = true;
        $.ajax({ url: "upgrade", timeout: 200000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ })});
      }
    },
    components: { vuetable, Dlg }
  }
</script>

<style scoped>
  .news-contents {
    min-height: 200px;
  }
  .sidenav-version {
    color: white;
    width: 100%;
    padding: 0px 10px;
    font-size: 12px;
  }
  .sidenav-version > div {
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
  .page-count {
    line-height: 11px;
    font-size: 12px;
    text-align: center;
  }
  @media screen and (min-width: 600px) {
    .table >>> th:nth-child(2), .table >>> th:nth-child(3)  {
      text-align: center;
    }
    .table >>> td:nth-child(2), .table >>> td:nth-child(3) {
      text-align: center;
    }
  }
</style>
