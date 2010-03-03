-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
-- 
-- kvalobs - quality assurance system for meteorological observations.
--
-- Copyright (C) 2007 met.no
--
--  Contact information:
--  Norwegian Meteorological Institute
--  Box 43 Blindern
--  0313 OSLO
--  NORWAY
--  E-mail: kvoss@met.no
--
--  This is free software; you can redistribute it and/or modify
--  it under the terms of the GNU General Public License as published by
--  the Free Software Foundation; either version 2 of the License, or
--  (at your option) any later version.
--
-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


CREATE TABLE data (
	stationid   INTEGER NOT NULL,
	obstime     TIMESTAMP NOT NULL,
	original    FLOAT NOT NULL,
	paramid	    INTEGER NOT NULL,
	tbtime	    TIMESTAMP NOT NULL,
	typeid	    INTEGER NOT NULL,
	sensor	    CHAR(1) DEFAULT '0',
	level	    INTEGER DEFAULT 0,
	corrected   FLOAT NOT NULL,
	controlinfo CHAR(16) DEFAULT '0000000000000000',
	useinfo     CHAR(16) DEFAULT '0000000000000000',
	cfailed     TEXT DEFAULT NULL,    	
	UNIQUE ( stationid, obstime, paramid, level, sensor, typeid ) 
);

CREATE INDEX data_obstime_index ON data (obstime);
CREATE INDEX data_tbtime_index ON data (tbtime);

REVOKE ALL ON data FROM public;
GRANT ALL ON data TO kv_admin;
GRANT SELECT ON data TO kv_read;
GRANT SELECT, UPDATE, INSERT, DELETE ON data TO kv_write;


--
-- This table maintains a complete history of everything that has happened to 
-- the data table.
-- The version row contains the version of the data - a higher number is newer.
-- Since each number in the version row is unique, you may see the entire 
-- data history in this table.  
-- 
CREATE TABLE data_history (
	version bigserial,
	stationid   INTEGER NOT NULL,
	obstime     TIMESTAMP NOT NULL,
	original    FLOAT,
	paramid	    INTEGER NOT NULL,
	tbtime	    TIMESTAMP NOT NULL,
	typeid	    INTEGER NOT NULL,
	sensor	    CHAR(1) DEFAULT '0',
	level	    INTEGER DEFAULT 0,
	corrected   FLOAT,
	controlinfo CHAR(16) DEFAULT '0000000000000000',
	useinfo     CHAR(16) DEFAULT '0000000000000000',
	cfailed     TEXT DEFAULT NULL,    	
	UNIQUE ( version, stationid, obstime, paramid, level, sensor, typeid ) 
);

CREATE INDEX data_history_main_index ON data_history (stationid, obstime, paramid, level, sensor, typeid);
CREATE INDEX data_history_obstime_index ON data_history (obstime);
CREATE INDEX data_history_tbtime_index ON data_history (tbtime);

REVOKE ALL ON data_history FROM public;
GRANT ALL ON data_history TO kv_admin;
GRANT SELECT ON data_history TO kv_read;
GRANT SELECT, UPDATE, INSERT, DELETE ON data_history TO kv_write;
GRANT USAGE ON SEQUENCE data_history_version_seq TO kv_write;


CREATE LANGUAGE plpgsql;

--
-- Trigger function for propagating changes to the data table into data_history
--
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
CREATE TRIGGER backup_data AFTER INSERT OR UPDATE ON data FOR EACH ROW EXECUTE PROCEDURE backup_old_data();

--
-- Trigger function for propagating deletes in the data table into data_history
-- Deleted rows are marked in data_history with NULL values for original, 
-- corrected, controlinfo, useinfo and cfailed.
--
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
CREATE TRIGGER backup_data_delete AFTER DELETE ON data FOR EACH ROW EXECUTE PROCEDURE backup_old_data_delete();


CREATE TABLE text_data (
	stationid   INTEGER NOT NULL,
	obstime     TIMESTAMP NOT NULL,
	original    TEXT NOT NULL,
	paramid	    INTEGER NOT NULL,
	tbtime	    TIMESTAMP NOT NULL,
	typeid	    INTEGER NOT NULL,
	UNIQUE ( stationid, obstime, paramid, typeid ) 
);

REVOKE ALL ON text_data FROM public;
GRANT ALL ON text_data TO kv_admin;
GRANT SELECT ON text_data TO kv_read;
GRANT SELECT, UPDATE, INSERT, DELETE ON text_data TO kv_write;


