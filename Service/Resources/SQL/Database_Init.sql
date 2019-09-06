DO $do$ BEGIN
  IF NOT EXISTS (SELECT FROM pg_catalog.pg_roles WHERE rolname = 'openzmeter') THEN
    CREATE ROLE openzmeter LOGIN PASSWORD 'openzmeter';
  END IF;
END $do$;

DO $do$ BEGIN
  IF NOT EXISTS (SELECT FROM pg_catalog.pg_database WHERE datname = 'openzmeter') THEN
    CREATE DATABASE openzmeter;
    ALTER DATABASE openzmeter OWNER TO openzmeter;
  END IF;
END $do$;