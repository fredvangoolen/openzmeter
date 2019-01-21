-- Create database
CREATE TABLE IF NOT EXISTS country(
  country_code  character varying NOT NULL PRIMARY KEY,
  country       character varying NOT NULL,
  icon          character varying NOT NULL,
  currency      character varying
) WITH (OIDS=FALSE);

CREATE TABLE IF NOT EXISTS users(
  username      character varying  NOT NULL PRIMARY KEY,
  password      character varying  NOT NULL,
  lastlogin     bigint             NOT NULL,
  country_code  character varying  NOT NULL REFERENCES country(country_code) MATCH SIMPLE ON UPDATE CASCADE ON DELETE CASCADE,
  name          character varying  NOT NULL,
	surname       character varying  NOT NULL,
	id_card       character varying  NOT NULL,
	address       character varying  NOT NULL,
	company       character varying  NOT NULL,
	photo         character varying  NULL,
	email         character varying  NOT NULL,
	telegram      character varying  NOT NULL
) WITH (OIDS=FALSE);
INSERT INTO users(username, password, lastlogin, country_code, name, surname, id_card, address, company, photo, email, telegram) VALUES('admin', MD5('admin'), 0, 'ESP', 'System administrator', '', '', '', '', '', '', '') ON CONFLICT (username) DO NOTHING;

CREATE TABLE IF NOT EXISTS sessions(
  id        serial NOT NULL PRIMARY KEY,
  username  character varying REFERENCES users ON UPDATE CASCADE ON DELETE CASCADE,
  timestamp bigint NOT NULL
) WITH (OIDS=FALSE);
CREATE INDEX IF NOT EXISTS sessions_timestamp_idx ON sessions USING btree (timestamp);

CREATE TABLE IF NOT EXISTS analyzers(
  serial    character varying NOT NULL PRIMARY KEY,
  username  character varying NOT NULL REFERENCES users ON UPDATE CASCADE ON DELETE CASCADE,
  contract  character varying NOT NULL,
  zipcode   character varying NOT NULL,
  frequency real              NOT NULL,
  voltage   real              NOT NULL,
  name      character varying NOT NULL,
  geo       point
) WITH (OIDS=FALSE);

CREATE TABLE IF NOT EXISTS rates(
  rate int NOT           NULL PRIMARY KEY,
  name character varying NOT NULL
) WITH (OIDS=FALSE);

CREATE TABLE IF NOT EXISTS analyzers_rates(
  serial    character varying NOT NULL REFERENCES analyzers ON UPDATE CASCADE ON DELETE CASCADE,
  rate      int               NOT NULL REFERENCES rates ON UPDATE CASCADE ON DELETE CASCADE,
  startdate bigint            NOT NULL,
  config    jsonb             NOT NULL
) WITH (OIDS=FALSE);
CREATE UNIQUE INDEX IF NOT EXISTS analyzers_rates_serial_startdate_idx ON analyzers_rates USING btree (serial, startdate);

CREATE TABLE IF NOT EXISTS permisions(
  username   character varying NOT NULL REFERENCES users(username) MATCH SIMPLE ON UPDATE CASCADE ON DELETE CASCADE,
  serial     character varying NOT NULL REFERENCES analyzers(serial) MATCH SIMPLE ON UPDATE CASCADE ON DELETE CASCADE,
  read       boolean           NOT NULL,
  write      boolean           NOT NULL
) WITH (OIDS=FALSE);


CREATE TABLE IF NOT EXISTS news(
  newstime  bigint NOT NULL PRIMARY KEY,
  header    character varying NOT NULL,
  contents  text              NOT NULL
) WITH (OIDS=FALSE);
INSERT INTO news(newstime, header, contents) VALUES( 0, 'Welcome!', 'openZmeter...... bla bla bla') ON CONFLICT (newstime) DO NOTHING;
INSERT INTO news(newstime, header, contents) VALUES( 1546209132, 'New function!', 'openZmeter now supports NULL values. What is this?<br>When current is to low, or frecuency is out of range some parameters have no sense. Before this parameters is saved as 0 but this broke some statistics.<br>For example if openZmeter is pluged half hour and unpluged other half average frequency is 25Hz!. This is mathematicaly real but not really useful.<br>Now the average is 50Hz, event is generated and the period is signaled. This happends with most parameters.<br>Most affected parameters is all related with current (Phi, Power factor, Current...) this parameter is not stored as 0. Its now stored as "no care" and dont disturb graphs ranges.') ON CONFLICT (newstime) DO NOTHING;
INSERT INTO news(newstime, header, contents) VALUES( 1546209133, 'Frequency view', 'Frequency view is ready... its a bit simple but works.<br>If you have some suggest please tell us!') ON CONFLICT (newstime) DO NOTHING;
INSERT INTO news(newstime, header, contents) VALUES( 1546209133, 'Voltage view', 'Voltage view is ready... its the biggest and more complex part of the WEB.<br>Please take a look!') ON CONFLICT (newstime) DO NOTHING;