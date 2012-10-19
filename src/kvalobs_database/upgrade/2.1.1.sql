BEGIN;

TRUNCATE data_history;
TRUNCATE text_data_history;


CREATE INDEX data_history_main_index ON data_history (stationid, obstime, paramid, level, sensor, typeid);
CREATE INDEX data_history_obstime_index ON data_history (obstime);
CREATE INDEX data_history_tbtime_index ON data_history (tbtime);

DROP TRIGGER backup_data ON data;
CREATE TRIGGER backup_data AFTER INSERT OR UPDATE ON data FOR EACH ROW EXECUTE PROCEDURE backup_old_data();
DROP TRIGGER backup_text_data ON text_data;
CREATE TRIGGER backup_text_data AFTER INSERT OR UPDATE ON text_data FOR EACH ROW EXECUTE PROCEDURE backup_old_text_data();

ALTER TABLE data_history ADD COLUMN modificationtime timestamp NOT NULL DEFAULT timezone('UTC', now());
CREATE INDEX data_history_modificationtime_index ON data_history (modificationtime);

CREATE OR REPLACE FUNCTION 
	backup_old_data() 
RETURNS trigger AS
$BODY$
BEGIN
	INSERT INTO data_history 
		(stationid,obstime,original,paramid,tbtime,typeid,sensor,level,corrected,controlinfo,useinfo,cfailed)
	VALUES
		(NEW.stationid,NEW.obstime,NEW.original,NEW.paramid,NEW.tbtime,NEW.typeid,NEW.sensor,NEW.level,NEW.corrected,NEW.controlinfo,NEW.useinfo,NEW.cfailed);
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
		(OLD.stationid,OLD.obstime,NULL,OLD.paramid,OLD.tbtime,OLD.typeid,OLD.sensor,OLD.level,NULL,NULL,NULL,NULL);
	RETURN NULL;
END;
$BODY$
LANGUAGE 'plpgsql';




ALTER TABLE text_data_history ADD COLUMN modificationtime timestamp NOT NULL DEFAULT timezone('UTC', now());
CREATE INDEX text_data_history_modificationtime_index ON text_data_history (modificationtime);
CREATE INDEX text_data_history_main_index ON text_data_history (stationid, obstime, paramid, typeid);


CREATE OR REPLACE FUNCTION 
	backup_old_text_data() 
RETURNS trigger AS
$BODY$
BEGIN
	INSERT INTO text_data_history
		(stationid,obstime,original,paramid,tbtime,typeid)
	VALUES
		(NEW.stationid,NEW.obstime,NEW.original,NEW.paramid,NEW.tbtime,NEW.typeid);
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
		(OLD.stationid,OLD.obstime,NULL,OLD.paramid,OLD.tbtime,OLD.typeid);
	RETURN NULL;
END;
$BODY$
LANGUAGE 'plpgsql';

CREATE TABLE metadatatype (
        metadatatypename text NOT NULL,
        description TEXT NOT NULL,
        PRIMARY KEY ( metadatatypename )
);
	
REVOKE ALL ON metadatatype FROM public;
GRANT ALL ON metadatatype TO kv_admin;
GRANT SELECT ON metadatatype TO kv_read;
GRANT SELECT, UPDATE, INSERT ON metadatatype TO kv_write;



CREATE TABLE station_metadata (
        stationid INTEGER NOT NULL,
        paramid INTEGER DEFAULT NULL,
        typeid INTEGER DEFAULT NULL,
        level INTEGER DEFAULT NULL,
        sensor CHAR(1) DEFAULT NULL,
        metadatatypename TEXT NOT NULL,
        metadata float NOT NULL,
        fromtime TIMESTAMP NOT NULL,
        totime TIMESTAMP DEFAULT NULL,
        UNIQUE ( stationid, paramid, typeid, level, sensor, metadatatypename, fromtime ),
        FOREIGN KEY(metadatatypename) REFERENCES metadatatype
); 

REVOKE ALL ON station_metadata FROM public;
GRANT ALL ON station_metadata TO kv_admin;
GRANT SELECT ON station_metadata TO kv_read;
GRANT SELECT, UPDATE, INSERT ON station_metadata TO kv_write;


COMMIT;
