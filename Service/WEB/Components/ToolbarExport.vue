<template>
  <q-btn stretch dense flat @click="Show" title="Export data to CSV file" :icon="mdiTable" />
  <q-dialog v-model="ShowDialog" no-backdrop-dismiss position="top">
    <q-card>
      <q-toolbar class="bg-grey-9 text-white">
        <q-toolbar-title>Export</q-toolbar-title>
      </q-toolbar>
      <q-card-section class="row q-px-sm q-col-gutter-x-sm">
        <q-input v-model="File" class="col-12" label="File name">
          <template v-slot:append>.CSV</template>
        </q-input>
        <div class="col-md-12 scrollpanel">
          <table class="table small">
            <tr>
              <td v-for="(val, idx) in Headers" :key="idx">{{ val }}</td>
            </tr>
            <tr v-for="(row, idx) in FilterRows" :key="idx">
              <td v-for="(val, idx2) in row" :key="idx2">{{ val }}</td>
            </tr>
          </table>
        </div>
      </q-card-section>
      <textarea type="text" ref="copyInput" style="max-height: 1px; position: absolute; top: -1000px;"></textarea>
      <template v-slot:footer>
        Document have {{ Rows.length }} rows.
      </template>

      <q-separator />
      <q-card-actions align="right" class="bg-grey-3">
        <q-btn unelevated color="primary" @click="Clipboard" label="Copy" />
        <q-btn unelevated color="primary" @click="Save" label="Save" />
        <q-btn unelevated color="primary" @click="ShowDialog=false" label="Cancel" />
        <!--toolbar-pager class="pull-right" :pageSize="100" :input="Rows" :output.sync="FilterRows"></toolbar-pager-->
      </q-card-actions>
    </q-card>
  </q-dialog>
</template>

<script>
  import { mdiTable } from '@quasar/extras/mdi-v6';
  export default {
    props: { rows: Function, file: String },
    data() {
      return { ShowDialog: false, File: '', Preview: 0, AllRows: null, Headers: null, Rows: null, FilterRows: null };
    },
    created: function() {
      this.mdiTable = mdiTable;
    },
    methods: {
      Show: function() {
        this.File = this.file;
        this.ShowDialog = true;
        this.Preview = false;
        this.AllRows = this.rows();
        this.Headers = this.AllRows.slice(0, 1)[0];
        this.Rows = this.AllRows.slice(1);
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
        for(var i in this.AllRows) csvFile += processRow(this.AllRows[i]);
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
