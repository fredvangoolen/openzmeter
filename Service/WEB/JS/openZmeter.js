import CBOR from 'cbor-js';
import { Notify } from 'quasar';
import { reactive, ref } from 'vue';
import Axios from 'axios';
const openZmeter = {
  install: function(App) {
    let This = App.config.globalProperties;
    This.API = reactive({
      Mode: 'application/json',
      Online: true,
      Success: 0,
      Fail: 0,
      Post: (url, params) => {
        return new Promise(function(resolve, reject) {
          Axios.post(url, (This.API.Mode == 'application/cbor') ? CBOR.encode(params) : params, { responseType: (This.API.Mode == 'application/cbor') ? 'arraybuffer' : 'json', headers: { 'Accept': This.API.Mode, 'Content-Type': This.API.Mode } }).then(
            (result) => {
              This.API.Success++;
              This.API.Online = true;
              resolve((This.API.Mode == 'application/cbor') ? CBOR.decode(result.data) : result.data);
            },
            (error) => {
              This.API.Fail++;
              if(error instanceof Error) {
                This.API.Online = false;
                reject(null);
              } else if(error.status == 403) {
                This.API.Online = true;
                Notify.create({ message: 'User is not allowed to do requested action', timeout: 3000, color: 'red', position: 'bottom-right', group: 'error403' });
                reject(null);
              } else if(error.status == 401) {
                This.API.Online = true;
                Notify.create({ message: 'Session is closed due to inactivity', timeout: 5000, color: 'red', position: 'bottom-right' });
                App.$router.push('/Login');
                reject(null);
              } else if(error.status == 500) {
                This.Comm.Online = true;
                Notify.create({ message: 'An internal error happends while procesing your request', timeout: 3000, color: 'red', position: 'bottom-right', group: 'error500' });
                reject(null);
              }
              reject({status: error.status, message: String.fromCharCode.apply(null, new Uint8Array(error.data))});
            }
          );
        });
      },
      GetDevices: () => {
        return This.API.Post('getDevices', {});
      },
      GetDevice: (serial) => {
        return new Promise(function(resolve, reject) {
          This.API.Post('getDevice', { Serial: serial }).then(
            (result) => {
              result.Device = serial;
              resolve(result);
            },
            (result) => reject(result)
          );
        });
      },
      GetDevice: (settings) => {
        return This.API.Post('setDevice', settings);
      },
      GetUsers: () => {
        return This.API.Post('getUsers', {});
      },
      GetUser: (userName) => {
        return new Promise(function(resolve, reject) {
          This.API.Post('getUser', { User: userName }).then(
            (result) => {
              result.User = userName;
              resolve(result);
            },
            (result) => reject(result)
          );
        });
      },
      LogOut: () => {
        This.API.Post('logOut', {}).then(
          () => {
            localStorage.removeItem("UserName");
            This.User.clear();
            resolve();
          },
          () => reject()
        );
      },
      LogIn: (user, password) => {
        return new Promise(function(resolve, reject) {
          This.API.Post('logIn', { User: user, Password: password, UpdateLastLogin: true }).then(
            (result) => {
              localStorage.setItem('LastLogin', result.LastLogin);
              localStorage.setItem('UserName', user);
              This.User.GetCurrent().then(resolve, reject);
            },
            () => reject()
          );
        });
      },
      GetTariffs: () => {
        return This.API.Post('getTariffs', {});
      },
      GetAnalyzers: () => {
        return This.API.Post('getAnalyzers', {});
      },
      GetAnalyzer: (serial) => {
        return new Promise(function(resolve, reject) {
          This.API.Post('getAnalyzer', { Analyzer: serial }).then(
            (result) => {
              result.Analyzer = serial;
              resolve(result);
            },
            (result) => reject(result)
          );
        });
      },
      GetSeriesNow: (analyzer, series) => {
        return new Promise(function(resolve, reject) {
          This.API.Post('getSeriesNow', { Analyzer: analyzer, Series: series }).then(resolve, reject);
        });
      }
    });

    This.APIList = (method, interval) => {
      let APIList = reactive({ Timer: null, Interval: (interval == null) ? 5000 : interval, Method: method, Loading: true, Rows: [],
        Stop: () => {
          APIList.Rows.splice(0);
          if(APIList.Timer != null) {
            clearTimeout(APIList.Timer);
            APIList.Timer = null;
          }
        },
        Refresh: () => {
          if(APIList.Timer != null) {
            clearTimeout(APIList.Timer);
            APIList.Timer = null;
          }
          APIList.Method().then(
            (result) => {
              APIList.Rows.splice(0);
              APIList.Loading = false;
              for(let i = 1; i < result.length; i++) {
                let Item = { };
                for(let n = 0; n < result[0].length; n++) Item[result[0][n]] = result[i][n];
                APIList.Rows.push(Item);
              }
              APIList.Timer = setTimeout(APIList.Refresh, APIList.Interval);
            },
            () => {
              APIList.Loading = false;
              APIList.Timer = setTimeout(APIList.Refresh, APIList.Interval);
            }
          );
        }
      });
      APIList.Refresh();
      return APIList;
    };

    This.User = reactive({
      Clear: () => {
        for(let param in This.User) {
          if(This.User[param] instanceof Function) continue;
          delete This.User[param];
        }
      },
      GetCurrent: () => {
        return new Promise(function(resolve, reject) {
          This.User.Clear();
          let UserName = localStorage.getItem("UserName");
          if(UserName == null) {
            reject();
          } else {
            This.API.GetUser(UserName).then(
              (result) => {
                Object.assign(This.User, result);
                resolve(result);
              },
              (result) => {
                if(result != null) localStorage.removeItem('UserName');
                reject(result);
              }
            );
          }
        });
      }
    });

    This.Analyzer = reactive({
      NowSeries: {
        Loading: true,
        Timer: null,
        Interval: 3000,
        GetSusbribed: () => {
          let Series = [];
          for(let param in This.Analyzer.NowSeries) {
            if((param == 'Timer') || (param == 'Interval') || (param == 'Loading')) continue;
            if(This.Analyzer.NowSeries[param] instanceof Function) continue;
            Series.push(param);
          }
          return Series;
        },
        Suscribe: (series) => {
          series.forEach(serie => {
            if(This.Analyzer.NowSeries[serie] == null) {
              This.Analyzer.NowSeries.Loading = false;
              This.Analyzer.NowSeries[serie] = { Suscribers: 0, Value: null };
            }
            This.Analyzer.NowSeries[serie].Suscribers++;
          });
          This.Analyzer.NowSeries.Refresh();
        },
        Unsuscribe: (series) => {
          series.forEach(serie => {
            This.Analyzer.NowSeries[serie].Subscribers--;
            if(This.Analyzer.NowSeries[serie].Suscribers == 0) delete This.Analyzer.NowSeries[serie];
          });
        },
        Refresh: () => {
          if(This.Analyzer.NowSeries.Timer != null) {
            clearTimeout(This.Analyzer.NowSeries.Timer);
            This.Analyzer.NowSeries.Timer = null;
          }
          let Series = This.Analyzer.NowSeries.GetSusbribed();
          if(Series.length == 0) return;
          This.API.GetSeriesNow(This.Analyzer.Analyzer, Series).then(
            (result) => {
              This.Analyzer.NowSeries.Loading = false;
              Series.forEach(serie => This.Analyzer.NowSeries[serie].Value = result[serie]);
              This.Analyzer.NowSeries.Timer = setTimeout(This.Analyzer.NowSeries.Refresh, This.Analyzer.NowSeries.Interval);
            },
            () => {
              This.Analyzer.NowSeries.Loading = false;
              This.Analyzer.NowSeries.Timer = setTimeout(This.Analyzer.NowSeries.Refresh, This.Analyzer.NowSeries.Interval);
            }
          );
        },
        Clear: () => {
          if(This.Analyzer.NowSeries.Timer != null) {
            clearTimeout(This.Analyzer.NowSeries.Timer);
            This.Analyzer.NowSeries.Timer = null;
          }
          This.Analyzer.NowSeries.Loading = true;
          for(let param in This.Analyzer.NowSeries) {
            if((param == 'Timer') || (param == 'Interval') || (param == 'Loading')) continue;
            if(This.Analyzer.NowSeries[param] instanceof Function) continue;
            This.Analyzer.NowSeries[param].Value = null;
          }
        }
      },
      Clear: () => {
        This.Analyzer.NowSeries.Clear();
        for(let param in This.Analyzer) {
          if(param == 'NowSeries') continue;
          if(This.Analyzer[param] instanceof Function) continue;
          delete This.Analyzer[param];
        }
      },
      GetCurrent: () => {
        return new Promise(function(resolve, reject) {
          This.Analyzer.Clear();
          let Analyzer = localStorage.getItem('Analyzer');
          if(Analyzer == null) {
            reject();
          } else {
            This.API.GetAnalyzer(Analyzer).then(
              (result) => {
                Object.assign(This.Analyzer, result);
                This.Analyzer.NowSeries.Refresh();
                resolve(result)
              },
              (result) => {
                if(result != null) localStorage.removeItem('Analyzer');
//                Notify.create({ message: 'Previous analyzer not found, choose new one', timeout: 3000, color: 'red', position: 'bottom-right' });
                reject(result)
              }
            );
          }
        });
      },
      SetCurrent: (serial) => {
        localStorage.setItem('Analyzer', serial);
        return This.Analyzer.GetCurrent();
      },
    });

    This.Aggregations = [{value: '3S',  name: '3 seconds'}, {value: '1M',  name: '1 minute'}, {value: '10M', name: '10 minutes'}, {value: '15M', name: '15 minutes'}, {value: '1H',  name: '1 hour'}, {value: '1D',  name: '1 day'}];

    This.SerieViewColor = (serie) => {
      if(serie == 'Frequency') return '#5CB85C';
      if(serie == 'Voltage') return '#D9534F';
      if(serie == 'Current') return '#428BCA';
      if(serie == 'Active_Power') return '#F0AD4E';
      if(serie == 'Reactive_Power') return '#DF842A';
      if(serie == 'Apparent_Power') return '#DF842A';
      if(serie == 'Active_Energy') return '#3D3D8B';
      if(serie == 'Reactive_Energy') return '#643D8B';
    };
    This.SerieUnits = (serie) => {
      if(serie == 'Voltage') return 'V';
      if(serie == 'Current') return 'A';
      if(serie == 'Active_Power') return 'W';
      if(serie == 'Active_Energy') return 'Wh';
      if(serie == 'Frequency') return 'Hz';
    };
    This.BestUnits = (val) => {
      return (Math.abs(val) > 1000000) ? 'M' : (Math.abs(val) > 1000) ? 'k' : '';
    };
    This.ScaleUnits = (val, units) => {
      return (units == 'M') ? val / 1000000 : (units == 'k') ? val / 1000 : val;
    };
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
    };
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
    };
    This.ArrayAbsMax = (array) => {
      let Max = 0;
      for(var i in array) Max = Math.max(Max, Array.isArray(array[i]) ? ArrayMax(array[i]) : Math.abs(array[i]));
      return Max;
    };
    This.ArrayRMS = (array) => {
      let Val = 0.0;
      array.forEach(val => Val += Math.pow(val, 2.0));
      return Math.sqrt(Val / array.length);
    };
    This.ArraySUM = (array) => {
      let Val = 0.0;
      array.forEach(val => Val += val);
      return Val;
    };
    This.BinPower = (complex) => {
      return Math.sqrt(Math.pow(complex[0], 2.0) + Math.pow(complex[1], 2.0));
    }

    This.GetConfig = (key, def, options) => {
      if(key == null) return def;
      var Stored = localStorage.getItem(key);
      if(Stored == null) {
        localStorage.setItem(key, def);
        return def;
      } else {
        if(options == null) return Stored;
        if(options.includes(Stored)) {
          return Stored;
        } else {
          localStorage.setItem(key, def);
          return def;
        }
      }
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
        This.API.Post('getSeriesNow', { Analyzer: analyzer.Analyzer, Series: series }).then(
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
          Promises.push(This.API.Post('getSeries', { Analyzer: analyzer.Analyzer, Aggreg: aggreg, From: ReqRange[0], To: ReqRange[1], Series: series }));
        } else {
          Promises.push(This.API.Post('getSeries', { Analyzer: analyzer.Analyzer, Aggreg: aggreg, From: ReqRange[0], To: Transfer.Data.Rows[0][0] - 1, Series: series }));
          Promises.push(This.API.Post('getSeries', { Analyzer: analyzer.Analyzer, Aggreg: aggreg, From: Transfer.Data.Rows.slice(-1)[0][0] + 1, To: ReqRange[1], Series: series }));
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