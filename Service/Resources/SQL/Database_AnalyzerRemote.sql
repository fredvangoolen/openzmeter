CREATE UNLOGGED TABLE IF NOT EXISTS aggreg_200ms (
  sampletime     bigint NOT NULL PRIMARY KEY,
  rms_v          real[] NOT NULL,
  rms_i          real[] NOT NULL,
  unbalance      real,
  freq           real,
  flag           boolean NOT NULL,
  active_power   real[] NOT NULL,
  reactive_power real[] NOT NULL,
  power_factor   real[] NOT NULL,
  thd_v          real[] NOT NULL,
  thd_i          real[] NOT NULL,
  phi            real[] NOT NULL,
  harmonics_v    real[] NOT NULL,
  harmonics_i    real[] NOT NULL,
  harmonics_p    real[] NOT NULL
) WITH (OIDS=FALSE);

CREATE TABLE IF NOT EXISTS aggreg_3s (
  sampletime     bigint NOT NULL PRIMARY KEY,
  rms_v          real[] NOT NULL,
  rms_i          real[] NOT NULL,
  freq           real,
  flag           boolean NOT NULL,
  active_power   real[] NOT NULL,
  reactive_power real[] NOT NULL
) WITH (OIDS=FALSE);

CREATE TABLE IF NOT EXISTS aggreg_1m (
  sampletime bigint NOT NULL PRIMARY KEY,
  rms_v      real[] NOT NULL,
  rms_i      real[] NOT NULL,
  freq       real,
  flag       boolean NOT NULL
) WITH (OIDS=FALSE);

CREATE TABLE IF NOT EXISTS aggreg_ext_1m (
  sampletime     bigint NOT NULL PRIMARY KEY REFERENCES aggreg_1m(sampletime) ON UPDATE CASCADE ON DELETE CASCADE,
  power_factor   real[] NOT NULL,
  unbalance      real,
  thd_v          real[] NOT NULL,
  thd_i          real[] NOT NULL,
  active_power   real[] NOT NULL,
  reactive_power real[] NOT NULL,
  phi            real[] NOT NULL,
  harmonics_v    real[] NOT NULL,
  harmonics_i    real[] NOT NULL,
  harmonics_p    real[] NOT NULL
) WITH (OIDS=FALSE);

CREATE TABLE IF NOT EXISTS aggreg_10m (
  sampletime bigint NOT NULL PRIMARY KEY,
  rms_v      real[] NOT NULL,
  rms_i      real[] NOT NULL,
  freq       real,
  flag       boolean NOT NULL
) WITH (OIDS=FALSE);

CREATE TABLE IF NOT EXISTS aggreg_15m (
  sampletime     bigint NOT NULL PRIMARY KEY,
  active_power   real[] NOT NULL,
  reactive_power real[] NOT NULL
) WITH (OIDS=FALSE);

CREATE TABLE IF NOT EXISTS aggreg_1h (
  sampletime     bigint      NOT NULL PRIMARY KEY,
  rms_v          real[]      NOT NULL,
  rms_i          real[]      NOT NULL,
  freq           real,
  flag           boolean     NOT NULL,
  active_power   real[]      NOT NULL,
  reactive_power real[]      NOT NULL,
  stat_voltage   smallint[]  NOT NULL,
  stat_frequency smallint[]  NOT NULL,
  stat_thd       smallint[]  NOT NULL,
  stat_harmonics smallint[]  NOT NULL,
  stat_unbalance smallint[]  NOT NULL
) WITH (OIDS=FALSE);

CREATE TABLE IF NOT EXISTS events (
  starttime   bigint            NOT NULL PRIMARY KEY,
  evttype     character varying NOT NULL,
  samplerate  real              NOT NULL,
  reference   real[]            NOT NULL,
  peak        real[]            NOT NULL,
  change      real[]            NOT NULL,
  notes       text              NOT NULL,
  duration    bigint            NOT NULL
) WITH (OIDS=FALSE);

CREATE TABLE IF NOT EXISTS events_ext (
  starttime   bigint            NOT NULL REFERENCES events(starttime) ON UPDATE CASCADE ON DELETE CASCADE,
  sampletype  smallint          NOT NULL,
  samples     real[],
  PRIMARY KEY (starttime, sampletype)
) WITH (OIDS=FALSE);

CREATE TABLE IF NOT EXISTS alarms (
  id            character varying NOT NULL PRIMARY KEY,
  name          varchar           NOT NULL,
  priority      smallint          NOT NULL,
  expression    character varying NOT NULL,
  aggreg        character varying NOT NULL,
  samples       smallint          NOT NULL,
  windowlen     smallint          NOT NULL
) WITH (OIDS=FALSE);

CREATE TABLE IF NOT EXISTS alarms_ext (
  id            character varying NOT NULL REFERENCES alarms(id) ON UPDATE CASCADE ON DELETE CASCADE,
  trigger       bigint            NOT NULL,
  release       bigint            NULL,
  PRIMARY KEY(trigger, id)
) WITH (OIDS=FALSE);

CREATE TABLE IF NOT EXISTS rates (
  id            character varying NOT NULL PRIMARY KEY,
  name          character varying NOT NULL,
  ordering      smallint          NOT NULL,
  startdate     date              NOT NULL,
  enddate       date              NOT NULL,
  template      jsonb             NOT NULL
) WITH (OIDS=FALSE);