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

BEGIN;

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
	modificationtime timestamp NOT NULL DEFAULT timezone('UTC', now()),
	UNIQUE ( version, stationid, obstime, paramid, level, sensor, typeid ) 
);

CREATE INDEX data_history_main_index ON data_history (stationid, obstime, paramid, level, sensor, typeid);
CREATE INDEX data_history_obstime_index ON data_history (obstime);
CREATE INDEX data_history_tbtime_index ON data_history (tbtime);
CREATE INDEX data_history_modificationtime_index ON data_history (modificationtime);

REVOKE ALL ON data_history FROM public;
GRANT ALL ON data_history TO kv_admin;
GRANT SELECT ON data_history TO kv_read;
GRANT SELECT, UPDATE, INSERT, DELETE ON data_history TO kv_write;
GRANT USAGE ON SEQUENCE data_history_version_seq TO kv_write;


CREATE OR REPLACE FUNCTION 
kvalobs_database_version()
RETURNS text AS
$BODY$
BEGIN
	RETURN '2.1.4';
END;
$BODY$
LANGUAGE plpgsql IMMUTABLE;

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
		(NEW.stationid,NEW.obstime,NEW.original,NEW.paramid,NEW.tbtime,NEW.typeid,NEW.sensor,NEW.level,NEW.corrected,NEW.controlinfo,NEW.useinfo,NEW.cfailed);
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
		(OLD.stationid,OLD.obstime,NULL,OLD.paramid,OLD.tbtime,OLD.typeid,OLD.sensor,OLD.level,NULL,NULL,NULL,NULL);
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
	modificationtime timestamp NOT NULL DEFAULT timezone('UTC', now()),
	UNIQUE ( version, stationid, obstime, paramid, typeid ) 
);
REVOKE ALL ON text_data_history FROM public;
GRANT ALL ON text_data_history TO kv_admin;
GRANT SELECT ON text_data_history TO kv_read;
GRANT SELECT, UPDATE, INSERT, DELETE ON text_data_history TO kv_write;
GRANT USAGE ON SEQUENCE text_data_history_version_seq TO kv_write;


CREATE INDEX text_data_history_main_index ON text_data_history (stationid, obstime, paramid, typeid);
CREATE INDEX text_data_history_modificationtime_index ON text_data_history (modificationtime);

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
		(NEW.stationid,NEW.obstime,NEW.original,NEW.paramid,NEW.tbtime,NEW.typeid);
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
		(OLD.stationid,OLD.obstime,NULL,OLD.paramid,OLD.tbtime,OLD.typeid);
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
	fixed boolean NOT NULL DEFAULT false,
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
	scalar BOOLEAN DEFAULT TRUE,
	UNIQUE ( paramid )
);

REVOKE ALL ON param FROM public;
GRANT ALL ON param TO kv_admin;
GRANT SELECT ON param TO kv_read;
GRANT SELECT, UPDATE, INSERT ON param TO kv_write;

--
-- PDATA is a view to the data table that shows parameter names instead of parameter ids.
--
CREATE VIEW pdata AS
    SELECT data.stationid, data.obstime, data.tbtime, data.typeid, (SELECT param.name FROM param WHERE (param.paramid = data.paramid))
    AS paramid, data.original, data.corrected, data.sensor, data.level, data.controlinfo, data.useinfo, data.cfailed FROM data;
REVOKE ALL ON pdata FROM public;
GRANT ALL ON pdata TO kv_admin;
GRANT SELECT ON pdata TO kv_read;


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



