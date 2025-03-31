BEGIN;


CREATE VIEW pstation_param AS
    SELECT station_param.stationid, (SELECT param.name FROM param WHERE (param.paramid = station_param.paramid))
    AS paramid, station_param.level, station_param.sensor, station_param.fromday, station_param.today,
    station_param.hour,station_param.qcx, station_param.metadata, station_param.desc_metadata,station_param.fromtime FROM station_param;
REVOKE ALL ON pstation_param FROM public;
GRANT ALL ON pstation_param TO kv_admin;
GRANT SELECT ON pstation_param TO kv_read;
GRANT SELECT ON pstation_param TO kv_write;

CREATE VIEW pstation_metadata AS
    SELECT sm.stationid, (SELECT param.name FROM param WHERE (param.paramid = sm.paramid)) AS paramid, 
        sm.typeid, sm.level, sm.sensor, sm.metadatatypename, sm.metadata, sm.fromtime, sm.totime
    FROM station_metadata as sm;
REVOKE ALL ON pstation_metadata FROM public;
GRANT ALL ON pstation_metadata TO kv_admin;
GRANT SELECT ON pstation_metadata TO kv_read;
GRANT SELECT ON pstation_metadata TO kv_write;


CREATE OR REPLACE FUNCTION 
kvalobs_database_version()
RETURNS text AS
$BODY$
BEGIN
	RETURN '5.0.5';
END;
$BODY$
LANGUAGE plpgsql IMMUTABLE;


END;