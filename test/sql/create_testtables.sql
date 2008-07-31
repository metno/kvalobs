CREATE TABLE rcvtest_data (
        stationid   INTEGER NOT NULL,
        obstime     TIMESTAMP NOT NULL,
        original    FLOAT NOT NULL,
        paramid     INTEGER NOT NULL,
        tbtime      TIMESTAMP NOT NULL,
        typeid      INTEGER NOT NULL,
        sensor      CHAR(1) DEFAULT '0',
        level       INTEGER DEFAULT 0,
        corrected   FLOAT NOT NULL,
        controlinfo CHAR(16) DEFAULT '0000000000000000',
        useinfo     CHAR(16) DEFAULT '0000000000000000',
        cfailed     TEXT DEFAULT NULL,
        UNIQUE ( stationid, obstime, paramid, level, sensor, typeid )
);
CREATE INDEX rcvtest_data_obstime_index ON rcvtest_data (obstime);
CREATE INDEX rcvtest_data_tbtime_index ON rcvtest_data (tbtime);

CREATE TABLE rcvtest_text_data (
        stationid   INTEGER NOT NULL,
        obstime     TIMESTAMP NOT NULL,
        original    TEXT NOT NULL,
        paramid     INTEGER NOT NULL,
        tbtime      TIMESTAMP NOT NULL,
        typeid      INTEGER NOT NULL,
        UNIQUE ( stationid, obstime, paramid, typeid )
);

CREATE INDEX rcvtest_text_data_obstime_index ON rcvtest_text_data (obstime);
CREATE INDEX rcvtest_text_data_tbtime_index ON rcvtest_text_data (tbtime);