CREATE TABLE obs_pgm (
	stationid INTEGER NOT NULL,
	paramid	  INTEGER NOT NULL,
	level	  INTEGER NOT NULL,
        nr_sensor INTEGER DEFAULT 1,
	typeid    INTEGER NOT NULL,
	priority_message  BOOLEAN DEFAULT TRUE,
	collector BOOLEAN DEFAULT FALSE,
	kl00 BOOLEAN DEFAULT FALSE,
	kl01 BOOLEAN DEFAULT FALSE,
	kl02 BOOLEAN DEFAULT FALSE,
	kl03 BOOLEAN DEFAULT FALSE,
	kl04 BOOLEAN DEFAULT FALSE,
	kl05 BOOLEAN DEFAULT FALSE,
	kl06 BOOLEAN DEFAULT FALSE,	
	kl07 BOOLEAN DEFAULT FALSE,
	kl08 BOOLEAN DEFAULT FALSE,
	kl09 BOOLEAN DEFAULT FALSE,
	kl10 BOOLEAN DEFAULT FALSE,
	kl11 BOOLEAN DEFAULT FALSE,
	kl12 BOOLEAN DEFAULT FALSE,
	kl13 BOOLEAN DEFAULT FALSE,
	kl14 BOOLEAN DEFAULT FALSE,
	kl15 BOOLEAN DEFAULT FALSE,
	kl16 BOOLEAN DEFAULT FALSE,
	kl17 BOOLEAN DEFAULT FALSE,
	kl18 BOOLEAN DEFAULT FALSE,
	kl19 BOOLEAN DEFAULT FALSE,
	kl20 BOOLEAN DEFAULT FALSE,
	kl21 BOOLEAN DEFAULT FALSE,
	kl22 BOOLEAN DEFAULT FALSE,
	kl23 BOOLEAN DEFAULT FALSE,
	mon  BOOLEAN DEFAULT FALSE,
	tue  BOOLEAN DEFAULT FALSE,
	wed  BOOLEAN DEFAULT FALSE,
	thu  BOOLEAN DEFAULT FALSE,
	fri  BOOLEAN DEFAULT FALSE,
	sat  BOOLEAN DEFAULT FALSE,
	sun  BOOLEAN DEFAULT FALSE,
        fromtime TIMESTAMP NOT NULL,
        totime   TIMESTAMP DEFAULT NULL,
	UNIQUE ( stationid, typeid, paramid, level, fromtime )	
);

CREATE INDEX obs_pgm_index_fromtime_totime_typeid_stationid ON obs_pgm (fromtime, totime, typeid, stationid);
REVOKE ALL ON obs_pgm FROM public;
GRANT ALL ON obs_pgm TO kv_admin;
GRANT SELECT ON obs_pgm TO kv_read;
GRANT SELECT, UPDATE, INSERT ON obs_pgm TO kv_write;


CREATE TABLE default_missing (
	paramid INT UNIQUE,
	value FLOAT NOT NULL,
	controlinfo CHAR(16) NOT NULL
);
INSERT INTO default_missing VALUES (NULL, -32767, '0000003000000000');
INSERT INTO default_missing VALUES (105, 0, '0000000000000000');
REVOKE ALL ON default_missing FROM public;
GRANT ALL ON default_missing TO kv_admin;
GRANT SELECT ON default_missing TO kv_read;
GRANT SELECT, UPDATE, INSERT ON default_missing TO kv_write;

CREATE VIEW default_missing_values AS
select
	p.paramid,
	m.value
from
	param p,
	default_missing m
where
	p.paramid=m.paramid or (
		m.paramid is null and
		p.paramid not in (
			select paramid from default_missing where paramid=p.paramid
		)
	);
REVOKE ALL ON default_missing_values FROM public;
GRANT ALL ON default_missing_values TO kv_admin;
GRANT SELECT ON default_missing_values TO kv_read;
GRANT SELECT ON obs_pgm TO kv_write;

	
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


CREATE TABLE qc2_statistical_reference_values (
       stationid   INTEGER NOT NULL,
       paramid     INTEGER NOT NULL,
       day_of_year INTEGER NOT NULL,
       key         TEXT NOT NULL,
       value       FLOAT NOT NULL);

REVOKE ALL ON qc2_statistical_reference_values FROM public;
GRANT ALL ON qc2_statistical_reference_values TO kv_admin;
GRANT SELECT ON qc2_statistical_reference_values TO kv_read;
GRANT SELECT, UPDATE, INSERT, DELETE ON qc2_statistical_reference_values TO kv_write;

CREATE TABLE qc2_interpolation_best_neighbors (
       stationid        INTEGER NOT NULL,
       neighborid       INTEGER NOT NULL,
       paramid          INTEGER NOT NULL,
       interpolation_id INTEGER NOT NULL,
       fit_offset       FLOAT NOT NULL,
       fit_slope        FLOAT NOT NULL,
       fit_sigma        FLOAT NOT NULL);

REVOKE ALL ON qc2_interpolation_best_neighbors FROM public;
GRANT ALL ON qc2_interpolation_best_neighbors TO kv_admin;
GRANT SELECT ON qc2_interpolation_best_neighbors TO kv_read;
GRANT SELECT, UPDATE, INSERT, DELETE ON qc2_interpolation_best_neighbors TO kv_write;

END;
