import CBOR from 'cbor-js';
import { Notify } from 'quasar';
import { computed, reactive, ref } from 'vue';
import Axios from 'axios';
import dayjs from 'dayjs';
import dayjs_utc from 'dayjs/plugin/utc';
import dayjs_timezone from 'dayjs/plugin/timezone';
import dayjs_dayOfYear from 'dayjs/plugin/dayOfYear';
import dayjs_customParseFormat from 'dayjs/plugin/customParseFormat';

const openZmeter = {
  install: function(App) {
    let This = App.config.globalProperties;
    dayjs.extend(dayjs_utc);
    dayjs.extend(dayjs_timezone);
    dayjs.extend(dayjs_dayOfYear);
    dayjs.extend(dayjs_customParseFormat);
    This.$dayjs = dayjs;
    This.API = reactive({
      Mode: 'application/json',
      Online: true,
      Success: 0,
      Fail: 0,
      Post: (url, params) => {
        return new Promise((resolve, reject) => {
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
        return new Promise((resolve, reject) => {
          This.API.Post('getDevice', { Serial: serial }).then(
            (result) => {
              result.Device = serial;
              resolve(result);
            },
            (result) => reject(result)
          );
        });
      },
      SetDevice: (settings) => {
        return This.API.Post('setDevice', settings);
      },
      GetUsers: () => {
        return This.API.Post('getUsers', {});
      },
      GetUser: (userName) => {
        return new Promise((resolve, reject) => {
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
        return new Promise((resolve, reject) => {
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
        return new Promise((resolve, reject) => {
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
        return This.API.Post('getSeriesNow', { Analyzer: analyzer, Series: series });
      },
      GetSeriesRange: (analyzer, aggreg) => {
        return This.API.Post("getSeriesRange", { Aggreg: aggreg, Analyzer: analyzer });
      },
      GetAlarmsRange: (analyzer) => {
        return This.API.Post("getAlarmsRange", { Analyzer: analyzer });
      },
      GetEventsRange: (analyzer) => {
        return This.API.Post("getEventsRange", { Analyzer: analyzer });
      },
      GetSeries: (analyzer, aggreg, series, from, to) => {
        return This.API.Post("getSeries", { Aggreg: aggreg, Analyzer: analyzer, Series: series, From: from, To: to });
      }
    });
    This.User = reactive({
      Clear: () => {
        for(let param in This.User) {
          if(This.User[param] instanceof Function) continue;
          delete This.User[param];
        }
      },
      GetCurrent: () => {
        return new Promise((resolve, reject) => {
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
    This.APIList = (method, interval) => {
      let APIList = reactive({ Timer: null, Interval: (interval == null) ? 5000 : interval, Method: method, Loading: true, Rows: [], Aborts: [],
        CancelPrev: () => {
          if(APIList.Timer != null) {
            clearTimeout(APIList.Timer);
            APIList.Timer = null;
          }
          while(APIList.Aborts.length > 0) APIList.Aborts.pop()();
        },
        Stop: () => {
          APIList.Rows.splice(0);
          APIList.CancelPrev();
        },
        Refresh: () => {
          let Cancelled = false;
          APIList.CancelPrev();
          APIList.Aborts.push(() => Cancelled = true );
          APIList.Method().then(
            (result) => {
              if(Cancelled == true) return;
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
              if(Cancelled == true) return;
              APIList.Loading = false;
              APIList.Timer = setTimeout(APIList.Refresh, APIList.Interval);
            }
          );
        }
      });
      APIList.Refresh();
      return APIList;
    };
    This.Analyzer = reactive({
      Aborts: [], PhaseNames: null, NeutralName: null,
      Ranges: {
        Loading: true, Aggreg3S: null, Aggreg1M: null, Aggreg10M: null, Aggreg15M: null, Aggreg1H: null, Alarms: null, Events: null, Timer: null, Aborts: [],
        CancelPrev: () => {
          if(This.Analyzer.Ranges.Timer != null) {
            clearTimeout(This.Analyzer.Ranges.Timer);
            This.Analyzer.Ranges.Timer = null;
          }
          while(This.Analyzer.Ranges.Aborts.length > 0) This.Analyzer.Ranges.Aborts.pop()();
        },
        Clear: () => {
          This.Analyzer.Ranges.Aggreg3S = null;
          This.Analyzer.Ranges.Aggreg1M = null;
          This.Analyzer.Ranges.Aggreg10M = null;
          This.Analyzer.Ranges.Aggreg15M = null;
          This.Analyzer.Ranges.Aggreg1H = null;
          This.Analyzer.Ranges.Alarms = null;
          This.Analyzer.Ranges.Events = null;
          This.Analyzer.Ranges.Loading = true;
        },
        Refresh: () => {
          let Promises = [];
          let Cancelled = false;
          This.Analyzer.Ranges.CancelPrev();
          This.Analyzer.Ranges.Aborts.push(() => Cancelled = true );
          Promises.push(This.API.GetSeriesRange(This.Analyzer.Analyzer, '3S'));
          Promises.push(This.API.GetSeriesRange(This.Analyzer.Analyzer, '1M'));
          Promises.push(This.API.GetSeriesRange(This.Analyzer.Analyzer, '10M'));
          Promises.push(This.API.GetSeriesRange(This.Analyzer.Analyzer, '15M'));
          Promises.push(This.API.GetSeriesRange(This.Analyzer.Analyzer, '1H'));
          Promises.push(This.API.GetAlarmsRange(This.Analyzer.Analyzer));
          Promises.push(This.API.GetEventsRange(This.Analyzer.Analyzer));
          Promise.all(Promises).then(
            (result) => {
              if(Cancelled == true) return;
              This.Analyzer.Ranges.Aggreg3S = result[0];
              This.Analyzer.Ranges.Aggreg1M = result[1];
              This.Analyzer.Ranges.Aggreg10M = result[2];
              This.Analyzer.Ranges.Aggreg15M = result[3];
              This.Analyzer.Ranges.Aggreg1H = result[4];
              This.Analyzer.Ranges.Alarms = result[5];
              This.Analyzer.Ranges.Events = result[6];
              This.Analyzer.Ranges.Timer = setTimeout(This.Analyzer.Ranges.Refresh, 300000);
            },
            () => {
              if(Cancelled == true) return;
              Suscriber.Timer = setTimeout(Suscriber.Refresh, Suscriber.Interval);
              Suscriber.Progress = null;
            }
          );
        }
      },
      Queries: {
        Subscribers: [],
        Suscribe: (type, series) => {
          let Suscriber = reactive({
            Type: type, Interval: 5000, Timer: null, Series: series, LastData: null, Progress: null, Range: null, Aggreg: null, Rows: [], Headers: null, Aborts: [],
            SetAggreg: (aggreg) => {
              if(Suscriber.Aggreg == aggreg) return;
              Suscriber.Aggreg = aggreg;
              Suscriber.Rows = [];
              Suscriber.Refresh();
            },
            SetRange: (range) => {
              if(Suscriber.Range == range) return;
              Suscriber.Range = range;
              Suscriber.Refresh();
            },
            Refresh: () => {
              if(((Suscriber.Type == 'Series') && (Suscriber.Aggreg == null)) || (This.Analyzer.Analyzer == null) || (Suscriber.Range == null)) {
                Suscriber.Rows = [];
                return;
              }
              let Done = 0;
              let Cancelled = false;
              let Promises = [];
              let ReqRange = null;
              let Now = This.$dayjs().tz(This.Analyzer.TimeZone);
              let UpdateProgress = () => {
                if(Suscriber.Progress == null) return;
                Done++;
                Suscriber.Progress = (Done / Promises.length);
              };
              Suscriber.CancelPrev();
              if(Suscriber.Range == 'LastMinute') ReqRange = [Now.valueOf() - 60000, Now.valueOf()];
              else if(Suscriber.Range == 'Last5Minutes') ReqRange = [Now.valueOf() - 300000, Now.valueOf()];
              else if(Suscriber.Range == 'Last10Minutes') ReqRange = [Now.valueOf() - 600000, Now.valueOf()];
              else if(Suscriber.Range == 'Last15Minutes') ReqRange = [Now.valueOf() - 900000, Now.valueOf()];
              else if(Suscriber.Range == 'Last30Minutes') ReqRange = [Now.valueOf() - 1800000, Now.valueOf()];
              else if(Suscriber.Range == 'LastHour') ReqRange = [Now.valueOf() - 3600000, Now.valueOf()];
              else if(Suscriber.Range == 'LastHours') ReqRange = [Now.valueOf() - 7200000, Now.valueOf()];
              else if(Suscriber.Range == 'LastDay') ReqRange = [Now.valueOf() - 86400000, Now.valueOf()];
              else if(Suscriber.Range == 'LastWeek') ReqRange = [Now.valueOf() - 604800000, Now.valueOf()];
              else if(Suscriber.Range == 'LastMonth') ReqRange = [Now.valueOf() - 2592000000, Now.valueOf()];
              else if(Suscriber.Range == 'LastYear') ReqRange = [Now.valueOf() - 31536000000, Now.valueOf()];
              else if(Suscriber.Range == 'Day') ReqRange = [Now.startOf('day').valueOf(), Now.endOf('day').valueOf()];
              else if(Suscriber.Range == 'Week') ReqRange = [Now.startOf('week').valueOf(), Now.endOf('week').valueOf()];
              else if(Suscriber.Range == 'Month') ReqRange = [Now.startOf('month').valueOf(), Now.endOf('month').valueOf()];
              else if(Suscriber.Range == 'Year') ReqRange = [Now.startOf('year').valueOf(), Now.endOf('year').valueOf()];
              else if(Array.isArray(Suscriber.Range)) ReqRange = Array.from(Suscriber.Range);
              for(let n = Suscriber.Rows.length - 1; n >= 0; n--) {
                if((Suscriber.Rows[n][0] < ReqRange[0]) || (Suscriber.Rows[n][0] > ReqRange[1])) Suscriber.Rows.splice(n, 1);
              }


//              Suscriber.Rows = Suscriber.Rows.filter(val => (val[0] >= ReqRange[0]) && (val[0] <= ReqRange[1]));
              if(ReqRange == null) {
                throw new Exception('No valid range');
              } else if(Suscriber.Rows.length == 0) {
                ReqRange = [ReqRange];
              } else {
                ReqRange = [[ReqRange[0], Suscriber.Rows[0][0] - 1], [Suscriber.Rows.slice(-1)[0][0] + 1, ReqRange[1]]];
              }
              ReqRange = ReqRange.filter(val => val[0] < val[1]);
              ReqRange.forEach(val => {
                if(Suscriber.Type == 'Series') {
                  let MS = 200 * 1000;
                  let From = val[0];
                  if(Suscriber.Aggreg == '3S') MS = 3000  * 1000
                  else if(Suscriber.Aggreg == '1M') MS = 60000 * 1000
                  else if(Suscriber.Aggreg == '10M') MS = 600000 * 1000
                  else if(Suscriber.Aggreg == '15M') MS = 900000 * 1000
                  else if(Suscriber.Aggreg == '1H') MS = 3600000 * 1000;
                  while(true) {
                    let To = Math.min(From + MS, val[1]);
                    Promises.push(new Promise((resolve, reject) => { This.API.GetSeries(This.Analyzer.Analyzer, Suscriber.Aggreg, Suscriber.Series, From, To).then((result) => { UpdateProgress(); resolve(result); }, reject)}));
                    if(To == val[1]) break;
                    From = To + 1;
                  }
                } else {
                  //Preparar para getevents o getalarms
//                  Promises.push(This.API.GetSeries(This.Analyzer.Analyzer, aggreg, series, From, To));
                }
              });
              if((Suscriber.Headers == null) || (Suscriber.Rows.length == 0)) Suscriber.Progress = 0;
              Suscriber.Aborts.push(() => Cancelled = true );
              Promise.all(Promises).then(
                (result) => {
                  if(Cancelled == true) return;
                  let Before = [];
                  result.forEach((val) => {
                    if(Suscriber.Headers == null) Suscriber.Headers = val[0];
                    if(val.length < 2) return;
                    val.shift();
                    if((Suscriber.Rows.length == 0) || (val[0][0] > Suscriber.Rows[0][0])) {
                      Suscriber.Rows = Suscriber.Rows.concat(val);
                    } else if(val[0][0] < Suscriber.Rows[0][0]) {
                      Before = Before.concat(val);
                    }
                  });
                  if(Before.length > 0) Suscriber.Rows = Before.concat(Suscriber.Rows);
                  Suscriber.Timer = setTimeout(Suscriber.Refresh, Suscriber.Interval);
                  Suscriber.Progress = null;
                },
                () => {
                  if(Cancelled == true) return;
                  Suscriber.Timer = setTimeout(Suscriber.Refresh, Suscriber.Interval);
                  Suscriber.Progress = null;
                }
              );
            },
            Stop: () => {
              Suscriber.Clear();
              This.Analyzer.Queries.Subscribers = This.Analyzer.Queries.Subscribers.filter(val => val != Suscriber);
            },
            Clear: () => {
              Suscriber.CancelPrev();
              Suscriber.Headers = null;
              Suscriber.Rows = [];
              Suscriber.Progress = null;
            },
            CancelPrev: () => {
              if(Suscriber.Timer != null) {
                clearTimeout(Suscriber.Timer);
                Suscriber.Timer = null;
              }
              while(Suscriber.Aborts.length > 0) Suscriber.Aborts.pop()();
            }
          });
          This.Analyzer.Queries.Subscribers.push(Suscriber);
          return Suscriber;
        },
        Refresh: () => {
          This.Analyzer.Queries.Subscribers.forEach(val => val.Refresh());
        },
        Clear: () => {
          This.Analyzer.Queries.Subscribers.forEach(val => val.Clear());
        }
      },
      NowSeries: {
        Aborts: [], Loading: true, Timer: null, Interval: 3000,
        GetSusbribed: () => {
          let Series = [];
          for(let param in This.Analyzer.NowSeries) {
            if(['Timer', 'Interval', 'Loading', 'Aborts'].includes(param)) continue;
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
        CancelPrev: () => {
          if(This.Analyzer.NowSeries.Timer != null) {
            clearTimeout(This.Analyzer.NowSeries.Timer);
            This.Analyzer.NowSeries.Timer = null;
          }
          while(This.Analyzer.NowSeries.Aborts.length > 0) This.Analyzer.NowSeries.Aborts.pop()();
        },
        Refresh: () => {
          if(This.Analyzer.Analyzer == null) return;
          let Series = This.Analyzer.NowSeries.GetSusbribed();
          let Cancelled = false;
          This.Analyzer.NowSeries.CancelPrev();
          if(Series.length == 0) return;
          This.Analyzer.NowSeries.Aborts.push(() => Cancelled = true );
          This.API.GetSeriesNow(This.Analyzer.Analyzer, Series).then(
            (result) => {
              if(Cancelled == true) return;
              This.Analyzer.NowSeries.Loading = false;
              Series.forEach(serie => This.Analyzer.NowSeries[serie].Value = result[serie]);
              This.Analyzer.NowSeries.Timer = setTimeout(This.Analyzer.NowSeries.Refresh, This.Analyzer.NowSeries.Interval);
            },
            () => {
              if(Cancelled == true) return;
              This.Analyzer.NowSeries.Loading = false;
              This.Analyzer.NowSeries.Timer = setTimeout(This.Analyzer.NowSeries.Refresh, This.Analyzer.NowSeries.Interval);
            }
          );
        },
        Clear: () => {
          This.Analyzer.NowSeries.CancelPrev();
          This.Analyzer.NowSeries.Loading = true;
          let Series = This.Analyzer.NowSeries.GetSusbribed();
          Series.forEach(val => val = null);
        }
      },
      Clear: () => {
        while(This.Analyzer.Aborts.length > 0) This.Analyzer.Aborts.pop()();
        This.Analyzer.NowSeries.Clear();
        This.Analyzer.Ranges.Clear();
        This.Analyzer.Queries.Clear();
        for(let param in This.Analyzer) {
          if(['Aborts', 'NowSeries', 'Queries', 'Ranges'].includes(param)) continue;
          if(This.Analyzer[param] instanceof Function) continue;
          delete This.Analyzer[param];
        }
        This.AnalyzerPhaseNames = null;
        This.Analyzer.NeutralName = null;
      },
      GetCurrent: () => {
        return new Promise((resolve, reject) => {
          This.Analyzer.Clear();
          let Analyzer = localStorage.getItem('Analyzer');
          if(Analyzer == null) {
            reject();
          } else {
            let Cancelled = false;
            This.Analyzer.Aborts.push(() => Cancelled = true );
            This.API.GetAnalyzer(Analyzer).then(
              (result) => {
                if(Cancelled == true) return;
                Object.assign(This.Analyzer, result);
                This.Analyzer.NowSeries.Refresh();
                This.Analyzer.Ranges.Refresh();
                This.Analyzer.Queries.Refresh();
                resolve(result)
              },
              (result) => {
                if(Cancelled == true) return;
                if(result != null) localStorage.removeItem('Analyzer');
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
      VoltageChannels: () => {
        return (This.Analyzer.PhaseNames != null) ? This.Analyzer.PhaseNames.length : 0;
      },
      CurrentChannels: () => {
         return This.Analyzer.VoltageChannels() + ((This.Analyzer.NeutralName != null) ? 1 : 0);
      }
    });

//    This.Aggregations = (series) => {
//      let Options = [];
//      let All = [{value: '3S',  name: '3 seconds'}, {value: '1M',  name: '1 minute'}, {value: '10M', name: '10 minutes'}, {value: '15M', name: '15 minutes'}, {value: '1H',  name: '1 hour'}, {value: '1D',  name: '1 day'}];
//      All.forEach(val => { if(series.includes(val.value)) Options.push(val); } );
//      return Options;
//    };
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
    This.ScaleUnits = (val, units) => {
      return (units == 'M') ? val / 1000000 : (units == 'k') ? val / 1000 : val;
    };
    This.Round = (val, decimals, html) => {
      if((val == null) || (isFinite(val) == false)) return '~';
      if(decimals == null) decimals = 2;
      if(html == null) html = false;
      let Exponent = Math.pow(10, decimals);
      val = Math.round(val * Exponent);
      let Integer = Math.trunc(val / Exponent);
      let Fraction = Math.abs(val % Exponent);
      Fraction = Fraction.toString();
      while(Fraction.length < decimals) Fraction = '0' + Fraction;
      return (((Integer == 0) && (val < 0.0)) ? '-' : '') + Integer.toString() + (html ? "<span style='font-size:80%'>." + Fraction + '</span>' : '.' + Fraction);
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
      let Stored = localStorage.getItem(key);
      if(Stored == null) {
        localStorage.setItem(key, def);
        return def;
      } else {
        if(Number.isInteger(def)) Stored = Number.parseInt(Stored);
        if(options == null) return Stored;
        if(options.includes(Stored)) {
          return Stored;
        } else {
          localStorage.setItem(key, def);
          return def;
        }
      }
    };


    This.SaveSvg = (svgEl, name) => {
      svgEl.setAttribute("xmlns", "http://www.w3.org/2000/svg");
      var svgData = svgEl.outerHTML;
      var preface = '<?xml version="1.0" standalone="no"?>\r\n';
      var svgBlob = new Blob([preface, svgData], {type:"image/svg+xml;charset=utf-8"});
      var svgUrl = URL.createObjectURL(svgBlob);
      var downloadLink = document.createElement("a");
      downloadLink.href = svgUrl;
      downloadLink.download = name;
      document.body.appendChild(downloadLink);
      downloadLink.click();
      document.body.removeChild(downloadLink);
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







////1446