CREATE TABLE IF NOT EXISTS users(
  username      character varying  NOT NULL PRIMARY KEY,
  password      character varying  NOT NULL,
  lastlogin     bigint             NOT NULL,
  time_zone     character varying  NOT NULL,
  name          character varying  NOT NULL,
  address       character varying  NOT NULL,
  company       character varying  NOT NULL,
  photo         text               NULL,
  email         character varying  NOT NULL,
  telegram      bigint             NULL UNIQUE
) WITH (OIDS=FALSE);
INSERT INTO users(username, password, lastlogin, time_zone, name, address, company, photo, email, telegram) VALUES('admin', MD5('admin'), 0, 'UTC', 'System administrator', '', '', '', '', 0) ON CONFLICT (username) DO NOTHING;

CREATE TABLE IF NOT EXISTS sessions(
  id        serial NOT NULL PRIMARY KEY,
  username  character varying REFERENCES users ON UPDATE CASCADE ON DELETE CASCADE,
  timestamp bigint NOT NULL
) WITH (OIDS=FALSE);
DELETE FROM sessions WHERE username IS NULL;
ALTER TABLE sessions ALTER COLUMN username SET NOT NULL;
CREATE INDEX IF NOT EXISTS sessions_timestamp_idx ON sessions USING btree (timestamp);

CREATE TABLE IF NOT EXISTS tariffs(
  tariff       serial            NOT NULL PRIMARY KEY,
  name         character varying NOT NULL,
  username     varchar           NOT NULL REFERENCES users(username) ON UPDATE CASCADE ON DELETE CASCADE,
  time_zone    varchar           NOT NULL,
  template     jsonb             NOT NULL,
  startdate    bigint            NOT NULL,
  enddate      bigint            NOT NULL
) WITH (OIDS=FALSE);

CREATE TABLE IF NOT EXISTS permissions(
  username   character varying NOT NULL REFERENCES users(username) ON UPDATE CASCADE ON DELETE CASCADE,
  serial     character varying NOT NULL,
  read       boolean           NOT NULL,
  write      boolean           NOT NULL,
  config     boolean           NOT NULL,
  CONSTRAINT user_serial UNIQUE(username, serial)
) WITH (OIDS=FALSE);

CREATE TABLE IF NOT EXISTS esios_index(
  keyvalue    serial             NOT NULL PRIMARY KEY,
  keyname     character varying  NOT NULL UNIQUE,
  shortname   character varying  NOT NULL
) WITH (OIDS=FALSE);

CREATE TABLE IF NOT EXISTS esios(
  keyvalue    serial  NOT NULL REFERENCES esios_index(keyvalue),
  timestamp   bigint  NOT NULL,
  value       real    NOT NULL,
  CONSTRAINT keyvalue_timestamp UNIQUE(keyvalue, timestamp)
) WITH (OIDS=FALSE);

CREATE TABLE IF NOT EXISTS telegram(
  id       serial NOT NULL PRIMARY KEY,
  msg      jsonb  NOT NULL
) WITH (OIDS=FALSE);