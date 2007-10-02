CREATE TABLE data (
  stationid   INTEGER NOT NULL,
  obstime     TIMESTAMP NOT NULL,
  original    FLOAT NOT NULL,
  paramid     INTEGER NOT NULL,
  tbtime      TIMESTAMP NOT NULL,
  typeid      INTEGER NOT NULL,
  sensor      CHAR(1) DEFAULT '0',
  level     INTEGER DEFAULT 0,
  corrected   FLOAT NOT NULL,
  controlinfo CHAR(16) DEFAULT '0000000000000000',
  useinfo     CHAR(16) DEFAULT '0000000000000000',
  cfailed     TEXT DEFAULT NULL,      
  UNIQUE ( stationid, obstime, paramid, level, sensor, typeid ) 
);

CREATE INDEX data_obstime_index ON data (obstime);
CREATE INDEX data_tbtime_index ON data (tbtime);

CREATE TABLE text_data (
  stationid   INTEGER NOT NULL,
  obstime     TIMESTAMP NOT NULL,
  original    TEXT NOT NULL,
  paramid     INTEGER NOT NULL,
  tbtime      TIMESTAMP NOT NULL,
  typeid      INTEGER NOT NULL,
  UNIQUE ( stationid, obstime, paramid, typeid ) 
);


-- CREATE VIEW data_view AS
-- SELECT data.stationid, data.obstime, CAST(data.original AS TEXT), data.paramid, data.tbtime, data.typeid
-- FROM data, text_data
-- WHERE data.stationid=text_data.stationid and data.obstime=text_data.obstime
-- UNION ALL
-- SELECT text_data.stationid, text_data.obstime, text_data.original, text_data.paramid, text_data.tbtime, text_data.typeid
-- FROM data, text_data
-- WHERE data.stationid=text_data.stationid and data.obstime=text_data.obstime
-- UNION  ALL
-- SELECT data.stationid, data.obstime, CAST(data.original AS TEXT), data.paramid, data.tbtime, data.typeid
-- FROM data
-- WHERE (data.stationid NOT IN ( SELECT text_data.stationid FROM text_data )) OR 
--       (data.obstime NOT IN ( SELECT text_data.obstime FROM text_data ));


-- CREATE VIEW text_data_view AS
-- SELECT text_data.stationid, text_data.obstime, text_data.original, text_data.paramid, text_data.tbtime, text_data.typeid
-- FROM text_data
-- WHERE (text_data.stationid NOT IN ( SELECT data.stationid FROM data )) OR 
--       (text_data.obstime NOT IN ( SELECT data.obstime FROM data ));



CREATE TABLE rejectdecode (
  message TEXT NOT NULL,
  tbtime TIMESTAMP NOT NULL,
  decoder TEXT NOT NULL,
  comment TEXT DEFAULT NULL,
  UNIQUE ( tbtime, message, decoder )
);


CREATE TABLE model_data (
  stationid INTEGER NOT NULL,
  obstime   TIMESTAMP NOT NULL,
  paramid   INTEGER NOT NULL,
  level     INTEGER NOT NULL,
        modelid   INTEGER NOT NULL,
  original  FLOAT DEFAULT NULL,  
  UNIQUE ( stationid, obstime, paramid, level, modelid )
);


CREATE TABLE model (
  modelid  INTEGER NOT NULL,
  name    TEXT DEFAULT NULL,
  comment TEXT DEFAULT NULL,
  UNIQUE ( modelid )
);


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


CREATE TABLE generated_types (
  stationid INTEGER NOT NULL,
  typeid  INTEGER NOT NULL,
  UNIQUE ( stationid, typeid )
);


CREATE TABLE param (
  paramid INTEGER NOT NULL,
  name    TEXT NOT NULL,
  description TEXT DEFAULT NULL,
  unit    TEXT DEFAULT NULL,
  level_scale INTEGER DEFAULT 0,
  comment TEXT DEFAULT NULL,
  UNIQUE ( paramid )
);


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


CREATE TABLE station_param (
  stationid INTEGER NOT NULL,
  paramid   INTEGER NOT NULL,
  level   INTEGER DEFAULT 0,   
  sensor    CHAR(1) DEFAULT '0',
  fromday   INTEGER NOT NULL,
  today   INTEGER NOT NULL,
  hour      INTEGER DEFAULT -1,
        qcx       TEXT NOT NULL,
  metadata  TEXT DEFAULT NULL,
        desc_metadata TEXT DEFAULT NULL,
        fromtime TIMESTAMP NOT NULL,
  UNIQUE ( stationid, paramid, level, sensor, fromday, today, hour, qcx, fromtime )
);