--
-- This table maintains a complete history of everything that has happened to 
-- the text_data table.
-- The version row contains the version of the data - a higher number is newer.
-- Since each number in the version row is unique, you may see the entire 
-- data history in this table.  
-- 
CREATE TABLE text_data_history (
	version bigserial,
	stationid   INTEGER NOT NULL,
	obstime     TIMESTAMP NOT NULL,
	original    TEXT,
	paramid	    INTEGER NOT NULL,
	tbtime	    TIMESTAMP NOT NULL,
	typeid	    INTEGER NOT NULL,
	UNIQUE ( version, stationid, obstime, paramid, typeid ) 
);
REVOKE ALL ON text_data_history FROM public;
GRANT ALL ON text_data_history TO kv_admin;
GRANT SELECT ON text_data_history TO kv_read;
GRANT SELECT, UPDATE, INSERT, DELETE ON text_data_history TO kv_write;
GRANT USAGE ON SEQUENCE text_data_history_version_seq TO kv_write;


--
-- Trigger function for propagating changes to the text_data table into text_data_history
--
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
CREATE TRIGGER backup_text_data AFTER INSERT OR UPDATE ON text_data FOR EACH ROW EXECUTE PROCEDURE backup_old_text_data();

--
-- Trigger function for propagating deletes in the text_data table into text_data_history
-- Deleted rows are marked in text_data_history with NULL values for original, 
-- corrected, controlinfo, useinfo and cfailed.
--
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
CREATE TRIGGER backup_text_data_delete AFTER DELETE ON text_data FOR EACH ROW EXECUTE PROCEDURE backup_old_text_data_delete();



CREATE VIEW data_view AS
SELECT data.stationid, data.obstime, CAST(data.original AS TEXT), data.paramid, data.tbtime, data.typeid
FROM data, text_data
WHERE data.stationid=text_data.stationid and data.obstime=text_data.obstime
UNION ALL
SELECT text_data.stationid, text_data.obstime, text_data.original, text_data.paramid, text_data.tbtime, text_data.typeid
FROM data, text_data
WHERE data.stationid=text_data.stationid and data.obstime=text_data.obstime
UNION  ALL
SELECT data.stationid, data.obstime, CAST(data.original AS TEXT), data.paramid, data.tbtime, data.typeid
FROM data
WHERE (data.stationid NOT IN ( SELECT text_data.stationid FROM text_data )) OR 
      (data.obstime NOT IN ( SELECT text_data.obstime FROM text_data ));




CREATE VIEW text_data_view AS
SELECT text_data.stationid, text_data.obstime, text_data.original, text_data.paramid, text_data.tbtime, text_data.typeid
FROM text_data
WHERE (text_data.stationid NOT IN ( SELECT data.stationid FROM data )) OR 
      (text_data.obstime NOT IN ( SELECT data.obstime FROM data ));



CREATE TABLE rejectdecode (
	message TEXT NOT NULL,
	tbtime TIMESTAMP NOT NULL,
	decoder TEXT NOT NULL,
	comment TEXT DEFAULT NULL,
	UNIQUE ( tbtime, message, decoder )
);

REVOKE ALL ON rejectdecode FROM public;
GRANT ALL ON rejectdecode TO kv_admin;
GRANT SELECT ON rejectdecode TO kv_read;
GRANT SELECT, UPDATE, INSERT, DELETE ON rejectdecode TO kv_write;


CREATE TABLE model_data (
	stationid INTEGER NOT NULL,
	obstime	  TIMESTAMP NOT NULL,
	paramid   INTEGER NOT NULL,
	level     INTEGER NOT NULL,
        modelid   INTEGER NOT NULL,
	original  FLOAT DEFAULT NULL,  
	UNIQUE ( stationid, obstime, paramid, level, modelid )
);
REVOKE ALL ON model_data FROM public;
GRANT ALL ON model_data TO kv_admin;
GRANT SELECT ON model_data TO kv_read;
GRANT SELECT, UPDATE, INSERT ON model_data TO kv_write;



CREATE TABLE model (
	modelid  INTEGER NOT NULL,
	name    TEXT DEFAULT NULL,
	comment TEXT DEFAULT NULL,
	UNIQUE ( modelid )
);

REVOKE ALL ON model FROM public;
GRANT ALL ON model TO kv_admin;
GRANT SELECT ON model TO kv_read;
GRANT SELECT, UPDATE, INSERT ON model TO kv_write;




