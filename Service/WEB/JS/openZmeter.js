const openZmeter = {
  install: function(Vue, options) {
    Vue.prototype.PriceTransfer = (series) => {
      var Transfer = { SingleShot: false, Timer: null, CancelFunc: null, Data: null, Periods: null, Bills: null, Series: series, Analyzer: null, Aggreg: null, Range: null, AggregFail: null };
      Transfer.Destroy = () => {
        if(Transfer.Timer != null) { clearTimeout(Transfer.Timer); Transfer.Timer = null; }
        if(Transfer.CancelFunc != null) { Transfer.CancelFunc(); Transfer.CancelFunc = null; }
      };
      Transfer.SetData = (range, aggreg, data) => {
        if(data == null) {
          Transfer.Periods = null;
          Transfer.Data = null;
          Transfer.Bills = null;
          return;
        }
        var Data = { Headers: data[0], Rows: data.slice(1), Aggreg: aggreg, From: range[0], To: range[1] };
        Transfer.Data = Object.seal(Data);
        var Bills = null;
        if(Transfer.Series.includes('BILL')) {
          Bills = [];
          var BillCol = Data.Headers.indexOf('Bill');
          var TimeCol = Data.Headers.indexOf('Sampletime');
          for(var i in Data.Rows) {
            if(Data.Rows[i][BillCol] == true) Bills.push(Data.Rows[i][TimeCol]);
          }
        }
        Transfer.Bills = Object.seal(Bills);
        var Periods = null;
        if(Transfer.Series.includes('COLOR') || Transfer.Series.includes('PERIOD')) {
          Periods = { Headers: [], Rows: [], Aggreg: aggreg };
          Periods.Headers.push('Sampletime');
          if(Transfer.Series.includes('PERIOD')) Periods.Headers.push('Period');
          if(Transfer.Series.includes('COLOR')) Periods.Headers.push('Color');
          var PeriodCol = Data.Headers.indexOf('Period');
          var ColorCol = Data.Headers.indexOf('Color');
          var TimeCol = Data.Headers.indexOf('Sampletime');
          var LastItem = [];
          if(PeriodCol != -1) LastItem.push(null);
          if(ColorCol != -1) LastItem.push(null);
          Periods.Rows.push([range[0]].concat(LastItem));
          for(var i in Data.Rows) {
            var Row = [];
            if(PeriodCol != -1) Row.push((Data.Rows[i][PeriodCol] == null) ? null : 'P' + Data.Rows[i][PeriodCol]);
            if(ColorCol != -1) Row.push(Data.Rows[i][ColorCol]);
            if(JSON.stringify(LastItem) == JSON.stringify(Row)) continue;
            LastItem = Row;
            Periods.Rows.push([Data.Rows[i][TimeCol]].concat(LastItem));
          }
          if(LastItem.length > 0) Periods.Rows.push([Data.Rows.slice(-1)[0][TimeCol]].concat(LastItem));
          LastItem = [];
          if(PeriodCol != -1) LastItem.push(null);
          if(ColorCol != -1) LastItem.push(null);
          Periods.Rows.push([range[1]].concat(LastItem));
        }
        Transfer.Periods = Object.seal(Periods);
      };
      Transfer.Update = (analyzer, aggreg, range) => {
        if((analyzer == null) || (aggreg == null) || (range == null)) return;
        var Cancelled = false;
        var ReqRange  = analyzer.CalcRange(range, aggreg);
        if(Transfer.Timer != null) { clearTimeout(Transfer.Timer); Transfer.Timer = null; }
        if(Transfer.CancelFunc != null) { Transfer.CancelFunc(); Transfer.CancelFunc = null; }
        if(aggreg == '1H') range[0] = (range[0] / 3600000) * 3600000;
        if((Transfer.Aggreg != aggreg) || (Transfer.Analyzer == null) || (Transfer.Analyzer.Analyzer != analyzer.Analyzer) || (Transfer.Range != range)) Transfer.SetData(null, null, null);
        Transfer.Analyzer  = analyzer;
        Transfer.Aggreg    = aggreg;
        Transfer.Range     = range;
        Transfer.CancelFunc = () => { Cancelled = true; };
        Vue.http.post('getCosts', { Analyzer: analyzer.Analyzer, Aggreg: aggreg, From: ReqRange[0], To: ReqRange[1], Series: series, Group: 0 }).then(
          (result) => {
            if(Cancelled == true) return;
            Transfer.SetData(ReqRange, aggreg, result.body);
            Transfer.Timer = setTimeout(() => { Transfer.Update(Transfer.Analyzer, Transfer.Aggreg, Transfer.Range); }, 900000);
          },
          (fail) => {
            if(Cancelled == true) return;
            Transfer.Timer = setTimeout(() => { Transfer.Update(Transfer.Analyzer, Transfer.Aggreg, Transfer.Range); }, 900000);
          }
        );
      };
      return Transfer;
    };
    Vue.prototype.StatusTransfer = (series) => {
      var Transfer = { SingleShot: false, Timer: null, CancelFunc: null, Data: null, Series: series, Analyzer: null };
      Transfer.Destroy = () => {
        if(Transfer.Timer != null) { clearTimeout(Transfer.Timer); Transfer.Timer = null; }
        if(Transfer.CancelFunc != null) { Transfer.CancelFunc(); Transfer.CancelFunc = null; }
      };
      Transfer.Update = (analyzer) => {
        if(analyzer == null) return;
        var Cancelled = false;
        if(Transfer.Timer != null) { clearTimeout(Transfer.Timer); Transfer.Timer = null; }
        if(Transfer.CancelFunc != null) { Transfer.CancelFunc(); Transfer.CancelFunc = null; }
        if((Transfer.Analyzer == null) || (Transfer.Analyzer.Analyzer != analyzer.Analyzer)) Transfer.Data = null;
        Transfer.Analyzer  = analyzer;
        Transfer.CancelFunc = () => { Cancelled = true; };
        Vue.http.post('getStatus', { Analyzer: analyzer.Analyzer, Series: series }).then(
          (result) => {
            if(Cancelled == true) return;
            Transfer.Data = Object.seal(result.body);
            Transfer.Timer = setTimeout(() => { Transfer.Update(Transfer.Analyzer); }, 3000);
          },
          (fail) => {
            if(Cancelled == true) return;
            Transfer.Timer = setTimeout(() => { Transfer.Update(Transfer.Analyzer); }, 3000);
          }
        );
      };
      return Transfer;
    };
    Vue.prototype.NowTransfer = (series) => {
      var Transfer = { Timer: null, CancelFunc: null, Data: null, Series: series, Analyzer: null };
      Transfer.Destroy = () => {
        if(Transfer.Timer != null) { clearTimeout(Transfer.Timer); Transfer.Timer = null; }
        if(Transfer.CancelFunc != null) { Transfer.CancelFunc(); Transfer.CancelFunc = null; }
      };
      Transfer.Update = (analyzer) => {
        if(analyzer == null) return;
        var Cancelled = false;
        if(Transfer.Timer != null) { clearTimeout(Transfer.Timer); Transfer.Timer = null; }
        if(Transfer.CancelFunc != null) { Transfer.CancelFunc(); Transfer.CancelFunc = null; }
        if((Transfer.Analyzer != null) && (Transfer.Analyzer.Analyzer != analyzer.Analyzer)) Transfer.Data = null;
        Transfer.Analyzer  = analyzer;
        Transfer.CancelFunc = () => { Cancelled = true; };
        Vue.http.post('getSeriesNow', { Analyzer: analyzer.Analyzer, Series: series }).then(
          (result) => {
            if(Cancelled == true) return;
            Transfer.Data = Object.seal(result.body);
            Transfer.Timer = setTimeout(() => { Transfer.Update(Transfer.Analyzer); }, 1500);
          },
          (fail) => {
            if(Cancelled == true) return;
            Transfer.Data = null;
            Transfer.Timer = setTimeout(() => { Transfer.Update(Transfer.Analyzer); }, 1500);
          }
        );
      };
      return Transfer;
    };
    Vue.prototype.SeriesTransfer = (series) => {
      var Transfer = { Timer: null, CancelFunc: null, Data: null, LastData: null, UpdateUnread: false, Periods: null, Series: series, Analyzer: null, Aggreg: null, Range: null, AggregFail: null };
      Transfer.Destroy = () => {
        if(Transfer.Timer != null) { clearTimeout(Transfer.Timer); Transfer.Timer = null; }
        if(Transfer.CancelFunc != null) { Transfer.CancelFunc(); Transfer.CancelFunc = null; }
      };
      Transfer.SetData = (range, aggreg, data) => {
        if(data == null) {
          Transfer.Data = null;
          Transfer.LastData = null;
          return;
        }
        var Data = { Headers: data[0], Rows: data.slice(1), Aggreg: aggreg, From: range[0], To: range[1]};
        Transfer.Data = Object.seal(Data);
        Transfer.LastData = (Transfer.Data.Rows.length == 0) ? null : Object.seal(Transfer.Data.Rows.slice(-1)[0]);
      };
      Transfer.Update = (analyzer, aggreg, range) => {
        if((analyzer == null) || (aggreg == null) || (range == null)) return;
        var Promises  = [];
        var Cancelled = false;
        var ReqRange  = analyzer.CalcRange(range, aggreg);
//        var Period    = (aggreg == '200MS') ? 200 : (aggreg == '3S') ? 3000 : (aggreg == '1M') ? 60000 : (aggreg == '10M') ? 600000 : (aggreg == '15M') ? 900000 : 3600000;
        if(Transfer.Timer != null) { clearTimeout(Transfer.Timer); Transfer.Timer = null; }
        if(Transfer.CancelFunc != null) { Transfer.CancelFunc(); Transfer.CancelFunc = null; }
        if((Transfer.Aggreg != aggreg) || (Transfer.Analyzer == null) || (Transfer.Analyzer.Analyzer != analyzer.Analyzer) || (Transfer.Range != range)) Transfer.SetData(null, null, null);
        while((Transfer.Data != null) && (Transfer.Data.Rows.length > 0) && (Transfer.Data.Rows[0][0] < ReqRange[0])) Transfer.Data.Rows.shift();
        while((Transfer.Data != null) && (Transfer.Data.Rows.length > 0) && (Transfer.Data.Rows.slice(-1)[0] > ReqRange[1])) Transfer.Data.Rows.pop();
        Transfer.Analyzer = analyzer;
        Transfer.Aggreg   = aggreg;
        Transfer.Range    = range;
        if((Transfer.Data == null) || (Transfer.Data.Rows.length == 0)) {
          Promises.push(Vue.http.post('getSeries', { Analyzer: analyzer.Analyzer, Aggreg: aggreg, From: ReqRange[0], To: ReqRange[1], Series: series }));
        } else {
          Promises.push(Vue.http.post('getSeries', { Analyzer: analyzer.Analyzer, Aggreg: aggreg, From: ReqRange[0], To: Transfer.Data.Rows[0][0] - 1, Series: series }));
          Promises.push(Vue.http.post('getSeries', { Analyzer: analyzer.Analyzer, Aggreg: aggreg, From: Transfer.Data.Rows.slice(-1)[0][0] + 1, To: ReqRange[1], Series: series }));
        }
        Transfer.CancelFunc = () => { Cancelled = true; };
        Promise.all(Promises).then(
          (result) => {
            if(Cancelled == true) return;
            var Data = result[0].body;
            if(Transfer.Data != null) Data = Data.concat(Transfer.Data.Rows);
            if((result.length == 2) && (result[1].body.length > 1)) Data = Data.concat(result[1].body.slice(1));
            Transfer.SetData(ReqRange, aggreg, Data);
            Transfer.Timer = setTimeout(() => { Transfer.Update(Transfer.Analyzer, Transfer.Aggreg, Transfer.Range); }, (aggreg == "200MS") ? 0 : 3000);
          },
          (fail) => {
            if(Cancelled == true) return;
            Transfer.Timer = setTimeout(() => { Transfer.Update(Transfer.Analyzer, Transfer.Aggreg, Transfer.Range); }, (aggreg == "200MS") ? 0 : 3000);
          }
        );
      };
      return Transfer;
    },
































    Vue.prototype.CreateSeriesTransfer = (parent, series) => {
      var Transfer = { This: parent, SingleShot: false, Timer: null, CancelFunc: null, Data: null, LastData: null, UpdateUnread: false, Periods: null, Series: series, Analyzer: null, Aggreg: null, Range: null, AggregFail: null };
      Transfer.Destroy = () => {
        if(Transfer.Timer != null) { clearTimeout(Transfer.Timer); Transfer.Timer = null; }
        if(Transfer.CancelFunc != null) { Transfer.CancelFunc(); Transfer.CancelFunc = null; }
      };

      Transfer.SetData = (data) => {
        Transfer.Data = Object.seal(data);
        if((data == null) || (data.Sampletime.length == 0)) {
          Transfer.LastData = null;
        } else {
          var LastData = {};
          for(var n in data) {
            if(Array.isArray(data[n])) LastData[n] = data[n].slice(-1)[0];
          }
          Transfer.LastData = Object.seal(LastData);
        }
      };
      Transfer.UpdateAlarms = (analyzer, range) => {
        if((analyzer == null) || (range == null)) return;
        var Cancelled = false;
        if(Transfer.Timer != null) { clearTimeout(Transfer.Timer); Transfer.Timer = null; }
        if(Transfer.CancelFunc != null) { Transfer.CancelFunc(); Transfer.CancelFunc = null; }
        var ReqRange = analyzer.CalcRange(range);
        var Data = { };
        if((Transfer.Analyzer != null) && (Transfer.Analyzer.Analyzer == analyzer.Analyzer)) Data = Object.assign(Data, Transfer.Data);
        if((Transfer.Analyzer == null) || (Transfer.Analyzer.Analyzer != analyzer.Analyzer) || (Transfer.Range != range)) Transfer.Data = null;
        Transfer.Analyzer  = analyzer;
        Transfer.Range     = range;
        Data.From   = null;
        Data.To     = null;
        if(Data.AlarmTime != null) {
          var Pos = 0;
          for(var n = 0; n < Data.AlarmTime.length; n++) {
            if(Data.AlarmTime[n] < ReqRange[0])
              Pos++;
            else
              break;
          }
          if(Pos != 0) {
            for(var serie in Data) {
              if(Array.isArray(Data[serie]) == true) Data[serie].splice(0, Pos);
            }
          }
          var Pos = 0;
          for(var n = 0; n < Data.AlarmTime.length; n++) {
            if(Data.AlarmTime[n] <= ReqRange[1])
              Pos++;
            else
              break;
          }
          if(Pos != 0) {
            for(var serie in Data) {
              if(Array.isArray(Data[serie]) == true) Data[serie].splice(Pos);
            }
          }
          Data.From = Data.AlarmTime[0];
          Data.To = Data.AlarmTime.slice(-1)[0];
        }
        var Promises = [];
        if((Data.From == null) || (Data.To == null)) {
          Promises.push(Transfer.This.$http.post('getAlarms', { Analyzer: analyzer.Analyzer, From: ReqRange[0], To: ReqRange[1], Update: Transfer.UpdateUnread }));
          Transfer.UpdateUnread = false;
        } else {
          if(ReqRange[0] < Data.From) Promises.push(Transfer.This.$http.post('getAlarms', { Analyzer: analyzer.Analyzer, From: ReqRange[0], To: Data.From - 1, Series: series }));
          if(ReqRange[1] > Data.To) Promises.push(Transfer.This.$http.post('getAlarms', { Analyzer: analyzer.Analyzer, From: Data.To + 1, To: ReqRange[1], Series: series }));
        }
        if(Promises.length == 0) {
          Data.From = ReqRange[0];
          Data.To = ReqRange[1];
          Transfer.Data = Object.seal(Data);
          if(Transfer.SingleShot == false) Transfer.Timer = setTimeout(() => { Transfer.UpdateAlarms(Transfer.Analyzer, Transfer.Range); }, 3000);
          return;
        }
        Transfer.CancelFunc = () => { Cancelled = true; };
        Promise.all(Promises).then(
          (result) => {
            if(Cancelled == true) return;
            for(var n in result) {
              var After = ((result[n].body.AlarmTime.length > 0) && (Data.From != null) && (Data.From > result[n].body.AlarmTime[0]));
              for(var serie in result[n].body) {
                if(Array.isArray(result[n].body[serie]) == false) continue
                if((Data.From == null) || (Data.To == null))
                  Data[serie] = result[n].body[serie];
                else if(After == true)
                  Data[serie] = result[n].body[serie].concat(Data[serie]);
                else
                  Data[serie] = Data[serie].concat(result[n].body[serie]);
              }
            }
            Data.From = ReqRange[0];
            Data.To = ReqRange[1];
            Transfer.Data = Object.seal(Data);
            if(Transfer.SingleShot == false) Transfer.Timer = setTimeout(() => { Transfer.UpdateAlarms(Transfer.Analyzer, Transfer.Range); }, 3000);
          },
          (fail) => {
            if(Cancelled == true) return;
            if(Transfer.SingleShot == false) Transfer.Timer = setTimeout(() => { Transfer.UpdateAlarms(Transfer.Analyzer, Transfer.Range); }, 3000);
          }
        );
      };
      Transfer.UpdateEvents = (analyzer, range) => {
        if((analyzer == null) || (range == null)) return;
        var Cancelled = false;
        if(Transfer.Timer != null) { clearTimeout(Transfer.Timer); Transfer.Timer = null; }
        if(Transfer.CancelFunc != null) { Transfer.CancelFunc(); Transfer.CancelFunc = null; }
        var ReqRange = analyzer.CalcRange(range);
        var Data = { };
        if((Transfer.Analyzer != null) && (Transfer.Analyzer.Analyzer == analyzer.Analyzer)) Data = Object.assign(Data, Transfer.Data);
        if((Transfer.Analyzer == null) || (Transfer.Analyzer.Analyzer != analyzer.Analyzer) || (Transfer.Range != range)) Transfer.Data = null;
        Transfer.Analyzer  = analyzer;
        Transfer.Range     = range;
        Data.From   = null;
        Data.To     = null;
        if(Data.StartTime != null) {
          var Pos = 0;
          for(var n = 0; n < Data.StartTime.length; n++) {
            if(Data.StartTime[n] < ReqRange[0])
              Pos++;
            else
              break;
          }
          if(Pos != 0) {
            for(var serie in Data) {
              if(Array.isArray(Data[serie]) == true) Data[serie].splice(0, Pos);
            }
          }
          var Pos = 0;
          for(var n = 0; n < Data.StartTime.length; n++) {
            if(Data.StartTime[n] <= ReqRange[1])
              Pos++;
            else
              break;
          }
          if(Pos != 0) {
            for(var serie in Data) {
              if(Array.isArray(Data[serie]) == true) Data[serie].splice(Pos);
            }
          }
          Data.From = Data.StartTime[0];
          Data.To = Data.StartTime.slice(-1)[0];
        }
        var Promises = [];
        if((Data.From == null) || (Data.To == null)) {
          Promises.push(Transfer.This.$http.post('getEvents', { Analyzer: analyzer.Analyzer, From: ReqRange[0], To: ReqRange[1], Update: Transfer.UpdateUnread }));
          Transfer.UpdateUnread = false;
        } else {
          if(ReqRange[0] < Data.From) Promises.push(Transfer.This.$http.post('getEvents', { Analyzer: analyzer.Analyzer, From: ReqRange[0], To: Data.From - 1, Series: series }));
          if(ReqRange[1] > Data.To) Promises.push(Transfer.This.$http.post('getEvents', { Analyzer: analyzer.Analyzer, From: Data.To + 1, To: ReqRange[1], Series: series }));
        }
        if(Promises.length == 0) {
          Data.From = ReqRange[0];
          Data.To = ReqRange[1];
          Transfer.Data = Object.seal(Data);
          if(Transfer.SingleShot == false) Transfer.Timer = setTimeout(() => { Transfer.UpdateEvents(Transfer.Analyzer, Transfer.Range); }, 3000);
          return;
        }
        Transfer.CancelFunc = () => { Cancelled = true; };
        Promise.all(Promises).then(
          (result) => {
            if(Cancelled == true) return;
            for(var n in result) {
              var After = ((result[n].body.StartTime.length > 0) && (Data.From != null) && (Data.From > result[n].body.StartTime[0]));
              for(var serie in result[n].body) {
                if(Array.isArray(result[n].body[serie]) == false) continue;
                if((Data.From == null) || (Data.To == null))
                  Data[serie] = result[n].body[serie];
                else if(After == true)
                  Data[serie] = result[n].body[serie].concat(Data[serie]);
                else
                  Data[serie] = Data[serie].concat(result[n].body[serie]);
              }
              if((Data.From == null) || (Data.To == null))
                Data.Deleted = Array(result[n].body.StartTime.length).fill(false);
              else if(After == true)
                Data.Deleted = Array(result[n].body.StartTime.length).fill(false).concat(Data[serie]);
              else
                Data.Deleted = Data.Deleted.concat(Array(result[n].body.StartTime.length).fill(false));
            }
            Data.From = ReqRange[0];
            Data.To = ReqRange[1];
            Transfer.Data = Object.seal(Data);
            if(Transfer.SingleShot == false) Transfer.Timer = setTimeout(() => { Transfer.UpdateEvents(Transfer.Analyzer, Transfer.Range); }, 3000);
          },
          (fail) => {
            if(Cancelled == true) return;
            if(Transfer.SingleShot == false) Transfer.Timer = setTimeout(() => { Transfer.UpdateEvents(Transfer.Analyzer, Transfer.Range); }, 3000);
          }
        );
      };
      Transfer.Update = (analyzer, aggreg, range) => {
        if((analyzer == null) || (aggreg == null) || (range == null)) return;
        var Cancelled = false;
        var Period    = (aggreg == '200MS') ? 200 : (aggreg == '3S') ? 3000 : (aggreg == '1M') ? 60000 : (aggreg == '10M') ? 600000 : (aggreg == '15M') ? 900000 : 3600000;
        if(Transfer.Timer != null) { clearTimeout(Transfer.Timer); Transfer.Timer = null; }
        if(Transfer.CancelFunc != null) { Transfer.CancelFunc(); Transfer.CancelFunc = null; }
        var ReqRange = analyzer.CalcRange(range, aggreg);
        var Data = { };
        if((Transfer.Aggreg == aggreg) && (Transfer.Analyzer != null) && (Transfer.Analyzer.Analyzer == analyzer.Analyzer)) Data = Object.assign(Data, Transfer.Data);
        if((Transfer.Aggreg != aggreg) || (Transfer.Analyzer == null) || (Transfer.Analyzer.Analyzer != analyzer.Analyzer) || (Transfer.Range != range)) Transfer.SetData(null);
        Transfer.Analyzer  = analyzer;
        Transfer.Aggreg    = aggreg;
        Transfer.Range     = range;
        Data.Aggreg = aggreg;
        Data.From   = null;
        Data.To     = null;
        if(Data.Sampletime != null) {
          var Pos = 0;
          for(var n = 0; n < Data.Sampletime.length; n++) {
            if(Data.Sampletime[n] < ReqRange[0])
              Pos++;
            else
              break;
          }
          if(Pos != 0) {
            for(var serie in Data) {
              if(Array.isArray(Data[serie]) == true) Data[serie].splice(0, Pos);
            }
          }
          var Pos = 0;
          for(var n = 0; n < Data.Sampletime.length; n++) {
            if(Data.Sampletime[n] <= ReqRange[1])
              Pos++;
            else
              break;
          }
          if(Pos != 0) {
            for(var serie in Data) {
              if(Array.isArray(Data[serie]) == true) Data[serie].splice(Pos);
            }
          }
          Data.From = Data.Sampletime[0];
          Data.To = Data.Sampletime.slice(-1)[0];
        }
        var Promises = [];
        if((Data.From == null) || (Data.To == null)) {
          Promises.push(Transfer.This.$http.post('getSeries', { Analyzer: analyzer.Analyzer, Aggreg: aggreg, From: ReqRange[0], To: ReqRange[1], Series: series }));
        } else {
          if((ReqRange[0] < Data.From) && (((Data.From - 1) - ReqRange[0]) > Period)) Promises.push(Transfer.This.$http.post('getSeries', { Analyzer: analyzer.Analyzer, Aggreg: aggreg, From: ReqRange[0], To: Data.From - 1, Series: series }));
          if((ReqRange[1] > Data.To)  && ((ReqRange[1] - (Data.To + 1)) > Period)) Promises.push(Transfer.This.$http.post('getSeries', { Analyzer: analyzer.Analyzer, Aggreg: aggreg, From: Data.To + 1, To: ReqRange[1], Series: series }));
        }
        if(Promises.length == 0) {
          Data.From = ReqRange[0];
          Data.To = ReqRange[1];
          Transfer.SetData(Data);
          Transfer.Timer = setTimeout(() => { Transfer.Update(Transfer.Analyzer, Transfer.Aggreg, Transfer.Range); }, (Period == 200) ? 100 : 3000);
          return;
        }
        Transfer.CancelFunc = () => { Cancelled = true; };
        Promise.all(Promises).then(
          (result) => {
            if(Cancelled == true) return;
            for(var n in result) {
              var After = ((Data.From != null) && (result[n].body.Sampletime.length > 0) && (Data.From > result[n].body.Sampletime[0]));
              for(var serie in result[n].body) {
                if(Array.isArray(result[n].body[serie]) == false) continue
                if((Data.From == null) || (Data.To == null))
                  Data[serie] = result[n].body[serie];
                else if(After == true)
                  Data[serie] = result[n].body[serie].concat(Data[serie]);
                else
                  Data[serie] = Data[serie].concat(result[n].body[serie]);
              }
            }
            Data.From = ReqRange[0];
            Data.To = ReqRange[1];
            Transfer.SetData(Data);
            Transfer.Timer = setTimeout(() => { Transfer.Update(Transfer.Analyzer, Transfer.Aggreg, Transfer.Range); }, (Period == 200) ? 0 : 3000);
          },
          (fail) => {
            if(Cancelled == true) return;
            if(fail.status == 400) {
              if(Transfer.AggregFail != null) Transfer.AggregFail();
              return;
            }
            Transfer.Timer = setTimeout(() => { Transfer.Update(Transfer.Analyzer, Transfer.Aggreg, Transfer.Range); }, (Period == 200) ? 0 : 3000);
          }
        );
      };
      return Transfer;
    },
    Vue.prototype.MatrixColsRange = (values, cols, options) => {
      var UpperRange = -Infinity;
      var LowerRange = +Infinity;
      for(var i in values) {
        for(var n in cols) {
          if((values[i][n] == null) || (cols[n] == false)) continue;
          if(values[i][n] > UpperRange) UpperRange = values[i][n];
          if(values[i][n] < LowerRange) LowerRange = values[i][n];
        }
      }
      if(options != null) {
        //This mode, enabled a minimum of 0 value, a min width of rmsRange and a margin of 10%
        if(options.rmsRange != null) {
          if((UpperRange == -Infinity) || (LowerRange == Infinity)) return [0, options.rmsRange];
          var Width = Math.max((UpperRange - LowerRange) * 0.1, options.rmsRange / 2.0);
          return [Math.max(0, LowerRange - Width), Math.max(UpperRange + Width)];
        }
        if(options.incZero != null) {
          if((UpperRange == -Infinity) || (LowerRange == Infinity)) return [0, options.incZero];
          if(LowerRange > 0) LowerRange = 0;
          if(UpperRange < 0) UpperRange = 0;
          var Width = Math.max((UpperRange - LowerRange) * 0.1, options.incZero / 2.0);
          return [(LowerRange < 0) ? LowerRange - Width : LowerRange, (UpperRange > 0) ? UpperRange + Width : UpperRange];
        }
        if(options.wave != null) {
          if((UpperRange == -Infinity) || (LowerRange == Infinity)) return [-options.wave, options.wave];
          var Max = Math.max(Math.abs(UpperRange), Math.abs(LowerRange));
          Max = (Max < (options.wave / 2.0)) ? options.wave / 2.0 : Max * 1.1;
          if((UpperRange > 0) && (LowerRange < 0)) {
            return [-Max, Max];
          } else if((UpperRange > 0) && (LowerRange >= 0)) {
            return [0, Max];
          } else {
            return [-Max, 0];
          }
        }



        if((UpperRange == -Infinity) && (options.defMax != null)) UpperRange = options.defMax;
        if((LowerRange == Infinity) && (options.defMin != null)) LowerRange = options.defMin;



        if((UpperRange > 0) && (options.upperPercent != null)) UpperRange += Math.abs(UpperRange * options.upperPercent);
        if((LowerRange < 0) && (options.lowerPercent != null)) LowerRange -= Math.abs(LowerRange * options.lowerPercent);
        if((UpperRange > 0) && (options.minUpperWidth != null)) UpperRange = Math.max(UpperRange, options.minUpperWidth);
        if((LowerRange < 0) && (options.minLowerWidth != null)) LowerRange = Math.min(LowerRange, options.minLowerWidth);
        if(options.percent != null) { UpperRange += Math.abs(UpperRange * options.percent); LowerRange -= Math.abs(LowerRange * options.percent); }
        if(options.minWidth != null) {
          UpperRange += Math.max((options.minWidth - (UpperRange - LowerRange)) / 2, 0);
          LowerRange -= Math.max((options.minWidth - (UpperRange - LowerRange)) / 2, 0);
        }
      }
      return [LowerRange, UpperRange];
    },
    Vue.prototype.BestUnits = (val) => {
      return (Math.abs(val) > 1000000) ? 'M' : (Math.abs(val) > 1000) ? 'k' : '';
    },
    Vue.prototype.ScaleUnits = (val, units) => {
      return (units == 'M') ? val / 1000000 : (units == 'k') ? val / 1000 : val;
    },
    Vue.prototype.Round = (val, decimals, html) => {
      if((val == null) || (isFinite(val) == false)) return '~';
      if(decimals == null) decimals = 2;
      if(html == null) html = false;
      var Exponent = Math.pow(10, decimals);
      val = Math.round(val * Exponent);
      var Integer = Math.trunc(val / Exponent);
      var Fraction = Math.abs(val % Exponent);
      Fraction = Fraction.toString();
      while(Fraction.length < decimals) Fraction = '0' + Fraction;
      return (((Integer == 0) && (val < 0.0)) ? '-' : '') + Integer.toString() + (html ? "<span style='font-size:80%'>." + Fraction + '</span>' : '.' + Fraction);
    },
    Vue.prototype.BestUnitsBin = (val, dec, html) => {
      if(val > 1024 * 1024 * 1024) {
        return Vue.prototype.Round(val / (1024 * 1024 * 1024), dec, html) + ' GB';
      } else if(val > 1024 * 1024) {
        return Vue.prototype.Round(val / (1024 * 1024), dec, html) + ' MB';
      } else if(val > 1024) {
        return Vue.prototype.Round(val / 1024) + ' kB';
      } else {
        return val + ' B';
      }
    },
    Vue.prototype.ArrayAbsMax = (array) => {
      var Max = 0;
      for(var i in array) Max = Math.max(Max, Array.isArray(array[i]) ? ArrayMax(array[i]) : Math.abs(array[i]));
      return Max;
    },
    Vue.prototype.BinPower = (complex) => {
      return Math.sqrt(Math.pow(complex[0], 2.0) + Math.pow(complex[1], 2.0));
    }
  }
};
export default openZmeter;

