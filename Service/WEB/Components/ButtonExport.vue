<template>
  <button type="button" class="btn btn-default btn-xs" v-if="rows != []" @click="File=file; ShowDialog=true" data-toggle="tooltip" title="Export data to CSV file"><img :src="require('Imgs/table_export.png')">
    <dlg v-if="ShowDialog">
      <template v-slot:header>
        <div class="pull-left"><h4 class="modal-title">Export</h4></div>
        <div class="pull-right btn-group">
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
      </div>
      <template v-slot:footer>
      Document have {{ rows.length }} rows.
      </template>
    </dlg>
  </button>
</template>

<script>
  export default {
    props: { rows: { type: Array, required: true }, file: { type: String, default: 'Unnamed'} },
    data() {
      return { ShowDialog: false, File: '' };
    },
    methods: {
      Save: function() {
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
        for(var i in this.rows) csvFile += processRow(this.rows[i]);
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