-- Create database
CREATE TABLE IF NOT EXISTS users(
  name      character varying NOT NULL PRIMARY KEY,
  password  character varying NOT NULL,
  lastlogin bigint            NOT NULL,
  admin     boolean           NOT NULL
) WITH (OIDS=FALSE);

CREATE TABLE IF NOT EXISTS sessions (
  id        serial NOT NULL PRIMARY KEY,
  name      character varying REFERENCES users ON UPDATE CASCADE ON DELETE CASCADE,
  timestamp bigint NOT NULL
) WITH (OIDS=FALSE);
CREATE INDEX IF NOT EXISTS sessions_timestamp_idx ON sessions USING btree (timestamp);

INSERT INTO users(name, password, lastlogin, admin) VALUES('admin', MD5('admin'), 0, true) ON CONFLICT (name) DO NOTHING;

CREATE TABLE IF NOT EXISTS permisions(
  name       character varying NOT NULL,
  serial     character varying NOT NULL,
  read       boolean           NOT NULL,
  write      boolean           NOT NULL,
  lastevents bigint            NOT NULL,
  CONSTRAINT permisions_name_pkey FOREIGN KEY (name) REFERENCES users (name) MATCH SIMPLE ON UPDATE CASCADE ON DELETE CASCADE
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