import chroma from 'chroma-js';
import Dygraph from 'JS/dygraph.js';
import moment from 'moment';
import Vue from 'vue';







export function RangeCalc(range) {
  return { Period: 86400000, To: 0 };
}
export function RoundHTML(val, decimals) {
  if((val == null) || (isFinite(val) == false)) {
    return '~';
  } else {
    if(decimals == null) decimals = 2;
    var Exponent = Math.pow(10, decimals);
    val = Math.round(val * Exponent);
    var Integer = Math.trunc(val / Exponent);
    var Fraction = Math.abs(val % Exponent);
    Fraction = Fraction.toString();
    while(Fraction.length < decimals) Fraction = '0' + Fraction;
    return (((Integer === 0) && (val < 0.0)) ? '-' : '') + Integer.toString() + "<span style='font-size:80%'>." + Fraction + '</span>';
  }
}
export function RoundPlain(val, decimals) {
  if(val === null) {
    return '-';
  } else {
    if(decimals === undefined) decimals = 2;
    var Exponent = Math.pow(10, decimals);
    val = Math.round(val * Exponent);
    var Integer = Math.trunc(val / Exponent);
    var Fraction = Math.abs(val % Exponent);
    Fraction = Fraction.toString();
    while(Fraction.length < decimals) Fraction = '0' + Fraction;
    return (((Integer === 0) && (val < 0.0)) ? '-' : '') + Integer.toString() + '.' + Fraction;
  }
}
export function SerieStyle(serie, analyzer, order) {
  var SerieLen = serie.Names(analyzer).length;
  if(SerieLen === 1) return { Colors: [serie.Color], Width: 0.5 };
  var Colors = ['#d9534f', '#5cb85c', '#318ce7', '#333333'];
  return {Colors: Colors.slice(0, SerieLen), Width: (order == 0) ? 0.5 : 1.5 }
}
export function MergeSeriesResult(result, previous) {
  var Out = Object.assign({}, previous);
  if(Out == null) Out = {};
  if(Out.LastSample == undefined) Out.LastSample = 0;
  for(var serie in result) {
    if(Array.isArray(result[serie]) == false) {
      Out[serie] = result[serie];
    } else {
      if(Out[serie] == undefined) Out[serie] = [];
      for(var pos in result.Sampletime) {
        if(result.Sampletime[pos] > Out.LastSample) Out[serie].push(result[serie][pos]);
        if(Out[serie].length > result.MaxSamples) Out[serie].shift();

      }
    }
  }
  if(Out.Sampletime.length > 0) Out.LastSample = Out.Sampletime.slice(-1)[0];
  return Object.freeze(Out);
}

