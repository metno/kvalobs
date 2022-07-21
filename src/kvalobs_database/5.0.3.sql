BEGIN;


ALTER TABLE workque ADD COLUMN tbtime timestamp DEFAULT current_timestamp(0);
ALTER TABLE workque ADD COLUMN qa_id smallint DEFAULT NULL;

ALTER TABLE workstatistik ADD COLUMN qa_id smallint DEFAULT NULL;
CREATE INDEX ON workque (tbtime);

CREATE INDEX ON workstatistik (tbtime);

CREATE VIEW workque_v  AS
select
	o.stationid,
	o.typeid,
  o.obstime,
  o.tbtime,
  q.tbtime AS que_tbtime,
  q.priority,
  q.process_start,
  q.qa_start,
  q.qa_stop,
  q.qa_id,
  q.observationid
from workque q, observations o 
where q.observationid=o.observationid;

REVOKE ALL ON workque_v FROM public;
GRANT ALL ON workque_v TO kv_admin;
GRANT SELECT ON workque_v TO kv_read;
GRANT SELECT ON workque_v TO kv_write;

DROP VIEW workque_count;
DROP VIEW workque_v;


CREATE VIEW workque_v  AS
select
	o.stationid,
	o.typeid,
    o.obstime,
    o.tbtime,
    q.tbtime AS que_tbtime,
    q.priority,
    q.process_start,
    q.qa_start,
    q.qa_stop,
    q.qa_id,
    q.observationid
from workque q, observations o 
where q.observationid=o.observationid;

REVOKE ALL ON workque_v FROM public;
GRANT ALL ON workque_v TO kv_admin;
GRANT SELECT ON workque_v TO kv_read;
GRANT SELECT ON workque_v TO kv_write;

CREATE VIEW workque_count  AS
select to_char(tbtime,'yyyy-mm-dd HH24:MI') as tbtime, stationid, typeid, count(*) 
from workque_v group by to_char(tbtime,'yyyy-mm-dd HH24:MI'),stationid, typeid;

REVOKE ALL ON workque_count FROM public;
GRANT ALL ON workque_count TO kv_admin;
GRANT SELECT ON workque_count TO kv_read;
GRANT SELECT ON workque_count TO kv_write;


CREATE VIEW workstatistik_count  AS
select to_char(tbtime,'yyyy-mm-dd HH24:MI') as tbtime, stationid, typeid, count(*) 
from workstatistik group by to_char(tbtime,'yyyy-mm-dd HH24:MI'),stationid, typeid;

REVOKE ALL ON workstatistik_count FROM public;
GRANT ALL ON workstatistik_count TO kv_admin;
GRANT SELECT ON workstatistik_count TO kv_read;
GRANT SELECT ON workstatistik_count TO kv_write;

CREATE OR REPLACE FUNCTION 
kvalobs_database_version()
RETURNS text AS
$BODY$
BEGIN
	RETURN '5.0.3';
END;
$BODY$
LANGUAGE plpgsql IMMUTABLE;

END;
