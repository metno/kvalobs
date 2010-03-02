BEGIN;

CREATE OR REPLACE FUNCTION 
	backup_old_data() 
RETURNS trigger AS
'
BEGIN
	INSERT INTO data_history 
		(stationid,obstime,original,paramid,tbtime,typeid,sensor,level,corrected,controlinfo,useinfo,cfailed)
	VALUES
		(OLD.stationid,OLD.obstime,OLD.original,OLD.paramid,now(),OLD.typeid,OLD.sensor,OLD.level,OLD.corrected,OLD.controlinfo,OLD.useinfo,OLD.cfailed);
	RETURN NULL;
END;
'
LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION 
	backup_old_data_delete() 
RETURNS trigger AS
'
BEGIN
	INSERT INTO data_history 
		(stationid,obstime,original,paramid,tbtime,typeid,sensor,level,corrected,controlinfo,useinfo,cfailed)
	VALUES
		(OLD.stationid,OLD.obstime,NULL,OLD.paramid,now(),OLD.typeid,OLD.sensor,OLD.level,NULL,NULL,NULL,NULL);
	RETURN NULL;
END;
'
LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION 
	backup_old_text_data() 
RETURNS trigger AS
'
BEGIN
	INSERT INTO text_data_history
		(stationid,obstime,original,paramid,tbtime,typeid)
	VALUES
		(OLD.stationid,OLD.obstime,OLD.original,OLD.paramid,now(),OLD.typeid);
	RETURN NULL;
END;
'
LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION 
	backup_old_text_data_delete() 
RETURNS trigger AS
'
BEGIN
	INSERT INTO text_data_history 
		(stationid,obstime,original,paramid,tbtime,typeid)
	VALUES
		(OLD.stationid,OLD.obstime,NULL,OLD.paramid,now(),OLD.typeid);
	RETURN NULL;
END;
'
LANGUAGE plpgsql;

COMMIT;