export var Series = {
  'VOLTAGE': { CSS_Class: 'panel-voltage', Title: 'Voltage', Serie: 'Voltage', Units: 'V',  Link: '/Analysis/Voltage', Color: '#d9534f', Decimals: 2,
    Names: function(analyzer) { return (analyzer.Mode === '1PHASE') ? ['V'] : ['V(R)', 'V(S)', 'V(T)']; },
    Range: function(values, phases) {
      var UpperRange = -Infinity;
      var LowerRange = +Infinity;
      for(var i = 0; i < values.length; i++) {
        for(var n = 0; n < phases.length; n++) {
          if((values[i][n + 1] === null) || (phases[n] === false)) continue;
          if(values[i][n + 1] > UpperRange) UpperRange = values[i][n + 1];
          if(values[i][n + 1] < LowerRange) LowerRange = values[i][n + 1];
        }
      }
      UpperRange = (UpperRange === -Infinity) ? 1 : UpperRange;
      LowerRange = (LowerRange === Infinity) ? 0 : LowerRange;
      if((UpperRange - LowerRange) > 5) {
        var Diff = Math.max(1, UpperRange - LowerRange);
        UpperRange = UpperRange + Diff * 0.1;
        LowerRange = LowerRange - Diff * 0.1;
      } else {
        UpperRange = UpperRange + 0.25;
        LowerRange = LowerRange - 0.25;
      }
      if(LowerRange < 0) LowerRange = 0;
      return [LowerRange, UpperRange];
    }
  },
  'CURRENT': { CSS_Class: 'panel-current', Title: 'Current', Serie: 'Current', Units: 'A', Link: '/Analysis/Current', Color: '#428bca', Decimals: 2,
    FormatValue: function(val) {return {Units: '' + 'A', Value: Round2(val) }; },
    Names: function(analyzer) { return (analyzer.Mode === '1PHASE') ? ['I'] : (analyzer.Mode === '3PHASE') ? ['I(R)', 'I(S)', 'I(T)'] : ['I(R)', 'I(S)', 'I(T)', 'I(N)'];},
    Range: function(values, phases) {
      var UpperRange = -Infinity;
      var LowerRange = +Infinity;
      for(var i = 0; i < values.length; i++) {
        for(var n = 0; n < phases.length; n++) {
          if((values[i][n + 1] === null) || (phases[n] === false)) continue;
          if(values[i][n + 1] > UpperRange) UpperRange = values[i][n + 1];
          if(values[i][n + 1] < LowerRange) LowerRange = values[i][n + 1];
        }
      }
      UpperRange = (UpperRange === -Infinity) ? 1 : UpperRange;
      LowerRange = (LowerRange === Infinity) ? 0 : LowerRange;
      if((UpperRange - LowerRange) > 1) {
        var Diff = Math.max(1, UpperRange - LowerRange);
        UpperRange = UpperRange + Diff * 0.1;
        LowerRange = LowerRange - Diff * 0.1;
      } else {
        UpperRange = UpperRange + 0.05;
        LowerRange = LowerRange - 0.05;
      }
      if(LowerRange < 0) LowerRange = 0;
      return [LowerRange, UpperRange];
    }
  },
  'ACTIVE_POWER': { CSS_Class: 'panel-active-power', Title: 'Active Power', Serie: 'Active_Power', Units: 'W', Link: '/Analysis/Power', Color: '#f0ad4e', Decimals: 2,
    FormatValue: function(val) { var Units = GetBestUnits(val); return { Units: Units + 'W', Value: Round2(ScaleToUnits(val, Units)) }; },
    Names: function(analyzer) { return (analyzer.Mode === '1PHASE') ? ['P'] : ['P(R)', 'P(S)', 'P(T)']; },
    Range: function(values, phases) {
      var UpperRange = -Infinity;
      var LowerRange = +Infinity;
      for(var i = 0; i < values.length; i++) {
        for(var n = 0; n < phases.length; n++) {
          if((values[i][n + 1] === null) || (phases[n] === false)) continue;
          if(values[i][n + 1] > UpperRange) UpperRange = values[i][n + 1];
          if(values[i][n + 1] < LowerRange) LowerRange = values[i][n + 1];
        }
      }
      UpperRange = (UpperRange === -Infinity) ? 20 : UpperRange;
      LowerRange = (LowerRange === Infinity) ? 0 : LowerRange;
      if((UpperRange > 1) && (LowerRange > 0)) {
        var Diff = Math.max(10, UpperRange - LowerRange);
        UpperRange = UpperRange + Diff * 0.1;
        LowerRange = LowerRange - Diff * 0.1;
      }
      return [LowerRange, UpperRange];
    }
  },
  'ACTIVE_ENERGY': { CSS_Class: 'panel-active-energy', Title: 'Active Energy', Serie: 'Active_Energy', Units: 'Wh', Color: '#3d3d8b', Decimals: 2,
    FormatValue: function(val) { var Units = GetBestUnits(val); return { Units: Units + 'Wh', Value: Round2(ScaleToUnits(val, Units)) }; },
    Names: function(analyzer) { return (analyzer.Mode === '1PHASE') ? ['P'] : ['P(R)', 'P(S)', 'P(T)']; },
    Range: function(values, phases) {
      var UpperRange = -Infinity;
      var LowerRange = 0;
      for(var i = 0; i < values.length; i++) {
        for(var n = 0; n < phases.length; n++) {
          if((values[i][n + 1] === null) || (phases[n] === false)) continue;
          if(values[i][n + 1] > UpperRange) UpperRange = values[i][n + 1];
          if(values[i][n + 1] < LowerRange) LowerRange = values[i][n + 1];
        }
      }
      UpperRange = (UpperRange === -Infinity) ? 1 : UpperRange * 1.12;
      LowerRange = (LowerRange === 0) ? 0 : LowerRange - 1;
      return [LowerRange, UpperRange];
    }
  },
  'REACTIVE_ENERGY': { CSS_Class: 'panel-reactive-energy', Title: 'Reactive Energy', Serie: 'Reactive_Energy', Units: 'VArh', Color: '#643d8b', Decimals: 2,
    FormatValue: function(val) { var Units = GetBestUnits(val); return { Units: Units + 'VArh', Value: Round2(ScaleToUnits(val, Units)) }; },
    Names: function(analyzer) { return (analyzer.Mode === '1PHASE') ? ['Q'] : ['Q(R)', 'Q(S)', 'Q(T)']; },
    Range: function(values, phases) {
      var UpperRange = -Infinity;
      var LowerRange = 0;
      for(var i = 0; i < values.length; i++) {
        for(var n = 0; n < phases.length; n++) {
          if((values[i][n + 1] === null) || (phases[n] === false)) continue;
          if(values[i][n + 1] > UpperRange) UpperRange = values[i][n + 1];
          if(values[i][n + 1] < LowerRange) LowerRange = values[i][n + 1];
        }
      }
      UpperRange = (UpperRange === -Infinity) ? 1 : UpperRange * 1.12;
      LowerRange = (LowerRange === 0) ? 0 : LowerRange - 1;
      return [LowerRange, UpperRange];
    }
  },
  'FREQUENCY': { CSS_Class: 'panel-frequency', Title: 'Frequency', Serie: 'Frequency', Units: 'Hz', Link: '/Analysis/Frequency', Color: '#5cb85c', Decimals: 3,
    FormatValue: function(val) { return {Units: 'Hz', Value: Round2(val) }; },
    Names: function(analyzer) { return ['F']; },
    Range: function(values) {
      var UpperRange = -Infinity;
      var LowerRange = Infinity;
      for(var i = 0; i < values.length; i++) {
        if(values[i][1] === null) continue;
        if(values[i][1] > UpperRange) UpperRange = values[i][1];
        if(values[i][1] < LowerRange) LowerRange = values[i][1];
      }
      UpperRange = (UpperRange === -Infinity) ? 1 : UpperRange;
      LowerRange = (LowerRange === 0) ? 0 : LowerRange;
      if((UpperRange > 1) && (LowerRange > 0)) {
        var Diff = Math.max(1, UpperRange - LowerRange);
        UpperRange = UpperRange + Diff * 0.1;
        LowerRange = LowerRange - Diff * 0.1;
      }
      return [LowerRange, UpperRange];
    }
  },
  'REACTIVE_POWER': { CSS_Class: '', Title: 'Fund. Reactive Power', Serie: 'Reactive_Power', Units: 'VAr', Link: '/Analysis/Power', Color:  '#DF842A', Decimals: 2,
    FormatValue: function(val) { var Units = GetBestUnits(val); return { Units: Units + 'VAr', Value: Round2(ScaleToUnits(val, Units)) }; },
    Names: function(analyzer) { return (analyzer.Mode === '1PHASE') ? ['Q'] : ['Q(R)', 'Q(S)', 'Q(T)']; },
    Range: function(values, phases) {
      var UpperRange = -Infinity;
      var LowerRange = 0;
      for(var i = 0; i < values.length; i++) {
        for(var n = 0; n < phases.length; n++) {
          if((values[i][n + 1] === null) || (phases[n] === false)) continue;
          if(values[i][n + 1] > UpperRange) UpperRange = values[i][n + 1];
          if(values[i][n + 1] < LowerRange) LowerRange = values[i][n + 1];
        }
      }
      UpperRange = (UpperRange === -Infinity) ? 1 : UpperRange * 1.12;
      LowerRange = (LowerRange === 0) ? 0 : LowerRange - 1;
      return [LowerRange, UpperRange];
    }
  },
  'APPARENT_POWER': { CSS_Class: '', Title: 'Apparent Power', Serie: 'Apparent_Power', Units: 'va', Link: '/Analysis/Power', Color: '', Deciamls: 2,
    Names: function(analyzer) { return (analyzer.Mode === '1PHASE') ? ['S'] : ['S(R)', 'S(S)', 'S(T)']; },
    FormatValue: function(val) { var Units = GetBestUnits(val); return { Units: Units + 'va', Value: Round2(ScaleToUnits(val, Units)) }; },
    rangeFunc: function() {}
  },
  'POWER_FACTOR': { CSS_Class: '', Title: 'Power Factor', Serie: 'Power_Factor', Units: '', Link: '',
    FormatValue: function(val) { return { Units: '', Value: Round2(val) }; },
    Names: function(analyzer) { return (analyzer.Mode === '1PHASE') ? ['PF'] : ['PF(R)', 'PF(S)', 'PF(T)']; },
  },
  'PHI': { CSS_Class: '', Title: 'Phi', Serie: 'Phi', Units: '', Link: '',
    FormatValue: function(val) { return { Units: ((val < 0) ? '° IND' : (val > 0) ? '° CAP' : '°'), Value: Round2(Math.abs(val)) }; },
    Names: function(analyzer) { return (analyzer.Mode === '1PHASE') ? ['Phi'] : ['Phi(R)', 'Phi(S)', 'Phi(T)']; },
  },
  'VOLTAGE_THD': {
    CSS_Class: '',
    Title: 'Voltage THD',
    Serie: 'Voltage_THD',
    Units: '%',
    Link:'',
    FormatValue: function(val) { return { Units: '%', Value: Round2(val * 100) }; },
    Names: function(analyzer) { return (analyzer.Mode === '1PHASE') ? ['THDv'] : ['THDv(R)', 'THDv(S)', 'THDv(T)']; },
  },
  'CURRENT_THD': {
    CSS_Class: '',
    Title: 'Current THD',
    Serie: 'Current_THD',
    Units: '%',
    Link:'',
    FormatValue: function(val) { return { Units: '%', Value: Round2(val * 100) }; }
  },
  'HARMONICS': {
    FormatValue: function(val) { return { Units: 'dB', Value: Round2(val) }; },
    Units: 'dB',
    Names: ['1st', '2nd', '3rd', '4th', '5th', '6th', '7th', '8th', '9th', '10th', '11th', '12th', '13th', '14th', '15th', '16th', '17th', '18th', '19th', '20th', '21th', '22th', '23th', '24th', '25th', '26th', '27th', '28th', '29th', '30th', '31th', '32th', '33th', '34th', '35th', '36th', '37th', '38th', '39th', '40th', '41th', '42th', '43th', '44th', '45th', '46th', '47th', '48th', '49th', '50th']
  },
  'HARMONICS_POWER': {
    FormatValue: function(val) { var Units = GetBestUnits(val); return { Units: Units + 'W', Value: Round2(ScaleToUnits(val, Units)) }; },
    Units: 'W',
    Names: ['1st', '2nd', '3rd', '4th', '5th', '6th', '7th', '8th', '9th', '10th', '11th', '12th', '13th', '14th', '15th', '16th', '17th', '18th', '19th', '20th', '21th', '22th', '23th', '24th', '25th', '26th', '27th', '28th', '29th', '30th', '31th', '32th', '33th', '34th', '35th', '36th', '37th', '38th', '39th', '40th', '41th', '42th', '43th', '44th', '45th', '46th', '47th', '48th', '49th', '50th']
  },
  'VOLTAGE_SAMPLES': {
    Range: function(values, phases) {
      var UpperRange = -Infinity;
      var LowerRange = +Infinity;
      for(var i = 0; i < values.length; i++) {
        for(var n = 0; n < phases.length; n++) {
          if((values[i][n + 1] === null) || (phases[n] === false)) continue;
          if(values[i][n + 1] > UpperRange) UpperRange = values[i][n + 1];
          if(values[i][n + 1] < LowerRange) LowerRange = values[i][n + 1];
        }
      }
      UpperRange = (UpperRange === -Infinity) ? 1 : UpperRange;
      LowerRange = (LowerRange === Infinity) ? -1 : LowerRange;
      var Diff = Math.max(10, (UpperRange - LowerRange) * 0.1);
      UpperRange = UpperRange + Diff;
      LowerRange = LowerRange - Diff;
      return [LowerRange, UpperRange];
    }
  },
  'CURRENT_SAMPLES': {
    Range: function(values, phases) {
      var UpperRange = -Infinity;
      var LowerRange = +Infinity;
      for(var i = 0; i < values.length; i++) {
        for(var n = 0; n < phases.length; n++) {
          if((values[i][n + 1] === null) || (phases[n] === false)) continue;
          if(values[i][n + 1] > UpperRange) UpperRange = values[i][n + 1];
          if(values[i][n + 1] < LowerRange) LowerRange = values[i][n + 1];
        }
      }
      UpperRange = (UpperRange === -Infinity) ? 1 : UpperRange;
      LowerRange = (LowerRange === Infinity) ? -1 : LowerRange;
      var Diff = Math.max(1, (UpperRange - LowerRange) * 0.1);
      UpperRange = UpperRange + Diff;
      LowerRange = LowerRange - Diff;
      return [LowerRange, UpperRange];
    }
  }

//    if(varName === 'Reactive_Energy') return (Self.GetSelected().Mode === '1PHASE') ? ['Q'] : ['Q(R)', 'Q(S)', 'Q(T)'];
//    if(varName === 'Reactive_Power') return (Self.GetSelected().Mode === '1PHASE') ? ['Q'] : ['Q(R)', 'Q(S)', 'Q(T)'];
//    if(varName === 'Power_Factor')  return (Self.GetSelected().Mode === '1PHASE') ? ['PF'] : ['PF(R)', 'PF(S)', 'PF(T)'];
//    if(varName === 'Voltage_THD')  return (Self.GetSelected().Mode === '1PHASE') ? ['THDv'] : ['THDv(R)', 'THDv(S)', 'THDv(T)'];
//    if(varName === 'Current_THD')  return (Self.GetSelected().Mode === '1PHASE') ? ['THDi'] : (Self.GetSelected().Mode) ? ['THDi(R)', 'THDi(S)', 'THDi(T)'] : ['THDi(R)', 'THDi(S)', 'THDi(T)', 'THDi(N)'];
//    if(varName === 'Phi')  return (Self.GetSelected().Mode === '1PHASE') ? ['PHI']  : ['PHI(R)', 'PHI(S)', 'PHI(T)'];
};


