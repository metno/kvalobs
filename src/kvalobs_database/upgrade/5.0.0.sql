BEGIN;

CREATE TABLE observations (
    observationid BIGSERIAL UNIQUE,
    stationid   INTEGER NOT NULL,
    typeid      INTEGER NOT NULL,
    obstime     TIMESTAMP NOT NULL,
    tbtime      TIMESTAMP NOT NULL,
    UNIQUE (stationid, typeid, obstime)
);

CREATE INDEX observations_obstime_index ON observations(obstime);
CREATE INDEX observations_tbtime_index ON observations(tbtime);

REVOKE ALL ON observations FROM public;
GRANT ALL ON observations TO kv_admin;
GRANT SELECT ON observations TO kv_read;
GRANT SELECT, UPDATE, INSERT, DELETE ON observations TO kv_write;
GRANT USAGE, SELECT ON SEQUENCE observations_observationid_seq TO kv_write,kv_admin;

CREATE TABLE obsdata (
    observationid BIGINT REFERENCES observations(observationid) ON DELETE CASCADE,
    obs_offset  INTERVAL NOT NULL DEFAULT '0h',
    original    FLOAT NOT NULL,
    paramid     INTEGER NOT NULL,
    sensor      CHAR(1) DEFAULT '0',
    level       INTEGER DEFAULT 0,
    corrected   FLOAT NOT NULL,
    controlinfo CHAR(16) DEFAULT '0000000000000000',
    useinfo     CHAR(16) DEFAULT '0000000000000000',
    cfailed     TEXT DEFAULT NULL,          
    UNIQUE ( observationid, paramid, level, sensor, obs_offset ) 
);

REVOKE ALL ON obsdata FROM public;
GRANT ALL ON obsdata TO kv_admin;
GRANT SELECT ON obsdata TO kv_read;
GRANT SELECT, UPDATE, INSERT, DELETE ON obsdata TO kv_write;


CREATE TABLE obstextdata (
    observationid BIGINT REFERENCES observations(observationid) ON DELETE CASCADE,
    obs_offset  INTERVAL NOT NULL DEFAULT '0h',
    original    TEXT NOT NULL,
    paramid     INTEGER NOT NULL,
    UNIQUE ( observationid, paramid, obs_offset ) 
);

REVOKE ALL ON obstextdata FROM public;
GRANT ALL ON obstextdata TO kv_admin;
GRANT SELECT ON obstextdata TO kv_read;
GRANT SELECT, UPDATE, INSERT, DELETE ON obstextdata TO kv_write;

CREATE TABLE priority (
	stationid INTEGER NOT NULL,
	typeid    INTEGER NOT NULL,
    priority  INTEGER NOT NULL,
    hour      INTEGER DEFAULT 6,
    pri_after_hour INTEGER DEFAULT 10,
	UNIQUE ( stationid, typeid )
);

REVOKE ALL ON priority FROM public;
GRANT ALL ON priority TO kv_admin;
GRANT SELECT ON priority TO kv_read;
GRANT SELECT, UPDATE, INSERT ON priority TO kv_write;

INSERT INTO observations (stationid, typeid, obstime, tbtime) (
    SELECT sub.stationid, sub.typeid, sub.obstime, max(sub.tbtime) FROM (
        SELECT stationid, typeid, obstime, max(tbtime) as tbtime FROM data group by 1,2,3 
        UNION 
        SELECT stationid, typeid, obstime, max(tbtime) as tbtime FROM text_data group by 1,2,3
    ) AS sub group by 1,2,3);

INSERT INTO obsdata (observationid, original, paramid, sensor, level, corrected, controlinfo, useinfo, cfailed)
(
    SELECT 
        o.observationid, 
        d.original, 
        d.paramid, 
        d.sensor, 
        d.level, 
        d.corrected, 
        d.controlinfo, 
        d.useinfo, 
        d.cfailed
    FROM 
        observations o, 
        data d
    WHERE
        o.stationid = d.stationid AND
        o.typeid = d.typeid AND
        o.obstime = d.obstime
);

INSERT INTO obstextdata (observationid, original, paramid)
(
    SELECT 
        o.observationid, 
        d.original, 
        d.paramid
    FROM 
        observations o, 
        text_data d
    WHERE
        o.stationid = d.stationid AND
        o.typeid = d.typeid AND
        o.obstime = d.obstime
);



