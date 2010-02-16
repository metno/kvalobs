BEGIN;

CREATE INDEX data_history_main_index ON data_history (stationid, obstime, paramid, level, sensor, typeid);
CREATE INDEX data_history_obstime_index ON data_history (obstime);
CREATE INDEX data_history_tbtime_index ON data_history (tbtime);


COMMIT;