export function dblClick(event, g, context) {
  if (!(event.offsetX && event.offsetY)) {
    event.offsetX = event.layerX - event.target.offsetLeft;
  }
  var xOffset = g.toDomCoords(g.xAxisRange()[0], null)[0];
  var x =  event.offsetX - xOffset;
  var w = g.toDomCoords(g.xAxisRange()[1], null)[0] - xOffset;
  var xPct = (w === 0 ? 0 : (x / w)) || 0.5;
  var axis = g.xAxisRange();
  var zoomInPercentage = (event.ctrlKey) ? -0.25 : 0.2;
  var delta = axis[1] - axis[0];
  var increment = delta * zoomInPercentage;
  var foo = [increment * xPct, increment * (1 - xPct)];
  g.updateOptions({
    dateWindow: [axis[0] + foo[0], axis[1] - foo[1]]
  });
}

export function zoom(g, zoomInPercentage, xBias, yBias) {
  xBias = xBias || 0.5;
  //yBias = yBias || 0.5;
  function adjustAxis(axis, zoomInPercentage, bias) {
    var delta = axis[1] - axis[0];
    var increment = delta * zoomInPercentage;
    var foo = [increment * bias, increment * (1-bias)];
    return [ axis[0] + foo[0], axis[1] - foo[1] ];
  }
  /*var yAxes = g.yAxisRanges();
  var newYAxes = [];
  for (var i = 0; i < yAxes.length; i++) {
    newYAxes[i] = adjustAxis(yAxes[i], zoomInPercentage, yBias);
  }*/

  g.updateOptions({
    dateWindow: adjustAxis(g.xAxisRange(), zoomInPercentage, xBias)/*,
    valueRange: newYAxes[0]*/
    });
}