CREATE TABLE algorithms (
  language  INTEGER DEFAULT NULL,
  checkname TEXT NOT NULL,
  signature TEXT NOT NULL,
  script TEXT NOT NULL,
  UNIQUE ( checkname, language )
);


CREATE TABLE reference_station (
  stationid INTEGER NOT NULL,
        paramsetid   INTEGER NOT NULL,
  reference TEXT DEFAULT NULL,
  UNIQUE ( stationid, paramsetid ) 
);


CREATE TABLE timecontrol (
  fromday   INTEGER NOT NULL,
  today   INTEGER NOT NULL,
  time      INTEGER NOT NULL,
  priority  INTEGER NOT NULL,
  qcx       TEXT NOT NULL,
  UNIQUE ( fromday, today, time, priority )  
);


CREATE TABLE obs_pgm (
  stationid INTEGER NOT NULL,
  paramid   INTEGER NOT NULL,
  level   INTEGER NOT NULL,
        nr_sensor INTEGER DEFAULT 1,
  typeid    INTEGER NOT NULL,
  collector BOOLEAN DEFAULT FALSE,
  kl00  BOOLEAN DEFAULT FALSE,
  kl01  BOOLEAN DEFAULT FALSE,
  kl02  BOOLEAN DEFAULT FALSE,
  kl03  BOOLEAN DEFAULT FALSE,
  kl04  BOOLEAN DEFAULT FALSE,
  kl05  BOOLEAN DEFAULT FALSE,
  kl06  BOOLEAN DEFAULT FALSE,  
  kl07  BOOLEAN DEFAULT FALSE,
  kl08  BOOLEAN DEFAULT FALSE,
  kl09  BOOLEAN DEFAULT FALSE,
  kl10  BOOLEAN DEFAULT FALSE,
  kl11  BOOLEAN DEFAULT FALSE,
  kl12  BOOLEAN DEFAULT FALSE,
  kl13  BOOLEAN DEFAULT FALSE,
  kl14  BOOLEAN DEFAULT FALSE,
  kl15  BOOLEAN DEFAULT FALSE,
  kl16  BOOLEAN DEFAULT FALSE,
  kl17  BOOLEAN DEFAULT FALSE,
  kl18  BOOLEAN DEFAULT FALSE,
  kl19  BOOLEAN DEFAULT FALSE,
  kl20  BOOLEAN DEFAULT FALSE,
  kl21  BOOLEAN DEFAULT FALSE,
  kl22  BOOLEAN DEFAULT FALSE,
  kl23  BOOLEAN DEFAULT FALSE,
  mon BOOLEAN DEFAULT FALSE,
  tue BOOLEAN DEFAULT FALSE,
  wed BOOLEAN DEFAULT FALSE,
  thu BOOLEAN DEFAULT FALSE,
  fri BOOLEAN DEFAULT FALSE,
  sat BOOLEAN DEFAULT FALSE,
  sun BOOLEAN DEFAULT FALSE,
        fromtime TIMESTAMP NOT NULL,
  UNIQUE ( stationid, paramid, level, fromtime )  
);


CREATE TABLE operator (
  username TEXT NOT NULL,
  userid   INTEGER NOT NULL,
  UNIQUE (username)
);


CREATE TABLE qcx_info (
  medium_qcx  TEXT NOT NULL,
  main_qcx    TEXT NOT NULL,  
  controlpart INTEGER DEFAULT NULL,
        comment     TEXT DEFAULT NULL,
  UNIQUE (medium_qcx)
);


CREATE TABLE key_val (
  package TEXT NOT NULL,
  key   TEXT NOT NULL,
  val     TEXT,
  UNIQUE (package, key)
); 


CREATE TABLE stationid_klima (
        stationid INTEGER NOT NULL,
        klima    INTEGER DEFAULT NULL,
        klop     INTEGER DEFAULT NULL,
        UNIQUE ( stationid )
);


CREATE TABLE param_feltfil (
  paramid INTEGER NOT NULL,
  level   INTEGER DEFAULT 0,  
  feltfil_code    INTEGER NOT NULL,
  feltfil_vertical_coordinate INTEGER DEFAULT 0,
  feltfil_level1  INTEGER DEFAULT NULL,
  feltfil_level2  INTEGER DEFAULT NULL,
  UNIQUE ( paramid, level, feltfil_vertical_coordinate )
);



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




















