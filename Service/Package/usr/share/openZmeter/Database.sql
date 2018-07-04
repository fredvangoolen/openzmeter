-- Create ROLE
CREATE ROLE openzmeter;
ALTER ROLE openzmeter WITH LOGIN ENCRYPTED PASSWORD 'openzmeter';


CREATE DATABASE openzmeter;
ALTER DATABASE openzmeter OWNER TO openzmeter;
ALTER DATABASE openzmeter WITH CONNECTION LIMIT = -1;
--GRANT CONNECT, TEMPORARY ON DATABASE openzmeter TO public;
GRANT ALL ON DATABASE openzmeter TO openzmeter;

\connect openzmeter;




