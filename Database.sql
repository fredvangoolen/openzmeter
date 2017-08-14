CREATE TABLE IF NOT EXISTS agreg_10c (
  node                       int NOT NULL,
  sampletime                 bigint NOT NULL,
  rms_v                      real,
  rms_i                      real,
  active_power               real,
  reactive_power             real,
  apparent_power             real,
  power_factor               real,
  active_energy              real,
  reactive_energy            real,
  fundamental_active_power   real,
  fundamental_reactive_power real,
  fundamental_apparent_power real,
  fundamental_power_factor   real,
  thd_v                      real,
  thd_i                      real,
  freq                       real,
  phi                        real,
  flaged                     boolean,
  harmonics_v                real[],
  harmonics_i                real[],
  PRIMARY KEY(node, sampletime)
) WITH (OIDS=FALSE);

CREATE TABLE IF NOT EXISTS agreg_3s (
  node                       int NOT NULL,
  sampletime                 bigint NOT NULL,
  rms_v                      real,
  rms_i                      real,
  active_power               real,
  reactive_power             real,
  apparent_power             real,
  power_factor               real,
  active_energy              real,
  reactive_energy            real,
  fundamental_active_power   real,
  fundamental_reactive_power real,
  fundamental_apparent_power real,
  fundamental_power_factor   real,
  thd_v                      real,
  thd_i                      real,
  freq                       real,
  phi                        real,
  flaged                     boolean,
  harmonics_v                real[],
  harmonics_i                real[],
  PRIMARY KEY(node, sampletime)
) WITH (OIDS=FALSE);

CREATE TABLE IF NOT EXISTS agreg_1m (
  node                       int NOT NULL,
  sampletime                 bigint NOT NULL,
  rms_v                      real,
  rms_i                      real,
  active_power               real,
  reactive_power             real,
  apparent_power             real,
  power_factor               real,
  active_energy              real,
  reactive_energy            real,
  fundamental_active_power   real,
  fundamental_reactive_power real,
  fundamental_apparent_power real,
  fundamental_power_factor   real,
  thd_v                      real,
  thd_i                      real,
  freq                       real,
  phi                        real,
  flaged                     boolean,
  harmonics_v                real[],
  harmonics_i                real[],
  PRIMARY KEY(node, sampletime)
) WITH (OIDS=FALSE);

CREATE TABLE IF NOT EXISTS agreg_10m (
  node                       int NOT NULL,
  sampletime                 bigint NOT NULL,
  rms_v                      real,
  rms_i                      real,
  active_power               real,
  reactive_power             real,
  apparent_power             real,
  power_factor               real,
  active_energy              real,
  reactive_energy            real,
  fundamental_active_power   real,
  fundamental_reactive_power real,
  fundamental_apparent_power real,
  fundamental_power_factor   real,
  thd_v                      real,
  thd_i                      real,
  freq                       real,
  phi                        real,
  flaged                     boolean,
  harmonics_v                real[],
  harmonics_i                real[],
  PRIMARY KEY(node, sampletime)
) WITH (OIDS=FALSE);

CREATE TABLE IF NOT EXISTS agreg_1h (
  node                       int NOT NULL,
  sampletime                 bigint NOT NULL,
  rms_v                      real,
  rms_i                      real,
  active_power               real,
  reactive_power             real,
  apparent_power             real,
  power_factor               real,
  active_energy              real,
  reactive_energy            real,
  fundamental_active_power   real,
  fundamental_reactive_power real,
  fundamental_apparent_power real,
  fundamental_power_factor   real,
  thd_v                      real,
  thd_i                      real,
  freq                       real,
  phi                        real,
  flaged                     boolean,
  harmonics_v                real[],
  harmonics_i                real[],
  PRIMARY KEY(node, sampletime)
) WITH (OIDS=FALSE);

CREATE TABLE IF NOT EXISTS events (
  starttime bigint NOT NULL CONSTRAINT events_pkey PRIMARY KEY,
  duration  real,
  eventtype character varying(16),
  rms_v     real,
  variation real,
  samples   real[]
) WITH (OIDS=FALSE);

CREATE TABLE IF NOT EXISTS prices (
  starttime bigint NOT NULL CONSTRAINT prices_pkey PRIMARY KEY,
  price     real
) WITH (OIDS=FALSE);

