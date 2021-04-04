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
  changetime  bigint            NOT NULL,
  starttime   bigint            NOT NULL PRIMARY KEY,
  evttype     character varying NOT NULL,
  samplerate  real              NOT NULL,
  reference   real[]            NOT NULL,
  peak        real[]            NOT NULL,
  change      real[]            NOT NULL,
  notes       text              NOT NULL,
  duration    int               NOT NULL,
  size_raw    smallint          NOT NULL,
  size_rms    smallint          NOT NULL,
  samples_raw real[]            NULL,
  samples_rms real[]            NULL
) WITH (OIDS=FALSE);
CREATE INDEX IF NOT EXISTS events_changetime_idx ON events USING btree (changetime);

CREATE TABLE IF NOT EXISTS alarms (
  changetime    serial            NOT NULL,
  id            character varying NOT NULL,
  trigger       bigint            NOT NULL,
  release       bigint            NULL,
  notes         text              NOT NULL,
  UNIQUE(trigger, id)
) WITH (OIDS=FALSE);
CREATE INDEX IF NOT EXISTS alarms_changetime_idx ON alarms USING btree (changetime);