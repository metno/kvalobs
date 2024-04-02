BEGIN;

ALTER TABLE model_data ADD COLUMN tbtime timestamp DEFAULT current_timestamp(0);

CREATE VIEW pmodel_data AS 
    SELECT stationid,obstime, (SELECT name FROM param WHERE paramid=model_data.paramid) AS paramid, 
    level, modelid, original, tbtime FROM model_data;
REVOKE ALL ON pmodel_data FROM public;
GRANT ALL ON pmodel_data TO kv_admin;
GRANT SELECT ON pmodel_data TO kv_read;
GRANT SELECT ON pmodel_data TO kv_write;


CREATE OR REPLACE FUNCTION 
kvalobs_database_version()
RETURNS text AS
$BODY$
BEGIN
	RETURN '5.0.4';
END;
$BODY$
LANGUAGE plpgsql IMMUTABLE;

END;