CREATE TABLE types (
	typeid  INTEGER NOT NULL,
	format   TEXT DEFAULT NULL,
	earlyobs INTEGER DEFAULT NULL,
        lateobs  INTEGER DEFAULT NULL,
	read     TEXT DEFAULT NULL,
        obspgm   TEXT DEFAULT NULL,
	comment  TEXT DEFAULT NULL,
	UNIQUE ( typeid )
);

REVOKE ALL ON types FROM public;
GRANT ALL ON types TO kv_admin;
GRANT SELECT ON types TO kv_read;
GRANT SELECT, UPDATE, INSERT ON types TO kv_write;



CREATE TABLE generated_types (
	stationid INTEGER NOT NULL,
	typeid  INTEGER NOT NULL,
	UNIQUE ( stationid, typeid )
);
REVOKE ALL ON generated_types FROM public;
GRANT ALL ON generated_types TO kv_admin;
GRANT SELECT ON generated_types TO kv_read;
GRANT SELECT, UPDATE, INSERT, DELETE ON generated_types TO kv_write;



CREATE TABLE param (
	paramid INTEGER NOT NULL,
	name    TEXT NOT NULL,
	description TEXT DEFAULT NULL,
	unit    TEXT DEFAULT NULL,
	level_scale INTEGER DEFAULT 0,
	comment TEXT DEFAULT NULL,
	UNIQUE ( paramid )
);

REVOKE ALL ON param FROM public;
GRANT ALL ON param TO kv_admin;
GRANT SELECT ON param TO kv_read;
GRANT SELECT, UPDATE, INSERT ON param TO kv_write;


CREATE TABLE station (
	stationid INTEGER NOT NULL,
	lat FLOAT DEFAULT NULL,
	lon FLOAT DEFAULT NULL,
	height FLOAT DEFAULT NULL,
	maxspeed FLOAT DEFAULT NULL,
	name       TEXT DEFAULT NULL,
	wmonr      INTEGER DEFAULT NULL,
	nationalnr INTEGER DEFAULT NULL,
	ICAOid     CHAR(4) DEFAULT NULL,
	call_sign  CHAR(7) DEFAULT NULL, 
	stationstr TEXT DEFAULT NULL,
        environmentid  INTEGER DEFAULT NULL,
	static    BOOLEAN DEFAULT FALSE,
        fromtime TIMESTAMP NOT NULL, 
	UNIQUE ( stationid, fromtime )
);

REVOKE ALL ON station FROM public;
GRANT ALL ON station TO kv_admin;
GRANT SELECT ON station TO kv_read;
GRANT SELECT, UPDATE, INSERT, DELETE ON station TO kv_write;




CREATE TABLE checks (
	stationid INTEGER NOT NULL,
	qcx       TEXT NOT NULL,
	medium_qcx TEXT NOT NULL,
	language  INTEGER NOT NULL,
	checkname TEXT DEFAULT NULL,
	checksignature TEXT DEFAULT NULL,
        active   TEXT DEFAULT '* * * * *',   
	fromtime TIMESTAMP NOT NULL,
	UNIQUE ( stationid, qcx, language, fromtime )
);
REVOKE ALL ON checks FROM public;
GRANT ALL ON checks TO kv_admin;
GRANT SELECT ON checks TO kv_read;
GRANT SELECT, UPDATE, INSERT ON checks TO kv_write;



CREATE TABLE station_param (
	stationid INTEGER NOT NULL,
	paramid   INTEGER NOT NULL,
	level	  INTEGER DEFAULT 0,	 
	sensor	  CHAR(1) DEFAULT '0',
	fromday	  INTEGER NOT NULL,
	today	  INTEGER NOT NULL,
	hour      INTEGER DEFAULT -1,
        qcx       TEXT NOT NULL,
	metadata  TEXT DEFAULT NULL,
        desc_metadata TEXT DEFAULT NULL,
        fromtime TIMESTAMP NOT NULL,
	UNIQUE ( stationid, paramid, level, sensor, fromday, today, hour, qcx, fromtime )
);

REVOKE ALL ON station_param FROM public;
GRANT ALL ON station_param TO kv_admin;
GRANT SELECT ON station_param TO kv_read;
GRANT SELECT, UPDATE, INSERT ON station_param TO kv_write;

CREATE TABLE algorithms (
	language  INTEGER DEFAULT NULL,
	checkname TEXT NOT NULL,
	signature TEXT NOT NULL,
	script TEXT NOT NULL,
	UNIQUE ( checkname, language )
);
REVOKE ALL ON algorithms FROM public;
GRANT ALL ON algorithms TO kv_admin;
GRANT SELECT ON algorithms TO kv_read;
GRANT SELECT, UPDATE, INSERT ON algorithms TO kv_write;