CREATE OR REPLACE FUNCTION VECTOR_POW2(arr1 real[])
RETURNS real[] AS $$
  SELECT array_agg(result) FROM (SELECT CAST(POW(tuple.val, 2.0) AS real) AS result FROM (SELECT UNNEST($1) AS val, generate_subscripts($1, 1) AS ix) tuple ORDER BY ix) inn;
$$ LANGUAGE SQL IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION VECTOR_RMS(arr1 real[], samples real)
RETURNS real[] AS $$
  SELECT array_agg(result) FROM (SELECT CAST(SQRT(tuple.val / samples) AS real) AS result FROM (SELECT UNNEST($1) AS val, generate_subscripts($1, 1) AS ix) tuple ORDER BY ix) inn;
$$ LANGUAGE SQL IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION VECTOR_ADD(arr1 real[], arr2 real[])
RETURNS real[] AS $$
SELECT array_agg(result) FROM (SELECT tuple.val1 + tuple.val2 AS result FROM (SELECT UNNEST($1) AS val1,UNNEST($2) AS val2, generate_subscripts($1, 1) AS ix) tuple ORDER BY ix) inn;
$$ LANGUAGE SQL IMMUTABLE STRICT;

DROP AGGREGATE IF EXISTS VECTOR_SUM(real[]);
CREATE AGGREGATE VECTOR_SUM(real[]) 
  ( sfunc = vector_add,
    stype = real[]
  );

--CREATE OR REPLACE FUNCTION CREATE_OR_UPDATE_EVENT(in_starttime bigint, in_duration real, in_type varchar, in_voltage real, in_percent real, in_samples real[])
--RETURNS void AS $$
--  -- Inserto en la tabla event
--  INSERT INTO events(timestamp, duration, type, voltage, percent, samples)
--  VALUES(in_timestamp, null, in_type, in_voltage, in_percent, in_samples)
--  ON CONFLICT(timestamp) DO UPDATE SET duration = in_duration, type = in_type, voltage = in_voltage, percent = in_percent, samples = in_samples;
--$$ LANGUAGE SQL VOLATILE STRICT;

CREATE OR REPLACE FUNCTION CREATE_10C(in_node int, in_sampletime bigint, in_rms_v real, in_rms_i real, in_active_power real, in_reactive_power real, in_apparent_power real, in_power_factor real, in_active_energy real, in_reactive_energy real, in_fundamental_active_power real, in_fundamental_reactive_power real, in_fundamental_apparent_power real, in_fundamental_power_factor real, in_thd_v real, in_thd_i real, in_freq real, in_phi real, in_flaged boolean, in_harmonics_v real[], in_harmonics_i real[])
RETURNS void AS $$
  -- Elimino muestras antiguas
  DELETE FROM agreg_10c WHERE sampletime < (in_sampletime - 14400000) AND node = in_node;
  -- Inserto en la tabla 10c
  INSERT INTO agreg_10c(   node,    sampletime,    rms_v,    rms_i,    active_power,    reactive_power,    apparent_power,    power_factor,    active_energy,    reactive_energy,    fundamental_active_power,    fundamental_reactive_power,    fundamental_apparent_power,    fundamental_power_factor,    thd_v,    thd_i,    freq,    phi,    flaged,    harmonics_v,    harmonics_i) 
                 VALUES(in_node, in_sampletime, in_rms_v, in_rms_i, in_active_power, in_reactive_power, in_apparent_power, in_power_factor, in_active_energy, in_reactive_energy, in_fundamental_active_power, in_fundamental_reactive_power, in_fundamental_apparent_power, in_fundamental_power_factor, in_thd_v, in_thd_i, in_freq, in_phi, in_flaged, in_harmonics_v, in_harmonics_i);
$$ LANGUAGE SQL VOLATILE;

