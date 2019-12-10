<template>
  <div>
    <div class="sidenav-version"><div>Version:<br/>{{ News.Version }}</div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" v-if="Updates!=null" v-on:click="ShowUpdates = true" v-bind:disabled="Updates.Updates.length == 0"><img v-bind:class="{blink_img: (Updates.Updates.length != 0)}" :src="require('Imgs/update.png')"></button>
        <button type="button" class="btn btn-default btn-xs" v-if="News!=null" v-on:click="FetchNews" v-bind:disabled="News.length == 0"><img :src="require('Imgs/newspaper.png')"></button>
      </div>
    </div>
    <dlg v-if="ShowNews" :loading="NewsLoading">
      <template v-slot:header>
        <div class="pull-left"><h4 class="modal-title">{{ NewsCurrent.Header }}</h4></div>
        <div class="pull-right btn-group">
          <div class="pull-right btn-group">
            <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Previous" :disabled="News.NewsPos <= 0" v-on:click="NewsPrev"><img :src="require('Imgs/resultset_previous.png')"></button>
            <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Next" :disabled="News.NewsPos == (News.News.length - 1)" v-on:click="NewsNext"><img :src="require('Imgs/resultset_next.png')"></button>
            <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Discard" v-on:click="ShowNews = false"><img :src="require('Imgs/arrow_undo.png')"></button>
          </div>
        </div>
        <div class="pull-right page-count">{{ News.NewsPos + 1}} of {{ News.News.length }}<br/>{{ FormatDate(NewsCurrent.NewsTime) }}</div>
      </template>
      <div class="news-contents" v-html="NewsCurrent.Contents"></div>
    </dlg>
    <dlg ref="Updates" v-if="ShowUpdates" :loading="Updating">
      <template v-slot:header>
        <div class="pull-left"><h4 class="modal-title">System updates</h4></div>
        <div class="pull-right btn-group">
          <button type="button" class="btn btn-default btn-xs btn-primary" v-if="!Updating" data-toggle="tooltip" title="Save changes" v-on:click="Upgrade">Upgrade</button>
          <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Discard" v-on:click="ShowUpdates = false"><img :src="require('Imgs/arrow_undo.png')"></button>
        </div>
        <div class="pull-right page-count">Last check<br/>{{ FormatDate(News.LastCheck) }}</div>
      </template>
      <vuetable ref="table" :data="Updates.Updates" :fields="Columns" :api-mode="false" class="table small table-striped"></vuetable>                  
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
      return { ShowNews: false, NewsLoading: false, NewsCurrent: null, News: { }, ShowUpdates: false, Updates: { Updates: [] }, Updating: false,
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
    created: function() {
      $.ajax({ url: "getNews", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ }), 
        success: (result) => {
          result.NewsPos = -1;
          if(isNaN(parseInt(localStorage.getItem('LastLogin'))) === true) localStorage.setItem('LastLogin', 0);
          var LastLogin = parseInt(localStorage.getItem('LastLogin'));
          for(var n = 0; n < result.News.length; n++) {
            if(result.News[n] > LastLogin) {
              result.NewsPos = n;
              this.FetchNews();
              break;
            }
          }
          if(result.NewsPos === -1) result.NewsPos = result.News.length - 1;
          this.News = result;
        }
      });
      this.FetchUpdates();
    },
    methods: {
      FormatDate: function(val) {
        return moment(val).format('DD/MM/YYYY');
      },
      FetchUpdates: function() {
        $.ajax({ url: "getUpdates", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ }),
          success: (result) => { this.Updates = result; }
        });
      },
      FetchNews: function() {
        this.ShowNews = true;
        this.NewsLoading = true;
        this.NewsCurrent = {};
        $.ajax({ url: "getNews", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ NewsTime: this.News.News[this.News.NewsPos] }),
          success: (result) => {
            this.NewsLoading = false;
            this.NewsCurrent = result;
            if(result.NewsTime > parseInt(localStorage.getItem('LastLogin'))) localStorage.setItem('LastLogin', result.NewsTime);
          }
        });
      },
      NewsPrev: function() {
        if(this.News.NewsPos > 0) this.News.NewsPos--;
        this.FetchNews();
      },
      NewsNext: function() {
        if(this.News.NewsPos < (this.News.News.length - 1)) this.News.NewsPos++;
        this.FetchNews();
      },
      Upgrade: function(row) {
        this.Upgrading = true;
        this.Updating = true;
        $.ajax({ url: "upgrade", timeout: 900000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ }),
          success: () => {
            this.Updating = false;
            this.FetchUpdates();
          },  
          error: () => {
            this.Updating = false;
          }
        });
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
    line-height: 13px;
    font-size: 12px;
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
  .blink_img {
    animation: blinker_img 1s linear infinite;
  }
  @keyframes blinker_img {  
    20% { opacity: 0.2; }
  }
</style>
