<template>
  <button type="button" class="btn btn-default btn-xs" @click="Show" data-toggle="tooltip" title="Export data to CSV file"><img :src="require('Imgs/table_export.png')">
    <dlg v-if="ShowDialog">
      <template v-slot:header>
        <div class="pull-left"><h4 class="modal-title">Export</h4></div>
        <div class="pull-right btn-group">
          <button type="button" class="btn btn-default btn-xs" @click="Clipboard"><img :src="require('Imgs/page_copy.png')"></button>
          <button type="button" class="btn btn-default btn-xs" @click="Save"><img :src="require('Imgs/diskette.png')"></button>
          <button type="button" class="btn btn-default btn-xs" @click="ShowDialog=false"><img :src="require('Imgs/arrow_undo.png')"></button>
        </div>
      </template>
      <div class="row">
        <div class="col-md-12">
          <label class="control-label">File name</label>
          <div class="input-group">
            <input type="text" class="form-control" v-model="File">
            <span class="input-group-addon">.CSV</span>
          </div>
        </div>
        <div class="col-md-12 scrollpanel">
          <button type="button" class="btn btn-default btn-sm" v-if="!Preview" @click="Preview=true"><img :src="require('Imgs/magnifier.png')">Load preview</button>
          <table v-if="Preview" class="table small">
            <tr v-for="(row, idx) in Rows" :key="idx">
              <td v-for="(val, idx2) in row" :key="idx2">{{ val }}</td>
            </tr>
          </table>
        </div>
      </div>
      <textarea type="text" ref="copyInput" style="max-height: 1px; position: absolute; top: -1000px;"></textarea>
      <template v-slot:footer>
      Document have {{ Rows.length }} rows.
      </template>
    </dlg>
  </button>
</template>

<script>
  export default {
    props: { rows: Function, file: String },
    data() {
      return { ShowDialog: false, File: '', Preview: false, Rows: null };
    },
    methods: {
      Show: function() {
        this.File = this.file;
        this.ShowDialog = true;
        this.Preview = false;
        this.Rows = this.rows();
      },
      FormatText: function() {
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
          return finalVal + '\r\n';
        };
        var csvFile = '';
        for(var i in this.Rows) csvFile += processRow(this.Rows[i]);
        return csvFile;
      },
      Clipboard: function() {
        this.$refs.copyInput.value =  this.FormatText();
        this.$refs.copyInput.focus();
        this.$refs.copyInput.select();
        document.execCommand("copy");
        this.ShowDialog = false;
      },
      Save: function() {
        var csvFile = this.FormatText();
        var blob = new Blob([csvFile], {type: 'text/csv;charset=utf-8;'});
        if(navigator.msSaveBlob) { // IE 10+
          navigator.msSaveBlob(blob, this.File + '.csv');
        } else {
          var link = document.createElement("a");
          if(link.download !== undefined) {
            var url = URL.createObjectURL(blob);
            link.setAttribute("href", url);
            link.setAttribute("download", this.File + '.csv');
            link.style.visibility = 'hidden';
            document.body.appendChild(link);
            link.click();
            document.body.removeChild(link);
          }
        }
        this.ShowDialog = false;
      }
    }
  }
</script>

<style scoped>
  .scrollpanel {
    margin-top: 5px;
    overflow: auto;
    max-height: 150px;
    padding: 0px 0px 0px 5px;
  }
  .scrollpanel button {
    width: 100%;
  }
  .scrollpanel table {
    margin-bottom: 0px;
    border: 1px solid #ccc;
    white-space: nowrap;
  }
  .scrollpanel table tr td {
    border: 1px solid #f5f5f5;
    text-align: right;
    font: 11px 'Fira Code';
    padding: 1px 3px;
  }
  .scrollpanel table tr:first-child td {
    font-weight: bold;
    background: #f5f5f5;
  }
</style>
