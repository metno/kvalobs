BEGIN;

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

COMMIT;
