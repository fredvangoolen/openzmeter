import CBOR from 'cbor-js';
import { Notify } from 'quasar';
import { reactive, ref } from 'vue';
import Axios from 'axios';
const openZmeter = {
  install: function(App) {
    let This = App.config.globalProperties;

    This.APIMode = 'application/cbor';
    This.Post = (url, params) => {
      return new Promise(function(resolve, reject) {
        Axios.post(url, (This.APIMode == 'application/cbor') ? CBOR.encode(params) : params, { responseType: (This.APIMode == 'application/cbor') ? 'arraybuffer' : 'json', headers: { 'Accept': This.APIMode, 'Content-Type': This.APIMode } }).then(
          (result) => {
//            EventBus.$emit('NoNetwork', false);
            resolve((This.APIMode == 'application/cbor') ? CBOR.decode(result.data) : result.body);
          },
          (error) => {
            if(error.status == 0) {
//              EventBus.$emit('NoNetwork', true);
              reject(null);
            } else if(error.status == 403) {
//              EventBus.$emit('NoNetwork', false);
              if(Vue.PostToastError != null) Vue.PostToastError.dismiss();
              Vue.PostToastError = Vue.$toast.open({type:'error', message: 'User is not allowed to do requested action', duration: 3000});
              reject(null);
            } else if(error.status == 401) {
//              EventBus.$emit('NoNetwork', false);
//              EventBus.$emit('UserUnathorized');
              reject(null);
            } else if(error.status == 500) {
//              EventBus.$emit('NoNetwork', false);
              if(Vue.PostToastError != null) Vue.PostToastError.dismiss();
              Vue.PostToastError = Vue.$toast.open({type:'error', message: 'An internal error happends while procesing your request', duration: 3000});
              reject(null);
            }
            reject({status: error.status, message: String.fromCharCode.apply(null, new Uint8Array(error.data))});
          }
        );
      });
    };

    This.Users = reactive({
      User: null,
      Timer: null,
      Listeners: 0,
      Headers: [],
      Items: [],
      Objects: [],
      Loading: false,
      RegisterListener: () => {
        This.Users.Listeners++;
        if(This.Users.Listeners == 1) {
          This.Users.Loading = true;
          This.Users.Refresh();
        }
      },
      UnregisterListener: () => {
        This.Users.Listeners--;
        if(This.Users.Listeners == 0) {
          This.Users.Items.splice(0);
          This.Users.Objects.splice(0);
          if(This.Users.Timer != null) {
            clearTimeout(This.Users.Timer);
            This.Users.Timer = null;
          }
        }
      },
      Refresh: () => {
        if(This.Users.Timer != null) {
          clearTimeout(This.Users.Timer);
          This.Users.Timer = null;
        }
        This.Users.getUsers().then(
          (result) => {
//            This.Users.Headers = result[0];
//            This.Users.Items.splice(0);
            This.Users.Objects.splice(0);
            This.Users.Loading = false;
            for(var i = 0; i < result.length; i++) {
//              This.Users.Items.push(result[i]);
//              let Item = {};
//              for(var n = 0; n < result[0].length; n++) Item[result[0][n]] = result[i][n];
//              This.Users.Objects.push(Item);
This.Users.Objects.push(result[i]);
            }
            This.Users.Timer = setTimeout(This.Users.Refresh, 5000);
          },
          () => { This.Users.Timer = setTimeout(This.Users.Refresh, 5000); }
        );
      },
      getUsers: () => {
        return This.Post('getUsers', {});
      },
      logOut: () => {
        This.Post('logOut', {}).then(
          () => {
            localStorage.removeItem("UserName");
            This.Users.User = null;
            resolve();
          },
          () => reject()
        );
      },
      logIn: (user, password) => {
        return new Promise(function(resolve, reject) {
          This.Post('logIn', { User: user, Password: password, UpdateLastLogin: true }).then(
            (result) => {
              localStorage.setItem('LastLogin', result.LastLogin);
              localStorage.setItem('UserName', user);
              resolve();
            },
            () => reject()
          );
        });
      },
      getCurrentUser: () => {
        return new Promise(function(resolve, reject) {
          let UserName = localStorage.getItem("UserName");
          if(UserName == null) {
            reject();
          } else if(This.Users.User != null) {
            resolve();
          } else {
            This.Users.getUser(UserName).then(
              (result) => {
                This.Users.User = result;
                Notify.create({ message: 'Logged in', timeout: 3000, color: 'green', position: 'bottom-right' });
                resolve(result)
              },
              (result) => {
                if(result != null) Notify.create({ message: 'Failed to login', caption: result.message, timeout: 3000, color: 'red', position: 'bottom-right' });
                reject(result)
              }
            );
          }
        });
      },
      getUser: (userName) => {
        return new Promise(function(resolve, reject) {
          This.Post('getUser', { User: userName }).then(
            (result) => resolve(result),
            (result) => reject(result)
          );
        });
      }
    });

    This.Analyzers = reactive({
      Timer: null,
      Listeners: 0,
      Headers: [],
      Items: [],
      Objects: [],
      Loading: false,
      RegisterListener: () => {
        This.Analyzers.Listeners++;
        if(This.Analyzers.Listeners == 1) {
          This.Analyzers.Loading = true;
          This.Analyzers.Refresh();
        }
      },
      UnregisterListener: () => {
        This.Analyzers.Listeners--;
        if(This.Analyzers.Listeners == 0) {
          This.Analyzers.Items.splice(0);
          This.Analyzers.Objects.splice(0);
          if(This.Analyzers.Timer != null) {
            clearTimeout(This.Analyzers.Timer);
            This.Analyzers.Timer = null;
          }
        }
      },
      Refresh: () => {
        if(This.Analyzers.Timer != null) {
          clearTimeout(This.Analyzers.Timer);
          This.Analyzers.Timer = null;
        }
        This.Analyzers.getAnalyzers().then(
          (result) => {
            console.log(result);
            This.Analyzers.Headers = result[0];
            This.Analyzers.Items.splice(0);
            This.Analyzers.Objects.splice(0);
            This.Analyzers.Loading = false;
            for(var i = 1; i < result.length; i++) {
              This.Analyzers.Items.push(result[i]);
              let Item = {};
              for(var n = 0; n < result[0].length; n++) Item[result[0][n]] = result[i][n];
              This.Analyzers.Objects.push(Item);
            }
            This.Analyzers.Timer = setTimeout(This.Analyzers.Refresh, 5000);
          },
          () => { This.Analyzers.Timer = setTimeout(This.Analyzers.Refresh, 5000); }
        );
      },
      getAnalyzers: () => {
        return This.Post('getAnalyzers', {});
      }
    });




    This.Analyzer = reactive({ Loaded: false });


    This.CBOR = CBOR;
    This.PostToastError = null;






    This.GetConfig = (key, def, options) => {
      //Leo key, si no existe guardo def y devuelvo dev
      //Si existe y options == undefined devuelvo value
      //Si options == [] busco que la opcion esta en el array
    };
    This.SetConfig = (key, value) => {

    };
    This.PriceTransfer = (series) => {
      var Transfer = { Timer: null, CancelFunc: null, Data: null, Series: series, Analyzer: null, Aggreg: null, Range: null };
      Transfer.Destroy = () => {
        if(Transfer.Timer != null) { clearTimeout(Transfer.Timer); Transfer.Timer = null; }
        if(Transfer.CancelFunc != null) { Transfer.CancelFunc(); Transfer.CancelFunc = null; }
      };
      Transfer.SetData = (range, aggreg, data) => {
        if(data == null) {
          Transfer.Data = null;
          return;
        }
        var Data = { Headers: data[0], Rows: data.slice(1), Aggreg: aggreg, From: range[0], To: range[1] };
        if(Transfer.Aggreg != '15M') {
          var ActivePowerCol = data[0].indexOf('Active_Power');
          var ReactivePowerCol = data[0].indexOf('Reactive_Power');
          var ActiveEnergyCol = data[0].indexOf('Active_Energy');
          var ReactiveEnergyCol = data[0].indexOf('Reactive_Energy');
          var ActivePower    = 0;
          var ReactivePower  = 0;
          var ActiveEnergy   = 0;
          var ReactiveEnergy = 0;
          var Count          = 0;
          var MaxCount       = (Transfer.Aggreg == '1H') ? 4.0 : 96.0;
          for(var i = 1; i < data.length; i++) {
            if(ActivePowerCol != -1) {
              ActivePower += data[i][ActivePowerCol] / MaxCount;
              data[i][ActivePowerCol] = null;
            }
            if(ReactivePowerCol != -1) {
              ReactivePower += data[i][ReactivePowerCol] / MaxCount;
              data[i][ReactivePowerCol] = null;
            }
            if(ActiveEnergyCol != -1) {
              ActiveEnergy += data[i][ActiveEnergyCol];
              data[i][ActiveEnergyCol] = null;
            }
            if(ReactiveEnergyCol != -1) {
              ReactiveEnergy += data[i][ReactiveEnergyCol];
              data[i][ReactiveEnergyCol] = null;
            }
            Count++;
            if(Count == MaxCount) {
              if(ActivePowerCol != -1) data[i][ActivePowerCol] = ActivePower;
              if(ReactivePowerCol != -1) data[i][ReactivePowerCol] = ReactivePower;
              if(ActiveEnergyCol != -1) data[i][ActiveEnergyCol] = ActiveEnergy;
              if(ReactiveEnergyCol != -1) data[i][ReactiveEnergyCol] = ReactiveEnergy;
              ActivePower = 0;
              ReactivePower = 0;
              ActiveEnergy = 0;
              ReactiveEnergy = 0;
              Count = 0;
            }
          }
        }
        Transfer.Data = Object.seal(Data);
      };
      Transfer.Update = (analyzer, aggreg, range) => {
        if((analyzer == null) || (aggreg == null) || (range == null)) {
          this.Data = null;
          return;
        }
        var Cancelled = false;
        var ReqRange  = analyzer.CalcRange(range, aggreg);
        if(aggreg == '1H') ReqRange[0] = ReqRange[0] / 3600000 * 3600000;
        if(Transfer.Timer != null) { clearTimeout(Transfer.Timer); Transfer.Timer = null; }
        if(Transfer.CancelFunc != null) { Transfer.CancelFunc(); Transfer.CancelFunc = null; }
        if((Transfer.Aggreg != aggreg) || (Transfer.Analyzer == null) || (Transfer.Analyzer.Analyzer != analyzer.Analyzer) || (Transfer.Range != range)) Transfer.SetData(null, null, null);
        Transfer.Analyzer  = analyzer;
        Transfer.Aggreg    = aggreg;
        Transfer.Range     = range;
        Transfer.CancelFunc = () => { Cancelled = true; };
        Vue.Post('getCosts', { Analyzer: analyzer.Analyzer, Aggreg: aggreg, From: ReqRange[0], To: ReqRange[1], Series: series, Group: 0 }).then(
          (result) => {
            if(Cancelled == true) return;
            Transfer.SetData(ReqRange, aggreg, result);
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
    This.StatusTransfer = (series) => {
      var Transfer = { Timer: null, CancelFunc: null, Data: null, Series: series, Analyzer: null };
      Transfer.Destroy = () => {
        if(Transfer.Timer != null) { clearTimeout(Transfer.Timer); Transfer.Timer = null; }
        if(Transfer.CancelFunc != null) { Transfer.CancelFunc(); Transfer.CancelFunc = null; }
      };
      Transfer.Update = (analyzer) => {
        if(analyzer == null) {
          this.Data = null;
          return;
        }
        var Cancelled = false;
        if(Transfer.Timer != null) { clearTimeout(Transfer.Timer); Transfer.Timer = null; }
        if(Transfer.CancelFunc != null) { Transfer.CancelFunc(); Transfer.CancelFunc = null; }
        if((Transfer.Analyzer == null) || (Transfer.Analyzer.Analyzer != analyzer.Analyzer)) Transfer.Data = null;
        Transfer.Analyzer  = analyzer;
        Transfer.CancelFunc = () => { Cancelled = true; };
        Vue.Post('getStatus', { Analyzer: analyzer.Analyzer, Series: series }).then(
          (result) => {
            if(Cancelled == true) return;
            Transfer.Data = Object.seal(result);
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
    This.NowTransfer = (series) => {
      var Transfer = { Timer: null, CancelFunc: null, Data: null, Series: series, Analyzer: null };
      Transfer.Destroy = () => {
        if(Transfer.Timer != null) { clearTimeout(Transfer.Timer); Transfer.Timer = null; }
        if(Transfer.CancelFunc != null) { Transfer.CancelFunc(); Transfer.CancelFunc = null; }
      };
      Transfer.Update = (analyzer) => {
        if(analyzer == null) {
          this.Data = null;
          return;
        }
        var Cancelled = false;
        if(Transfer.Timer != null) { clearTimeout(Transfer.Timer); Transfer.Timer = null; }
        if(Transfer.CancelFunc != null) { Transfer.CancelFunc(); Transfer.CancelFunc = null; }
        if((Transfer.Analyzer != null) && (Transfer.Analyzer.Analyzer != analyzer.Analyzer)) Transfer.Data = null;
        Transfer.Analyzer  = analyzer;
        Transfer.CancelFunc = () => { Cancelled = true; };
        Vue.Post('getSeriesNow', { Analyzer: analyzer.Analyzer, Series: series }).then(
          (result) => {
            if(Cancelled == true) return;
            Transfer.Data = Object.seal(result);
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
    This.SeriesTransfer = (series) => {
      var Transfer = { Timer: null, CancelFunc: null, Data: null, LastData: null, Series: series, Analyzer: null, Aggreg: null, Range: null };
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
        if(Transfer.Data.Rows.length == 0) {
          Transfer.LastData = null;
         } else {
           var Row  = Transfer.Data.Rows.slice(-1)[0];
           var Last = {};
           for(var n = 0; n < Transfer.Data.Headers.length; n++) Last[Transfer.Data.Headers[n]] = Row[n];
           Transfer.LastData = Object.seal(Last);
         }
      };
      Transfer.Update = (analyzer, aggreg, range) => {
        if((analyzer == null) || (aggreg == null) || (range == null)) {
          this.Data = null;
          return;
        }
        var Promises  = [];
        var Cancelled = false;
        var ReqRange  = analyzer.CalcRange(range, aggreg);
        if(Transfer.Timer != null) { clearTimeout(Transfer.Timer); Transfer.Timer = null; }
        if(Transfer.CancelFunc != null) { Transfer.CancelFunc(); Transfer.CancelFunc = null; }
        if((Transfer.Aggreg != aggreg) || (Transfer.Analyzer == null) || (Transfer.Analyzer.Analyzer != analyzer.Analyzer) || (Transfer.Range != range)) Transfer.SetData(null, null, null);
        while((Transfer.Data != null) && (Transfer.Data.Rows.length > 0) && (Transfer.Data.Rows[0][0] < ReqRange[0])) Transfer.Data.Rows.shift();
        while((Transfer.Data != null) && (Transfer.Data.Rows.length > 0) && (Transfer.Data.Rows.slice(-1)[0] > ReqRange[1])) Transfer.Data.Rows.pop();
        Transfer.Analyzer = analyzer;
        Transfer.Aggreg   = aggreg;
        Transfer.Range    = range;
        if((Transfer.Data == null) || (Transfer.Data.Rows.length == 0)) {
          Promises.push(Vue.Post('getSeries', { Analyzer: analyzer.Analyzer, Aggreg: aggreg, From: ReqRange[0], To: ReqRange[1], Series: series }));
        } else {
          Promises.push(Vue.Post('getSeries', { Analyzer: analyzer.Analyzer, Aggreg: aggreg, From: ReqRange[0], To: Transfer.Data.Rows[0][0] - 1, Series: series }));
          Promises.push(Vue.Post('getSeries', { Analyzer: analyzer.Analyzer, Aggreg: aggreg, From: Transfer.Data.Rows.slice(-1)[0][0] + 1, To: ReqRange[1], Series: series }));
        }
        Transfer.CancelFunc = () => { Cancelled = true; };
        Promise.all(Promises).then(
          (result) => {
            if(Cancelled == true) return;
            var Data = result[0];
            if(Transfer.Data != null) Data = Data.concat(Transfer.Data.Rows);
            if((result.length == 2) && (result[1].length > 1)) Data = Data.concat(result[1].slice(1));
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
    This.AlarmsTransfer = () => {
      var Transfer = { Timer: null, CancelFunc: null, Data: null, UpdateUnread: false, Analyzer: null, Range: null, ChangeTime: 0 };
      Transfer.Destroy = () => {
        if(Transfer.Timer != null) { clearTimeout(Transfer.Timer); Transfer.Timer = null; }
        if(Transfer.CancelFunc != null) { Transfer.CancelFunc(); Transfer.CancelFunc = null; }
      };
      Transfer.SetData = (range, data) => {
        if(data == null) {
          Transfer.Data = null;
          this.ChangeTime = 0;
          return;
        }
        var Data = { Headers: data[0], Rows: data.slice(1), From: range[0], To: range[1]};
        for(var n = 0; n < Data.Rows.length; n++) this.ChangeTime = Math.max(this.ChangeTime, Data.Rows[n][0]);
        Transfer.Data = Object.seal(Data);
      };
      Transfer.Update = (analyzer, range) => {
        if((analyzer == null) || (range == null)) {
          this.Data = null;
          return;
        }
        var Promises  = [];
        var Cancelled = false;
        var ReqRange  = analyzer.CalcRange(range);
        if(Transfer.Timer != null) { clearTimeout(Transfer.Timer); Transfer.Timer = null; }
        if(Transfer.CancelFunc != null) { Transfer.CancelFunc(); Transfer.CancelFunc = null; }
        if((Transfer.Analyzer == null) || (Transfer.Analyzer.Analyzer != analyzer.Analyzer) || (Transfer.Range != range)) Transfer.SetData(null, null);
        while((Transfer.Data != null) && (Transfer.Data.Rows.length > 0) && (Transfer.Data.Rows[0][1] < ReqRange[0])) Transfer.Data.Rows.shift();
        while((Transfer.Data != null) && (Transfer.Data.Rows.length > 0) && (Transfer.Data.Rows.slice(-1)[1] > ReqRange[1])) Transfer.Data.Rows.pop();
        Transfer.Analyzer  = analyzer;
        Transfer.Range     = range;
        if((Transfer.Data == null) || (Transfer.Data.Rows.length == 0)) {
          Promises.push(Vue.prototype.Post('getAlarms', { Analyzer: analyzer.Analyzer, From: ReqRange[0], To: ReqRange[1], Update: Transfer.UpdateUnread, ChangeTime: 0 }));
        } else {
          Promises.push(Vue.prototype.Post('getAlarms', { Analyzer: analyzer.Analyzer, From: ReqRange[0], To: Transfer.Data.Rows[0][2] - 1, Update: false, ChangeTime: 0 }));
          Promises.push(Vue.prototype.Post('getAlarms', { Analyzer: analyzer.Analyzer, From: Transfer.Data.Rows.slice(-1)[0][2] + 1, To: ReqRange[1], Update: false, ChangeTime: 0 }));
          Promises.push(Vue.prototype.Post('getAlarms', { Analyzer: analyzer.Analyzer, From: ReqRange[0], To: ReqRange[1], Update: false, ChangeTime: this.ChangeTime }));
        }
        Transfer.CancelFunc = () => { Cancelled = true; };
        Promise.all(Promises).then(
          (result) => {
            if(Cancelled == true) return;
            if(Cancelled == true) return;
            var Data = result[0];
            if(Transfer.Data != null) Data = Data.concat(Transfer.Data.Rows);
            if(result.length == 3) {
              if(result[1].length > 1) Data = Data.concat(result[1].slice(1));
              if(result[2].length > 1) {
                for(var n = 1; n < result[2].length; n++) {
                  for(var i = 0; i < Data.length; i++) {
                    if(Data[i][1] == result[2][n][1]) Data[i] = result[2][n];
                  }
                }
              }
            }
            Transfer.SetData(ReqRange, Data);
            Transfer.Timer = setTimeout(() => { Transfer.Update(Transfer.Analyzer, Transfer.Range); }, 3000);
          },
          (fail) => {
            if(Cancelled == true) return;
            Transfer.Timer = setTimeout(() => { Transfer.Update(Transfer.Analyzer, Transfer.Range); }, 3000);
          }
        );
      };
      return Transfer;
    },
    This.EventsTransfer = () => {
      var Transfer = { Timer: null, CancelFunc: null, Data: null, UpdateUnread: false, Analyzer: null, Range: null, ChangeTime: 0 };
      Transfer.Destroy = () => {
        if(Transfer.Timer != null) { clearTimeout(Transfer.Timer); Transfer.Timer = null; }
        if(Transfer.CancelFunc != null) { Transfer.CancelFunc(); Transfer.CancelFunc = null; }
      };
      Transfer.SetData = (range, data) => {
        if(data == null) {
          Transfer.Data = null;
          this.ChangeTime = 0;
          return;
        }
        var Data = { Headers: data[0], Rows: data.slice(1), From: range[0], To: range[1]};
        for(var n = 0; n < Data.Rows.length; n++) this.ChangeTime = Math.max(this.ChangeTime, Data.Rows[n][0]);
        if((Transfer.Data == null) || (Transfer.Data.From != range[0]) || (Transfer.Data.To != range[1]) || (Transfer.Data.Rows.length != Data.Rows.length)) Transfer.Data = Object.seal(Data);
      };
      Transfer.Update = (analyzer, range) => {
        if((analyzer == null) || (range == null)) {
          this.Data = null;
          return;
        }
        var Promises  = [];
        var Cancelled = false;
        var ReqRange  = analyzer.CalcRange(range);
        if(Transfer.Timer != null) { clearTimeout(Transfer.Timer); Transfer.Timer = null; }
        if(Transfer.CancelFunc != null) { Transfer.CancelFunc(); Transfer.CancelFunc = null; }
        if((Transfer.Analyzer == null) || (Transfer.Analyzer.Analyzer != analyzer.Analyzer) || (Transfer.Range != range)) Transfer.SetData(null, null);
        while((Transfer.Data != null) && (Transfer.Data.Rows.length > 0) && (Transfer.Data.Rows[0][1] < ReqRange[0])) Transfer.Data.Rows.shift();
        while((Transfer.Data != null) && (Transfer.Data.Rows.length > 0) && (Transfer.Data.Rows.slice(-1)[1] > ReqRange[1])) Transfer.Data.Rows.pop();
        Transfer.Analyzer  = analyzer;
        Transfer.Range     = range;
        if((Transfer.Data == null) || (Transfer.Data.Rows.length == 0)) {
          Promises.push(Vue.prototype.Post('getEvents', { Analyzer: analyzer.Analyzer, From: ReqRange[0], To: ReqRange[1], Update: Transfer.UpdateUnread, Samples: 'None', ChangeTime: 0 }));
        } else {
          Promises.push(Vue.prototype.Post('getEvents', { Analyzer: analyzer.Analyzer, From: ReqRange[0], To: Transfer.Data.Rows[0][1] - 1, Update: false, Samples: 'None', ChangeTime: 0 }));
          Promises.push(Vue.prototype.Post('getEvents', { Analyzer: analyzer.Analyzer, From: Transfer.Data.Rows.slice(-1)[0][1] + 1, To: ReqRange[1], Update: false, Samples: 'None', ChangeTime: 0 }));
          Promises.push(Vue.prototype.Post('getEvents', { Analyzer: analyzer.Analyzer, From: ReqRange[0], To: ReqRange[1], Update: false, Samples: 'None', ChangeTime: this.ChangeTime }));
        }
        Transfer.CancelFunc = () => { Cancelled = true; };
        Promise.all(Promises).then(
          (result) => {
            if(Cancelled == true) return;
            var Data = result[0];
            if(Transfer.Data != null) Data = Data.concat(Transfer.Data.Rows);
            if(result.length == 3) {
              if(result[1].length > 1) Data = Data.concat(result[1].slice(1));
              if(result[2].length > 1) {
                for(var n = 1; n < result[2].length; n++) {
                  for(var i = 0; i < Data.length; i++) {
                    if(Data[i][1] == result[2][n][1]) Data[i] = result[2][n];
                  }
                }
              }
            }
            Transfer.SetData(ReqRange, Data);
            Transfer.Timer = setTimeout(() => { Transfer.Update(Transfer.Analyzer, Transfer.Range); }, 3000);
          },
          (fail) => {
            if(Cancelled == true) return;
            Transfer.Timer = setTimeout(() => { Transfer.Update(Transfer.Analyzer, Transfer.Range); }, 3000);
          }
        );
      };
      return Transfer;
    },
    This.MatrixColsRange = (values, cols, options) => {
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
    This.BestUnits = (val) => {
      return (Math.abs(val) > 1000000) ? 'M' : (Math.abs(val) > 1000) ? 'k' : '';
    },
    This.ScaleUnits = (val, units) => {
      return (units == 'M') ? val / 1000000 : (units == 'k') ? val / 1000 : val;
    },
    This.Round = (val, decimals, html) => {
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
    This.BestUnitsBin = (val, dec, html) => {
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
    This.ArrayAbsMax = (array) => {
      var Max = 0;
      for(var i in array) Max = Math.max(Max, Array.isArray(array[i]) ? ArrayMax(array[i]) : Math.abs(array[i]));
      return Max;
    },
    This.BinPower = (complex) => {
      return Math.sqrt(Math.pow(complex[0], 2.0) + Math.pow(complex[1], 2.0));
    }
  }
};
export default openZmeter;

//import chroma from 'chroma-js';
//import Dygraph from 'JS/dygraph.js';
//import moment from 'moment';
//import Vue from 'vue';
//import VueResource from 'vue-resource';







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
/*export function SerieStyle(serie, analyzer, order) {
  var SerieLen = serie.Names(analyzer).length;
  if(SerieLen === 1) return { Colors: [serie.Color], Width: 0.5 };
  var Colors = ['#d9534f', '#5cb85c', '#318ce7', '#333333'];
  return {Colors: Colors.slice(0, SerieLen), Width: (order == 0) ? 0.5 : 1.5 }
}*/

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



////1446