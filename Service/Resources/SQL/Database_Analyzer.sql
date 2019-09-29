CREATE TABLE IF NOT EXISTS alarms(
  alarm         serial            NOT NULL PRIMARY KEY,  
  username      character varying NOT NULL REFERENCES users(username) ON UPDATE CASCADE ON DELETE CASCADE,
  name          character varying NOT NULL,
  description   character varying NOT NULL,
  formula       character varying NOT NULL,
  aggregation   character varying NOT NULL,
  message       character varying NOT NULL,
  samples       int               NOT NULL,
  window_length int               NOT NULL,
  priority      smallint          NOT NULL
) WITH (OIDS=FALSE);