drop view data_view;
drop view text_data_view;
drop view pdata;

DROP TABLE data;

CREATE VIEW data AS (
    SELECT 
        o.stationid,
        o.obstime + d.obs_offset as obstime,
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

DROP TABLE text_data;

CREATE VIEW text_data AS (
    SELECT 
        o.stationid,
        o.obstime + d.obs_offset as obstime,
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

ALTER TABLE workque ADD COLUMN observationid bigint REFERENCES observations(observationid) ON DELETE CASCADE;
UPDATE workque q SET observationid=(SELECT observationid FROM observations o WHERE o.stationid=q.stationid AND o.typeid=q.typeid AND o.obstime=q.obstime);
DELETE FROM workque WHERE observationid IS NULL;
ALTER TABLE workque ALTER COLUMN observationid SET NOT NULL;
ALTER TABLE workque DROP COLUMN stationid;
ALTER TABLE workque DROP COLUMN obstime;
ALTER TABLE workque DROP COLUMN typeid;
ALTER TABLE workque DROP COLUMN tbtime;
CREATE INDEX workque_priority_obsid ON workque (priority, observationid);
CREATE INDEX workque_qa_start_qa_stop_idx ON workque (qa_start, qa_stop);


ALTER TABLE workstatistik DROP CONSTRAINT workstatistik_stationid_obstime_typeid_key;
ALTER TABLE workstatistik ADD COLUMN observationid bigint;
UPDATE workstatistik s SET observationid=(SELECT observationid FROM observations o WHERE o.stationid=s.stationid AND o.typeid=s.typeid AND o.obstime=s.obstime);
CREATE INDEX ON workstatistik (stationid, obstime, typeid);
CREATE INDEX ON workstatistik (observationid);

--
--
-- TODO: Handle deletion from observations table
--
--

--
-- Trigger function for propagating changes to the data table into data_history
--
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
		(obs.stationid,obs.obstime+NEW.obs_offset,NEW.original,NEW.paramid,obs.tbtime,obs.typeid,NEW.sensor,NEW.level,NEW.corrected,NEW.controlinfo,NEW.useinfo,NEW.cfailed);
	RETURN NULL;
END;
$BODY$
LANGUAGE 'plpgsql';
CREATE TRIGGER backup_data AFTER INSERT OR UPDATE ON obsdata FOR EACH ROW EXECUTE PROCEDURE backup_old_data();

--
-- Trigger function for propagating deletes in the data table into data_history
-- Deleted rows are marked in data_history with NULL values for original, 
-- corrected, controlinfo, useinfo and cfailed.

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
		(obs.stationid,obs.obstime+OLD.obs_offset,NULL,OLD.paramid,obs.tbtime,obs.typeid,OLD.sensor,OLD.level,NULL,NULL,NULL,NULL);
	RETURN NULL;
END;
$BODY$
LANGUAGE 'plpgsql';
CREATE TRIGGER backup_data_delete AFTER DELETE ON obsdata FOR EACH ROW EXECUTE PROCEDURE backup_old_data_delete();



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
		(obs.stationid,obs.obstime+NEW.obs_offset,NEW.original,NEW.paramid,obs.tbtime,obs.typeid);
	RETURN NULL;
END;
$BODY$
LANGUAGE 'plpgsql';
CREATE TRIGGER backup_text_data AFTER INSERT OR UPDATE ON obstextdata FOR EACH ROW EXECUTE PROCEDURE backup_old_text_data();

--
-- Trigger function for propagating deletes in the text_data table into text_data_history
-- Deleted rows are marked in text_data_history with NULL values for original.
--
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
		(obs.stationid,obs.obstime+OLD.obs_offset,NULL,OLD.paramid,obs.tbtime,obs.typeid);
	RETURN NULL;
END;
$BODY$
LANGUAGE 'plpgsql';
CREATE TRIGGER backup_text_data_delete AFTER DELETE ON obstextdata FOR EACH ROW EXECUTE PROCEDURE backup_old_text_data_delete();

CREATE OR REPLACE FUNCTION mkdata(
    stationid_ integer,
    obstime_ timestamp,
    original float,
    paramid integer,
    tbtime timestamp,
    typeid_ integer,
    sensor char(1),
    level integer,
    corrected float,
    controlinfo char(16),
    useinfo char(16),
    cfailed text
) RETURNS bigint AS
$BODY$
DECLARE 
    obs bigint;
BEGIN
    SELECT observationid into obs FROM observations o WHERE o.stationid=stationid_ AND o.typeid=typeid_ AND o.obstime=obstime_;
    IF NOT FOUND THEN
        INSERT INTO observations (stationid, typeid, obstime, tbtime) VALUES (stationid_, typeid_, obstime_, tbtime) RETURNING observationid INTO obs;
    END IF;
    INSERT INTO obsdata VALUES (
        obs,
        '0h',
        original,
        paramid,
        sensor,
        level,
        corrected,
        controlinfo,
        useinfo,
        cfailed
    );
    RETURN obs;
END;
$BODY$
LANGUAGE 'plpgsql';

CREATE OR REPLACE RULE data_insert AS ON INSERT TO data DO INSTEAD (
    SELECT mkdata(NEW.stationid, NEW.obstime, NEW.original, NEW.paramid, NEW.tbtime, NEW.typeid, NEW.sensor, NEW.level, NEW.corrected, NEW.controlinfo, NEW.useinfo, NEW.cfailed);
);

CREATE OR REPLACE RULE data_update AS ON UPDATE TO data DO INSTEAD (
    UPDATE obsdata SET 
        original=NEW.original,
        paramid=NEW.paramid,
        sensor=NEW.sensor,
        level=NEW.level,
        corrected=NEW.corrected,
        controlinfo=NEW.controlinfo,
        useinfo=NEW.useinfo,
        cfailed=NEW.cfailed
    where 
        observationid=(
            select observationid from observations where
            stationid=NEW.stationid AND typeid=NEW.typeid AND obstime=NEW.obstime) AND
        paramid=OLD.paramid AND
        sensor=OLD.sensor AND
        level=OLD.level
);

CREATE OR REPLACE RULE data_delete AS ON DELETE TO data DO INSTEAD (
    DELETE FROM obsdata WHERE 
        observationid=(
            select observationid from observations where
            stationid=OLD.stationid AND typeid=OLD.typeid AND obstime=OLD.obstime) AND
        paramid=OLD.paramid AND
        sensor=OLD.sensor AND
        level=OLD.level
);



CREATE OR REPLACE FUNCTION mktextdata(
    stationid_ integer,
    obstime_ timestamp,
    original text,
    paramid integer,
    tbtime timestamp,
    typeid_ integer
) RETURNS bigint AS
$BODY$
DECLARE 
    obs bigint;
BEGIN
    SELECT observationid into obs FROM observations o WHERE o.stationid=stationid_ AND o.typeid=typeid_ AND o.obstime=obstime_;
    IF NOT FOUND THEN
        INSERT INTO observations (stationid, typeid, obstime, tbtime) VALUES (stationid_, typeid_, obstime_, tbtime) RETURNING observationid INTO obs;
    END IF;
    INSERT INTO obstextdata VALUES (
        obs,
        '0h',
        original,
        paramid
    );
    RETURN obs;
END;
$BODY$
LANGUAGE 'plpgsql';

CREATE OR REPLACE RULE text_data_insert AS ON INSERT TO text_data DO INSTEAD (
    SELECT mktextdata(NEW.stationid, NEW.obstime, NEW.original, NEW.paramid, NEW.tbtime, NEW.typeid);
);

CREATE OR REPLACE RULE text_data_update AS ON UPDATE TO text_data DO INSTEAD (
    UPDATE obstextdata SET 
        original=NEW.original,
        paramid=NEW.paramid
    where 
        observationid=(
            select observationid from observations where
            stationid=NEW.stationid AND typeid=NEW.typeid AND obstime=NEW.obstime) AND
        paramid=OLD.paramid
);

CREATE OR REPLACE RULE text_data_delete AS ON DELETE TO text_data DO INSTEAD (
    DELETE FROM obstextdata WHERE 
        observationid=(
            select observationid from observations where
            stationid=OLD.stationid AND typeid=OLD.typeid AND obstime=OLD.obstime) AND
        paramid=OLD.paramid
);

CREATE OR REPLACE FUNCTION 
kvalobs_database_version()
RETURNS text AS
$BODY$
BEGIN
	RETURN '5.0.0';
END;
$BODY$
LANGUAGE plpgsql IMMUTABLE;

END;