CREATE OR REPLACE FUNCTION AGREGATE_3S(in_sampletime bigint, in_node int)
RETURNS void AS $$
  -- El timestamp de entrada es el de la ultima muestra añadida en la tabla 10C
  -- Eliminar filas con mas de 60 horas de antiguedad
  DELETE FROM agreg_3s WHERE sampletime < (in_sampletime - 216000000) AND node = in_node;
  -- Inserto en la tabla 1m los valores que selecciono de la tabla 1m
  INSERT INTO agreg_3s(node, sampletime,        rms_v,                      rms_i,                      active_power,      reactive_power,      apparent_power,      power_factor,      active_energy,      reactive_energy,      fundamental_active_power,      fundamental_reactive_power,      fundamental_apparent_power,      fundamental_power_factor,      thd_v,                      thd_i,                      freq,      phi,      flaged,          harmonics_v,                                                harmonics_i) 
                SELECT node, MAX(sampletime),   SQRT(AVG(POW(rms_v, 2.0))), SQRT(AVG(POW(rms_i, 2.0))), AVG(active_power), AVG(reactive_power), AVG(apparent_power), AVG(power_factor), SUM(active_energy), SUM(reactive_energy), AVG(fundamental_active_power), AVG(fundamental_reactive_power), AVG(fundamental_apparent_power), AVG(fundamental_power_factor), SQRT(AVG(POW(thd_v, 2.0))), SQRT(AVG(POW(thd_i, 2.0))), AVG(freq), AVG(phi), BOOL_OR(flaged), VECTOR_RMS(VECTOR_SUM(VECTOR_POW2(harmonics_v)), COUNT(*)), VECTOR_RMS(VECTOR_SUM(VECTOR_POW2(harmonics_i)), COUNT(*)) 
  FROM ( SELECT        node, sampletime, rms_v, rms_i,                      active_power,      reactive_power,      apparent_power,      power_factor,      active_energy,      reactive_energy,      fundamental_active_power,      fundamental_reactive_power,      fundamental_apparent_power,      fundamental_power_factor,      thd_v,                      thd_i,                      freq,      phi,      flaged,          harmonics_v,                                                harmonics_i
         FROM agreg_10c WHERE sampletime > (in_sampletime - 3600) AND sampletime <= in_sampletime AND node = in_node ORDER BY sampletime DESC LIMIT 15) AS Table1 GROUP BY node;
$$ LANGUAGE SQL VOLATILE STRICT;

CREATE OR REPLACE FUNCTION AGREGATE_1M(in_sampletime bigint, in_node int)
RETURNS void AS $$
  -- El timestamp de entrada es cualquier instante de tiempo DENTRO del periodo de 1 minuto que se quiere agregar
  -- Eliminar filas con mas de 50 dias de antiguedad
  DELETE FROM agreg_1m WHERE sampletime < (in_sampletime - 4320000000) AND node = in_node;
  -- Inserto en la tabla 1m los valores que selecciono de la tabla 1m
  INSERT INTO agreg_1m(node, sampletime,                                      rms_v,                      rms_i,                      active_power,      reactive_power,      apparent_power,      power_factor,      active_energy,      reactive_energy,      fundamental_active_power,      fundamental_reactive_power,      fundamental_apparent_power,      fundamental_power_factor,      thd_v,                      thd_i,                      freq,      phi,      flaged,          harmonics_v,                                                harmonics_i) 
                SELECT node, sampletime + 59999,                              SQRT(AVG(POW(rms_v, 2.0))), SQRT(AVG(POW(rms_i, 2.0))), AVG(active_power), AVG(reactive_power), AVG(apparent_power), AVG(power_factor), SUM(active_energy), SUM(reactive_energy), AVG(fundamental_active_power), AVG(fundamental_reactive_power), AVG(fundamental_apparent_power), AVG(fundamental_power_factor), SQRT(AVG(POW(thd_v, 2.0))), SQRT(AVG(POW(thd_i, 2.0))), AVG(freq), AVG(phi), BOOL_OR(flaged), VECTOR_RMS(VECTOR_SUM(VECTOR_POW2(harmonics_v)), COUNT(*)), VECTOR_RMS(VECTOR_SUM(VECTOR_POW2(harmonics_i)), COUNT(*)) 
  FROM ( SELECT        node, TRUNC(sampletime / 60000) * 60000 AS sampletime, rms_v,                      rms_i,                      active_power,      reactive_power,      apparent_power,      power_factor,      active_energy,      reactive_energy,      fundamental_active_power,      fundamental_reactive_power,      fundamental_apparent_power,      fundamental_power_factor,      thd_v,                      thd_i,                      freq,      phi,      flaged,          harmonics_v,                                                harmonics_i
         FROM agreg_10c WHERE (TRUNC(sampletime / 60000) * 60000) = (TRUNC(in_sampletime / 60000) * 60000) AND node = in_node) AS Table1
  GROUP BY sampletime, node;
