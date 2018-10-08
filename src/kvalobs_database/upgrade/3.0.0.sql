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

END;
