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
  thd_v          real[] NOT NULL,
  thd_i          real[] NOT NULL,
  active_power   real[] NOT NULL,
  reactive_power real[] NOT NULL,
  phi            real[] NOT NULL,
  harmonics_v    real[] NOT NULL,
  harmonics_i    real[] NOT NULL
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
  sampletime bigint NOT NULL PRIMARY KEY, 
  rms_v          real[]  NOT NULL,
  rms_i          real[]  NOT NULL,
  freq           real,
  flag           boolean NOT NULL,
  active_power   real[]  NOT NULL,
  reactive_power real[]  NOT NULL,
  stat_voltage   int4[]  NOT NULL,
  stat_frequency int4[]  NOT NULL,
  stat_thd       int4[]  NOT NULL,
  stat_harmonics int4[]  NOT NULL
) WITH (OIDS=FALSE);

CREATE TABLE IF NOT EXISTS events (
  starttime   bigint                NOT NULL,
  evttype     character varying(16) NOT NULL,
  duration    bigint                NOT NULL,
  samplerate  real                  NOT NULL,
  reference   real[]                NOT NULL,
  peak        real[]                NOT NULL,
  change      real[]                NOT NULL,
  finished    boolean               NOT NULL,
  notes       text                  NOT NULL,
  samples1    real[],
  rms1        real[],
  samples2    real[],
  rms2        real[],
  samples3    real[],
  rms3        real[],
  CONSTRAINT events_pkey PRIMARY KEY (starttime, evttype)
) WITH (OIDS=FALSE);

CREATE TABLE IF NOT EXISTS prices (
  sampletime bigint NOT NULL PRIMARY KEY, 
  price real NOT NULL
) WITH (OIDS=FALSE);

CREATE TABLE IF NOT EXISTS rates(
  rate      serial            NOT NULL PRIMARY KEY,  
  name      character varying NOT NULL,
  startdate bigint            NOT NULL,
  enddate   bigint            NOT NULL,
  config    jsonb             NOT NULL,
  template  jsonb             NOT NULL
) WITH (OIDS=FALSE);
