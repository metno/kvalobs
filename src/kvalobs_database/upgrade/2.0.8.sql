BEGIN;

DROP TRIGGER backup_data ON data;
CREATE TRIGGER backup_data AFTER INSERT OR UPDATE ON data FOR EACH ROW EXECUTE PROCEDURE backup_old_data();
DROP TRIGGER backup_text_data ON text_data;
CREATE TRIGGER backup_text_data AFTER INSERT OR UPDATE ON text_data FOR EACH ROW EXECUTE PROCEDURE backup_old_text_data();

CREATE OR REPLACE FUNCTION 
	backup_old_data() 
RETURNS trigger AS
$BODY$
BEGIN
	INSERT INTO data_history 
		(stationid,obstime,original,paramid,tbtime,typeid,sensor,level,corrected,controlinfo,useinfo,cfailed)
	VALUES
		(NEW.stationid,NEW.obstime,NEW.original,NEW.paramid,now(),NEW.typeid,NEW.sensor,NEW.level,NEW.corrected,NEW.controlinfo,NEW.useinfo,NEW.cfailed);
	RETURN NULL;
END;
$BODY$
LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION 
	backup_old_data_delete() 
RETURNS trigger AS
$BODY$
BEGIN
	INSERT INTO data_history 
		(stationid,obstime,original,paramid,tbtime,typeid,sensor,level,corrected,controlinfo,useinfo,cfailed)
	VALUES
		(OLD.stationid,OLD.obstime,NULL,OLD.paramid,now(),OLD.typeid,OLD.sensor,OLD.level,NULL,NULL,NULL,NULL);
	RETURN NULL;
END;
$BODY$
LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION 
	backup_old_text_data() 
RETURNS trigger AS
$BODY$
BEGIN
	INSERT INTO text_data_history
		(stationid,obstime,original,paramid,tbtime,typeid)
	VALUES
		(NEW.stationid,NEW.obstime,NEW.original,NEW.paramid,now(),NEW.typeid);
	RETURN NULL;
END;
$BODY$
LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION 
	backup_old_text_data_delete() 
RETURNS trigger AS
$BODY$
BEGIN
	INSERT INTO text_data_history 
		(stationid,obstime,original,paramid,tbtime,typeid)
	VALUES
		(OLD.stationid,OLD.obstime,NULL,OLD.paramid,now(),OLD.typeid);
	RETURN NULL;
END;
$BODY$
LANGUAGE 'plpgsql';

COMMIT;