$$ LANGUAGE SQL VOLATILE STRICT;

CREATE OR REPLACE FUNCTION AGREGATE_10M(in_sampletime bigint, in_node int)
RETURNS void AS $$
  -- El timestamp de entrada es cualquier instante de tiempo DENTRO del periodo de 10 minutos que se quiere agregar
  -- Eliminar filas con mas de 500 dias de antiguedad
  DELETE FROM agreg_10m WHERE sampletime < (in_sampletime - 43200000000) AND node = in_node;
  -- Inserto en la tabla 10m los valores que selecciono de la tabla 1m
  INSERT INTO agreg_10m(node, sampletime,                       rms_v,                      rms_i,                      active_power,      reactive_power,      apparent_power,      power_factor,      active_energy,      reactive_energy,      fundamental_active_power,      fundamental_reactive_power,      fundamental_apparent_power,      fundamental_power_factor,      thd_v,                      thd_i,                      freq,      phi,      flaged,          harmonics_v,                                                harmonics_i) 
                 SELECT node, sampletime + 599999,              SQRT(AVG(POW(rms_v, 2.0))), SQRT(AVG(POW(rms_i, 2.0))), AVG(active_power), AVG(reactive_power), AVG(apparent_power), AVG(power_factor), SUM(active_energy), SUM(reactive_energy), AVG(fundamental_active_power), AVG(fundamental_reactive_power), AVG(fundamental_apparent_power), AVG(fundamental_power_factor), SQRT(AVG(POW(thd_v, 2.0))), SQRT(AVG(POW(thd_i, 2.0))), AVG(freq), AVG(phi), BOOL_OR(flaged), VECTOR_RMS(VECTOR_SUM(VECTOR_POW2(harmonics_v)), COUNT(*)), VECTOR_RMS(VECTOR_SUM(VECTOR_POW2(harmonics_i)), COUNT(*)) 
  FROM ( SELECT         node, TRUNC(sampletime / 600000) * 600000 AS sampletime, rms_v,                      rms_i,                      active_power,      reactive_power,      apparent_power,      power_factor,      active_energy,      reactive_energy,      fundamental_active_power,      fundamental_reactive_power,      fundamental_apparent_power,      fundamental_power_factor,      thd_v,                      thd_i,                      freq,      phi,      flaged,          harmonics_v,                                                harmonics_i
         FROM agreg_1m WHERE (TRUNC(sampletime / 600000) * 600000) = (TRUNC(in_sampletime / 600000) * 600000) AND node = in_node) AS Table1
  GROUP BY sampletime, node;
$$ LANGUAGE SQL VOLATILE STRICT;

CREATE OR REPLACE FUNCTION AGREGATE_1H(in_sampletime bigint, in_node int)
RETURNS void AS $$
  -- El timestamp de entrada es cualquier instante de tiempo DENTRO del periodo de 1 hora que se quiere agregar
  -- Eliminar filas con mas de 500 dias de antiguedad
  DELETE FROM agreg_1h WHERE sampletime < (in_sampletime - 518400000000) AND node = in_node;
--  -- Inserto en la tabla 10m los valores que selecciono de la tabla 1m
  INSERT INTO agreg_1h( node, sampletime,                         rms_v,                      rms_i,                      active_power,      reactive_power,      apparent_power,      power_factor,      active_energy,      reactive_energy,      fundamental_active_power,      fundamental_reactive_power,      fundamental_apparent_power,      fundamental_power_factor,      thd_v,                      thd_i,                      freq,      phi,      flaged,          harmonics_v,                                                harmonics_i) 
                 SELECT node, sampletime + 7199999,              SQRT(AVG(POW(rms_v, 2.0))), SQRT(AVG(POW(rms_i, 2.0))), AVG(active_power), AVG(reactive_power), AVG(apparent_power), AVG(power_factor), SUM(active_energy), SUM(reactive_energy), AVG(fundamental_active_power), AVG(fundamental_reactive_power), AVG(fundamental_apparent_power), AVG(fundamental_power_factor), SQRT(AVG(POW(thd_v, 2.0))), SQRT(AVG(POW(thd_i, 2.0))), AVG(freq), AVG(phi), BOOL_OR(flaged), VECTOR_RMS(VECTOR_SUM(VECTOR_POW2(harmonics_v)), COUNT(*)), VECTOR_RMS(VECTOR_SUM(VECTOR_POW2(harmonics_i)), COUNT(*)) 
  FROM ( SELECT         node, TRUNC(sampletime / 3600000) * 3600000 AS sampletime, rms_v,                      rms_i,                      active_power,      reactive_power,      apparent_power,      power_factor,      active_energy,      reactive_energy,      fundamental_active_power,      fundamental_reactive_power,      fundamental_apparent_power,      fundamental_power_factor,      thd_v,                      thd_i,                      freq,      phi,      flaged,          harmonics_v,                                                harmonics_i
         FROM agreg_10m WHERE (TRUNC(sampletime / 3600000) * 3600000) = (TRUNC(in_sampletime / 3600000) * 3600000) AND node = in_node) AS Table1
  GROUP BY sampletime, node;
