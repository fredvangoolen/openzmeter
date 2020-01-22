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
  telegram      character varying  NOT NULL
) WITH (OIDS=FALSE);
INSERT INTO users(username, password, lastlogin, time_zone, name, address, company, photo, email, telegram) VALUES('admin', MD5('admin'), 0, 'UTC', 'System administrator', '', '', '', '', '') ON CONFLICT (username) DO NOTHING;

CREATE TABLE IF NOT EXISTS sessions(
  id        serial NOT NULL PRIMARY KEY,
  username  character varying REFERENCES users ON UPDATE CASCADE ON DELETE CASCADE,
  timestamp bigint NOT NULL
) WITH (OIDS=FALSE);
DELETE FROM sessions WHERE username = NULL;
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
  username   character varying NOT NULL REFERENCES users(username) MATCH SIMPLE ON UPDATE CASCADE ON DELETE CASCADE,
  serial     character varying NOT NULL,
  read       boolean           NOT NULL,
  write      boolean           NOT NULL,
  config     boolean           NOT NULL,
  CONSTRAINT user_serial UNIQUE(username, serial)
) WITH (OIDS=FALSE);

CREATE TABLE IF NOT EXISTS news(
  newstime  bigint            NOT NULL PRIMARY KEY,
  header    character varying NOT NULL,
  contents  text              NOT NULL
) WITH (OIDS=FALSE);