/**
 * Creates captions on stackable view (mobile devices) to create a card for each row.
 * @param {Table as vue object} tableVue
 */
export function createCaptionsForResponsiveTable (tableVue) {
  if(tableVue === undefined) return;
  var headertext = [],
  headers = tableVue.$el.querySelectorAll("th"),
  tablebody = tableVue.$el.querySelector("tbody");
  for(var i = 0; i < headers.length; i++) {
    var current = headers[i];
    headertext.push(current.textContent.replace(/\r?\n|\r/,""));
  }
  for (var i = 0, row; row = tablebody.rows[i]; i++) {
    for (var j = 0, col; col = row.cells[j]; j++) {
      col.setAttribute("data-th", headertext[j]);
    }
  }
}

///* find abs max value of array (maybe array of arrays) */
export function ArrayAbsMax(array) {
  var Max = 0;
  for(var i in array) Max = Math.max(Max, Array.isArray(array[i]) ? ArrayMax(array[i]) : Math.abs(array[i]));
  return Max;
}
///* Scale all values of an array to units ('', 'k', 'M') */
export function ScaleArrayToUnits(array, units) {
  if(units === '') return array;
  for(var i = 0; i < array.length; i++) array[i] = ScaleToUnits(array[i], units);
  return array;
}
//
///* Scale value to units ('', 'k', 'M') */
export function ScaleToUnits(val, units) {
  if(units === '') return val;
  if(units === 'k') {
    return val / 1000;
  } else if(units === 'M') {
    return val / 1000000;
  }
}
///* Generate an array of colors from a base */
export function ColorArray(base, items) {
  var Array = [];
  for(var i = 0; i < items; i++) {
   Array.push(base);
   base = chroma(base).darken();
  }
  return Array;
}


////1446