CREATE TABLE reference_station (
	stationid INTEGER NOT NULL,
        paramsetid   INTEGER NOT NULL,
	reference TEXT DEFAULT NULL,
	UNIQUE ( stationid, paramsetid ) 
);
REVOKE ALL ON reference_station FROM public;
GRANT ALL ON reference_station TO kv_admin;
GRANT SELECT ON reference_station TO kv_read;
GRANT SELECT, UPDATE, INSERT ON reference_station TO kv_write;



CREATE TABLE timecontrol (
	fromday	  INTEGER NOT NULL,
	today	  INTEGER NOT NULL,
	time      INTEGER NOT NULL,
	priority  INTEGER NOT NULL,
	qcx       TEXT NOT NULL,
	UNIQUE ( fromday, today, time, priority )  
);
REVOKE ALL ON timecontrol FROM public;
GRANT ALL ON timecontrol TO kv_admin;
GRANT SELECT ON timecontrol TO kv_read;
GRANT SELECT, UPDATE, INSERT ON timecontrol TO kv_write;


CREATE TABLE obs_pgm (
	stationid INTEGER NOT NULL,
	paramid	  INTEGER NOT NULL,
	level	  INTEGER NOT NULL,
        nr_sensor INTEGER DEFAULT 1,
	typeid    INTEGER NOT NULL,
	collector BOOLEAN DEFAULT FALSE,
	kl00	BOOLEAN DEFAULT FALSE,
	kl01	BOOLEAN DEFAULT FALSE,
	kl02	BOOLEAN DEFAULT FALSE,
	kl03	BOOLEAN DEFAULT FALSE,
	kl04	BOOLEAN DEFAULT FALSE,
	kl05	BOOLEAN DEFAULT FALSE,
	kl06	BOOLEAN DEFAULT FALSE,	
	kl07	BOOLEAN DEFAULT FALSE,
	kl08	BOOLEAN DEFAULT FALSE,
	kl09	BOOLEAN DEFAULT FALSE,
	kl10	BOOLEAN DEFAULT FALSE,
	kl11	BOOLEAN DEFAULT FALSE,
	kl12	BOOLEAN DEFAULT FALSE,
	kl13	BOOLEAN DEFAULT FALSE,
	kl14	BOOLEAN DEFAULT FALSE,
	kl15	BOOLEAN DEFAULT FALSE,
	kl16	BOOLEAN DEFAULT FALSE,
	kl17	BOOLEAN DEFAULT FALSE,
	kl18	BOOLEAN DEFAULT FALSE,
	kl19	BOOLEAN DEFAULT FALSE,
	kl20	BOOLEAN DEFAULT FALSE,
	kl21	BOOLEAN DEFAULT FALSE,
	kl22	BOOLEAN DEFAULT FALSE,
	kl23	BOOLEAN DEFAULT FALSE,
	mon	BOOLEAN DEFAULT FALSE,
	tue	BOOLEAN DEFAULT FALSE,
	wed	BOOLEAN DEFAULT FALSE,
	thu	BOOLEAN DEFAULT FALSE,
	fri	BOOLEAN DEFAULT FALSE,
	sat	BOOLEAN DEFAULT FALSE,
	sun	BOOLEAN DEFAULT FALSE,
        fromtime TIMESTAMP NOT NULL,
        totime   TIMESTAMP DEFAULT NULL,
	UNIQUE ( stationid, typeid, paramid, level, fromtime, totime )	
);

CREATE INDEX obs_pgm_index_fromtime_totime_typeid_stationid ON obs_pgm (fromtime, totime, typeid, stationid);
REVOKE ALL ON obs_pgm FROM public;
GRANT ALL ON obs_pgm TO kv_admin;
GRANT SELECT ON obs_pgm TO kv_read;
GRANT SELECT, UPDATE, INSERT ON obs_pgm TO kv_write;



CREATE TABLE operator (
	username TEXT NOT NULL,
	userid	 INTEGER NOT NULL,
	UNIQUE (username)
);
REVOKE ALL ON operator FROM public;
GRANT ALL ON operator TO kv_admin;
GRANT SELECT ON operator TO kv_read;
GRANT SELECT, UPDATE, INSERT ON operator TO kv_write;