$$ LANGUAGE SQL VOLATILE STRICT;

CREATE OR REPLACE FUNCTION STARTUP(in_sampletime bigint)
RETURNS bigint AS $$
  -- Eliminar filas con mas de 50/500/5000 dias de antiguedad segun tabla
  DELETE FROM agreg_1m WHERE sampletime < (in_sampletime - 4320000000);
  DELETE FROM agreg_10m WHERE sampletime < (in_sampletime - 43200000000);
  DELETE FROM agreg_1h WHERE sampletime < (in_sampletime - 432000000000);

  -- Inserto en la tabla 1m                                                                                                                                                                                                                
  INSERT INTO agreg_1m(node, sampletime,                                rms_v,                      rms_i,                      active_power,      reactive_power,      apparent_power,      power_factor,      active_energy,      reactive_energy,      fundamental_active_power,      fundamental_reactive_power,      fundamental_apparent_power,      fundamental_power_factor,      thd_v,                      thd_i,                      freq,      phi,      flaged,          harmonics_v,                                                harmonics_i) 
                SELECT node, sampletime + 59999,                       SQRT(AVG(POW(rms_v, 2.0))), SQRT(AVG(POW(rms_i, 2.0))), AVG(active_power), AVG(reactive_power), AVG(apparent_power), AVG(power_factor), SUM(active_energy), SUM(reactive_energy), AVG(fundamental_active_power), AVG(fundamental_reactive_power), AVG(fundamental_apparent_power), AVG(fundamental_power_factor), SQRT(AVG(POW(thd_v, 2.0))), SQRT(AVG(POW(thd_i, 2.0))), AVG(freq), AVG(phi), BOOL_OR(flaged), VECTOR_RMS(VECTOR_SUM(VECTOR_POW2(harmonics_v)), COUNT(*)), VECTOR_RMS(VECTOR_SUM(VECTOR_POW2(harmonics_i)), COUNT(*)) 
  FROM ( SELECT node, TRUNC(sampletime / 60000) * 60000 AS sampletime, rms_v,                      rms_i,                      active_power,      reactive_power,      apparent_power,      power_factor,      active_energy,      reactive_energy,      fundamental_active_power,      fundamental_reactive_power,      fundamental_apparent_power,      fundamental_power_factor,      thd_v,                      thd_i,                      freq,      phi,      flaged,          harmonics_v,                                                harmonics_i
         FROM agreg_10c WHERE sampletime > COALESCE((SELECT MAX(sampletime) FROM agreg_1m), 0) AND sampletime < (TRUNC(in_sampletime / 60000) * 60000)) AS Table1                                                                                
  GROUP BY sampletime, node; 
  
  -- Inserto en la tabla 10m                                                                                                                                                                                                                
  INSERT INTO agreg_10m(node, sampletime,                                rms_v,                      rms_i,                      active_power,      reactive_power,      apparent_power,      power_factor,      active_energy,      reactive_energy,      fundamental_active_power,      fundamental_reactive_power,      fundamental_apparent_power,      fundamental_power_factor,      thd_v,                      thd_i,                      freq,      phi,      flaged,          harmonics_v,                                                harmonics_i) 
                 SELECT node, sampletime + 599999,                       SQRT(AVG(POW(rms_v, 2.0))), SQRT(AVG(POW(rms_i, 2.0))), AVG(active_power), AVG(reactive_power), AVG(apparent_power), AVG(power_factor), SUM(active_energy), SUM(reactive_energy), AVG(fundamental_active_power), AVG(fundamental_reactive_power), AVG(fundamental_apparent_power), AVG(fundamental_power_factor), SQRT(AVG(POW(thd_v, 2.0))), SQRT(AVG(POW(thd_i, 2.0))), AVG(freq), AVG(phi), BOOL_OR(flaged), VECTOR_RMS(VECTOR_SUM(VECTOR_POW2(harmonics_v)), COUNT(*)), VECTOR_RMS(VECTOR_SUM(VECTOR_POW2(harmonics_i)), COUNT(*)) 
  FROM ( SELECT node, TRUNC(sampletime / 600000) * 600000 AS sampletime, rms_v,                      rms_i,                      active_power,      reactive_power,      apparent_power,      power_factor,      active_energy,      reactive_energy,      fundamental_active_power,      fundamental_reactive_power,      fundamental_apparent_power,      fundamental_power_factor,      thd_v,                      thd_i,                      freq,      phi,      flaged,          harmonics_v,                                                harmonics_i
         FROM agreg_1m WHERE sampletime > COALESCE((SELECT MAX(sampletime) FROM agreg_10m), 0) AND sampletime < (TRUNC(in_sampletime / 600000) * 600000)) AS Table1                                                                                
  GROUP BY sampletime, node;                                                                                                                                                                                                                     
                                                                                                                                                                                                                                            
  -- Inserto en la tabla 1h                                                                                                                                                                                                                 
  INSERT INTO agreg_1h(node, sampletime,                                   rms_v,                      rms_i,                      active_power,      reactive_power,      apparent_power,      power_factor,      active_energy,      reactive_energy,      fundamental_active_power,      fundamental_reactive_power,      fundamental_apparent_power,      fundamental_power_factor,      thd_v,                      thd_i,                      freq,      phi,      flaged,          harmonics_v,                                                harmonics_i)
                SELECT node, sampletime + 3599999,                         SQRT(AVG(POW(rms_v, 2.0))), SQRT(AVG(POW(rms_i, 2.0))), AVG(active_power), AVG(reactive_power), AVG(apparent_power), AVG(power_factor), SUM(active_energy), SUM(reactive_energy), AVG(fundamental_active_power), AVG(fundamental_reactive_power), AVG(fundamental_apparent_power), AVG(fundamental_power_factor), SQRT(AVG(POW(thd_v, 2.0))), SQRT(AVG(POW(thd_i, 2.0))), AVG(freq), AVG(phi), BOOL_OR(flaged), VECTOR_RMS(VECTOR_SUM(VECTOR_POW2(harmonics_v)), COUNT(*)), VECTOR_RMS(VECTOR_SUM(VECTOR_POW2(harmonics_i)), COUNT(*)) 
  FROM ( SELECT node, TRUNC(sampletime / 3600000) * 3600000 AS sampletime, rms_v,                      rms_i,                      active_power,      reactive_power,      apparent_power,      power_factor,      active_energy,      reactive_energy,      fundamental_active_power,      fundamental_reactive_power,      fundamental_apparent_power,      fundamental_power_factor,      thd_v,                      thd_i,                      freq,      phi,      flaged,          harmonics_v,                                                harmonics_i
         FROM agreg_10m WHERE sampletime > COALESCE((SELECT MAX(sampletime) FROM agreg_1h), 0) AND sampletime < (TRUNC(in_sampletime / 3600000) * 3600000)) AS Table1                                                                                             
  GROUP BY sampletime, node;                                                                                                                                                                                                                     
  -- Obtiene la ultima tarifa de precios descargada
  SELECT MAX(starttime) FROM prices;
$$ LANGUAGE sql VOLATILE;

CREATE OR REPLACE FUNCTION INSERT_PRICES(in_starttime bigint, in_price real[])
RETURNS void AS $$
  INSERT INTO prices(starttime, price) 
  SELECT in_starttime + (3600 * (ROW_NUMBER()OVER () - 1)), Table1.*  FROM (SELECT UNNEST(in_price)) AS Table1
  ON CONFLICT (starttime) DO NOTHING;
  
$$ LANGUAGE sql VOLATILE;