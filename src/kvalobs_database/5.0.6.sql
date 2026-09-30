BEGIN;

ALTER TABLE observations ADD COLUMN mtime TIMESTAMP DEFAULT now();

ALTER TABLE obsdata
    ALTER COLUMN sensor TYPE SMALLINT
    USING trim(sensor)::smallint;

ALTER TABLE data_history
    ALTER COLUMN sensor TYPE SMALLINT
    USING trim(sensor)::smallint;

CREATE FUNCTION mkdata(
    stationid_ integer,
    obstime_ timestamp,
    original float,
    paramid integer,
    tbtime timestamp,
    typeid_ integer,
    sensor smallint,
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
    SELECT observationid INTO obs
      FROM observations o
     WHERE o.stationid=stationid_
       AND o.typeid=typeid_
       AND o.obstime=obstime_;
    IF NOT FOUND THEN
        INSERT INTO observations (stationid, typeid, obstime, tbtime)
        VALUES (stationid_, typeid_, obstime_, tbtime)
        RETURNING observationid INTO obs;
    END IF;
    INSERT INTO obsdata VALUES (
        obs, original, paramid, sensor, level, corrected,
        controlinfo, useinfo, cfailed
    );
    RETURN obs;
END;
$BODY$
LANGUAGE 'plpgsql';

DROP RULE data_insert ON data;
CREATE OR REPLACE RULE data_insert AS ON INSERT TO data DO INSTEAD (
    SELECT mkdata(NEW.stationid, NEW.obstime, NEW.original, NEW.paramid,
                   NEW.tbtime, NEW.typeid, NEW.sensor, NEW.level,
                   NEW.corrected, NEW.controlinfo, NEW.useinfo, NEW.cfailed)
);

CREATE OR REPLACE FUNCTION
kvalobs_database_version()
RETURNS text AS
$BODY$
BEGIN
    RETURN '5.0.6';
END;
$BODY$
LANGUAGE plpgsql IMMUTABLE;

CREATE OR REPLACE FUNCTION update_observations_mtime()
RETURNS TRIGGER AS $$
BEGIN
    UPDATE observations
    SET mtime = now()
    WHERE observations.observationid = NEW.observationid;
    
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

END