-- Create ROLE
CREATE ROLE openzmeter;
ALTER ROLE openzmeter WITH LOGIN ENCRYPTED PASSWORD 'openzmeter';


CREATE DATABASE openzmeter;
ALTER DATABASE openzmeter OWNER TO openzmeter;
ALTER DATABASE openzmeter WITH CONNECTION LIMIT = -1;
--GRANT CONNECT, TEMPORARY ON DATABASE openzmeter TO public;
GRANT ALL ON DATABASE openzmeter TO openzmeter;

\connect openzmeter;

CREATE TABLE IF NOT EXISTS aggreg_3s (
  node smallint NOT NULL,
  sampletime bigint NOT NULL,
  rms_v real[],
  rms_i real[],
  active_power real[],
  active_energy real[],
  fundamental_reactive_power real[],
  freq real,
  flag boolean,
  fundamental_reactive_energy real[],
  CONSTRAINT aggreg_3s_pkey PRIMARY KEY (sampletime, node)
) WITH (OIDS=FALSE);
ALTER TABLE aggreg_3s OWNER TO openzmeter;

CREATE TABLE IF NOT EXISTS aggreg_1m (
  node smallint NOT NULL,
  sampletime bigint NOT NULL,
  rms_v real[],
  rms_i real[],
  active_power real[],
  active_energy real[],
  fundamental_reactive_power real[],
  freq real,
  flag boolean,
  fundamental_reactive_energy real[],
  CONSTRAINT aggreg_1m_pkey PRIMARY KEY (sampletime, node)
) WITH (OIDS=FALSE);
ALTER TABLE aggreg_1m OWNER TO openzmeter;

CREATE TABLE IF NOT EXISTS aggreg_ext_1m (
  node smallint NOT NULL,
  sampletime bigint NOT NULL,
  power_factor real[],
  thd_v real[],
  thd_i real[],
  phi real[],
  harmonics_v real[],
  harmonics_i real[],
  CONSTRAINT aggreg_ext_1m_pkey PRIMARY KEY (sampletime, node)
) WITH (OIDS=FALSE);
ALTER TABLE aggreg_ext_1m OWNER TO openzmeter;

CREATE TABLE IF NOT EXISTS aggreg_10m (
  node smallint NOT NULL,
  sampletime bigint NOT NULL,
  rms_v real[],
  rms_i real[],
  active_power real[],
  active_energy real[],
  fundamental_reactive_power real[],
  freq real,
  flag boolean,
  fundamental_reactive_energy real[],
  CONSTRAINT aggreg_10m_pkey PRIMARY KEY (sampletime, node)
) WITH (OIDS=FALSE);
ALTER TABLE aggreg_10m OWNER TO openzmeter;

CREATE TABLE IF NOT EXISTS aggreg_1h (
  node smallint NOT NULL,
  sampletime bigint NOT NULL,
  rms_v real[],
  rms_i real[],
  active_power real[],
  active_energy real[],
  fundamental_reactive_power real[],
  freq real,
  flag boolean,
  fundamental_reactive_energy real[],
  CONSTRAINT aggreg_1h_pkey PRIMARY KEY (sampletime, node)
) WITH (OIDS=FALSE);
ALTER TABLE aggreg_1h OWNER TO openzmeter;

CREATE TABLE IF NOT EXISTS events (
  starttime bigint NOT NULL CONSTRAINT events_pkey PRIMARY KEY,
  duration  real,
  eventtype character varying(16),
  rms_v     real,
  variation real,
  samples   real[]
) WITH (OIDS=FALSE);
ALTER TABLE events OWNER TO openzmeter;

CREATE TABLE IF NOT EXISTS prices (
  starttime bigint NOT NULL CONSTRAINT prices_pkey PRIMARY KEY,
  price     real
) WITH (OIDS=FALSE);
ALTER TABLE prices OWNER TO openzmeter;