CREATE TABLE qcx_info (
	medium_qcx  TEXT NOT NULL,
	main_qcx    TEXT NOT NULL,  
 	controlpart INTEGER DEFAULT NULL,
        comment	    TEXT DEFAULT NULL,
	UNIQUE (medium_qcx)
);
REVOKE ALL ON qcx_info FROM public;
GRANT ALL ON qcx_info TO kv_admin;
GRANT SELECT ON qcx_info TO kv_read;
GRANT SELECT, UPDATE, INSERT ON qcx_info TO kv_write;


CREATE TABLE key_val (
	package TEXT NOT NULL,
	key 	TEXT NOT NULL,
	val   	TEXT,
	UNIQUE (package, key)
); 
REVOKE ALL ON key_val FROM public;
GRANT ALL ON key_val TO kv_admin;
GRANT SELECT ON key_val TO kv_read;
GRANT SELECT, UPDATE, INSERT, DELETE ON key_val TO kv_write;



CREATE TABLE stationid_klima (
        stationid INTEGER NOT NULL,
        klima    INTEGER DEFAULT NULL,
        klop     INTEGER DEFAULT NULL,
        UNIQUE ( stationid )
);
REVOKE ALL ON stationid_klima FROM public;
GRANT ALL ON stationid_klima TO kv_admin;
GRANT SELECT ON stationid_klima TO kv_read;
GRANT SELECT, UPDATE, INSERT ON stationid_klima TO kv_write;



CREATE TABLE param_feltfil (
	paramid INTEGER NOT NULL,
	level   INTEGER DEFAULT 0,	
	feltfil_code    INTEGER NOT NULL,
	feltfil_vertical_coordinate INTEGER DEFAULT 0,
	feltfil_level1  INTEGER DEFAULT NULL,
	feltfil_level2  INTEGER DEFAULT NULL,
	UNIQUE ( paramid, level, feltfil_vertical_coordinate )
);
REVOKE ALL ON param_feltfil FROM public;
GRANT ALL ON param_feltfil TO kv_admin;
GRANT SELECT ON param_feltfil TO kv_read;
GRANT SELECT, UPDATE, INSERT ON param_feltfil TO kv_write;



CREATE TABLE workque (
       stationid     INTEGER   NOT NULL,
       obstime       TIMESTAMP NOT NULL,
       typeid        INTEGER   NOT NULL,
       tbtime        TIMESTAMP NOT NULL,
       priority      INTEGER   NOT NULL,
       process_start TIMESTAMP ,
       qa_start      TIMESTAMP ,
       qa_stop       TIMESTAMP ,
       service_start TIMESTAMP ,
       service_stop  TIMESTAMP ,
       UNIQUE(stationid, obstime, typeid)
);

CREATE INDEX workque_index_priority_stationid_typeid_obstime ON
workque (priority, stationid, typeid, obstime);
REVOKE ALL ON workque FROM public;
GRANT ALL ON workque TO kv_admin;
GRANT SELECT ON workque TO kv_read;
GRANT SELECT, UPDATE, INSERT, DELETE ON workque TO kv_write;



CREATE TABLE workstatistik  (
       stationid     INTEGER   NOT NULL,
       obstime       TIMESTAMP NOT NULL,
       typeid        INTEGER   NOT NULL,
       tbtime        TIMESTAMP NOT NULL,
       priority      INTEGER   NOT NULL,
       process_start TIMESTAMP ,
       qa_start      TIMESTAMP ,
       qa_stop       TIMESTAMP ,
       service_start TIMESTAMP ,
       service_stop  TIMESTAMP ,
       UNIQUE(stationid, obstime, typeid)
);
REVOKE ALL ON workstatistik FROM public;
GRANT ALL ON workstatistik TO kv_admin;
GRANT SELECT ON workstatistik TO kv_read;
GRANT SELECT, UPDATE, INSERT, DELETE ON workstatistik TO kv_write;


CREATE TABLE ps_subscribers  (
       name     TEXT NOT NULL,
       subscriberid TEXT NOT NULL,
       comment  TEXT NOT NULL,
       delete_after_hours INTEGER NOT NULL,
       sior     TEXT NOT NULL,
       created  TIMESTAMP NOT NULL,
       UNIQUE(name)
);

REVOKE ALL ON ps_subscribers FROM public;
GRANT ALL ON ps_subscribers TO kv_admin;
GRANT SELECT ON ps_subscribers TO kv_read;
GRANT SELECT, UPDATE, INSERT, DELETE ON ps_subscribers TO kv_write;
