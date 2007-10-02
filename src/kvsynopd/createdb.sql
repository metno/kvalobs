CREATE TABLE data (stationid integer, obstime timestamp, original text, paramid integer, typeid integer, sensor character(1), level integer, UNIQUE(stationid, obstime, paramid, typeid, level, sensor));

CREATE TABLE synop(wmono integer, obstime timestamp, createtime timestamp, crc integer, ccx integer, wmomsg text, UNIQUE(wmono, obstime));

CREATE TABLE waiting(wmono integer, obstime timestamp, delaytime timestamp, UNIQUE(wmono, obstime));

CREATE TABLE keyval(key text, val text, UNIQUE(key));

