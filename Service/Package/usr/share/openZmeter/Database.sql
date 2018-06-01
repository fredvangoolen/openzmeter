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
  node_ext smallint NOT NULL,
  sampletime_ext bigint NOT NULL,
  power_factor real[],
  thd_v real[],
  thd_i real[],
  phi real[],
  harmonics_v real[],
  harmonics_i real[],
  CONSTRAINT aggreg_ext_1m_pkey PRIMARY KEY (sampletime_ext, node_ext)
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

CREATE TABLE IF NOT EXISTS aggreg_stat_1h (
  node_ext smallint NOT NULL,
  sampletime_ext bigint NOT NULL,
  stat_voltage smallint[],
  stat_frequency smallint[],
  stat_thd smallint[],
  stat_harmonics smallint[],
  CONSTRAINT aggreg_stat_1h_pkey PRIMARY KEY (sampletime_ext, node_ext)
) WITH (OIDS=FALSE);
ALTER TABLE aggreg_stat_1h OWNER TO openzmeter;

CREATE TABLE IF NOT EXISTS events (
  node smallint NOT NULL,  
  starttime bigint NOT NULL,
  duration  bigint,
  type character varying(16),
  voltage real,
  percent real,
  samples real[],
  CONSTRAINT events_pkey PRIMARY KEY (starttime, node)
) WITH (OIDS=FALSE);
ALTER TABLE events OWNER TO openzmeter;


