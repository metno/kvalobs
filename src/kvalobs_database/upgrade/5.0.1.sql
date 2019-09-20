BEGIN;

-- drop view pdata;

DROP VIEW data;
CREATE VIEW data AS (
    SELECT 
        o.stationid,
        o.obstime,
        d.original,
        d.paramid,
        o.tbtime,
        o.typeid,
        d.sensor,
        d.level,
        d.corrected,
        d.controlinfo,
        d.useinfo,
        d.cfailed
    FROM 
        observations o,
        obsdata d
    WHERE
        o.observationid = d.observationid
);
REVOKE ALL ON data FROM public;
GRANT ALL ON data TO kv_admin;
GRANT SELECT ON data TO kv_read;
GRANT SELECT ON data TO kv_write;

CREATE OR REPLACE FUNCTION 
	backup_old_data() 
RETURNS trigger AS
$BODY$
DECLARE
    obs observations%ROWTYPE;
BEGIN
    SELECT * into obs FROM observations WHERE observationid=NEW.observationid;
	INSERT INTO data_history 
		(stationid,obstime,original,paramid,tbtime,typeid,sensor,level,corrected,controlinfo,useinfo,cfailed)
	VALUES
		(obs.stationid,obs.obstime,NEW.original,NEW.paramid,obs.tbtime,obs.typeid,NEW.sensor,NEW.level,NEW.corrected,NEW.controlinfo,NEW.useinfo,NEW.cfailed);
	RETURN NULL;
END;
$BODY$
LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION 
	backup_old_data_delete() 
RETURNS trigger AS
$BODY$
DECLARE
    obs observations%ROWTYPE;
BEGIN
    SELECT * into obs FROM observations WHERE observationid=OLD.observationid;
    IF NOT FOUND THEN
        RETURN NULL;
    END IF;
	INSERT INTO data_history 
		(stationid,obstime,original,paramid,tbtime,typeid,sensor,level,corrected,controlinfo,useinfo,cfailed)
	VALUES
		(obs.stationid,obs.obstime,NULL,OLD.paramid,obs.tbtime,obs.typeid,OLD.sensor,OLD.level,NULL,NULL,NULL,NULL);
	RETURN NULL;
END;
$BODY$
LANGUAGE 'plpgsql';

ALTER TABLE obsdata DROP COLUMN obs_offset;



drop view text_data;
CREATE VIEW text_data AS (
    SELECT 
        o.stationid,
        o.obstime,
        d.original,
        d.paramid,
        o.tbtime,
        o.typeid
    FROM 
        observations o,
        obstextdata d
    WHERE
        o.observationid = d.observationid
);

REVOKE ALL ON text_data FROM public;
GRANT ALL ON text_data TO kv_admin;
GRANT SELECT ON text_data TO kv_read;
GRANT SELECT ON text_data TO kv_write;

CREATE OR REPLACE FUNCTION 
	backup_old_text_data() 
RETURNS trigger AS
$BODY$
DECLARE
    obs observations%ROWTYPE;
BEGIN
    SELECT * into obs FROM observations WHERE observationid=NEW.observationid;
	INSERT INTO text_data_history
		(stationid,obstime,original,paramid,tbtime,typeid)
	VALUES
		(obs.stationid,obs.obstime,NEW.original,NEW.paramid,obs.tbtime,obs.typeid);
	RETURN NULL;
END;
$BODY$
LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION 
	backup_old_text_data_delete() 
RETURNS trigger AS
$BODY$
DECLARE
    obs observations%ROWTYPE;
BEGIN
    SELECT * into obs FROM observations WHERE observationid=OLD.observationid;
    IF NOT FOUND THEN
        RETURN NULL;
    END IF;
	INSERT INTO text_data_history 
		(stationid,obstime,original,paramid,tbtime,typeid)
	VALUES
		(obs.stationid,obs.obstime,NULL,OLD.paramid,obs.tbtime,obs.typeid);
	RETURN NULL;
END;
$BODY$
LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION 
kvalobs_database_version()
RETURNS text AS
$BODY$
BEGIN
	RETURN '5.0.1';
END;
$BODY$
LANGUAGE plpgsql IMMUTABLE;

ALTER TABLE obstextdata DROP COLUMN obs_offset;

CREATE index ON obstextdata (observationid);

CREATE VIEW pdata AS
    SELECT data.stationid, data.obstime, data.tbtime, data.typeid, (SELECT param.name FROM param WHERE (param.paramid = data.paramid))
    AS paramid, data.original, data.corrected, data.sensor, data.level, data.controlinfo, data.useinfo, data.cfailed FROM data;
REVOKE ALL ON pdata FROM public;
GRANT ALL ON pdata TO kv_admin;
GRANT SELECT ON pdata TO kv_read;
GRANT SELECT ON pdata TO kv_write;


COMMIT;