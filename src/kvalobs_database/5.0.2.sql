BEGIN;

CREATE UNIQUE INDEX ON obsdata (observationid, paramid, level, sensor);

END;