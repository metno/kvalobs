BEGIN;
CREATE OR REPLACE FUNCTION 
kvalobs_database_version()
RETURNS text AS
$BODY$
BEGIN
	RETURN '5.0.4';
END;
$BODY$
LANGUAGE plpgsql IMMUTABLE;

DROP FUNCTION IF EXISTS insert_into_workque ;

--
-- insert_into_workque inserts a row into the workque. 
--
-- If qa_id_default_ is NULL the element is inserted with qa_id NULL.
--
-- If qa_id_default_ is NOT NULL the workque is searched for a qa_id, not null, 
-- for the stationid_, typeid_ and obstime_ in the interval
-- obstime hours_back_ to obstime_. If any is found set qa_id to the same qa_id as 
-- the newest element. If not found set qa_id to qa_id_default_. 
--
-- The new row will get observationid_, priority_ and qa_id set.
-- 
-- Return nothing.
-- 
CREATE OR REPLACE FUNCTION insert_into_workque(
    observationid_ bigint,
    priority_      integer,
    stationid_     integer,
    typeid_        integer,
    obstime_       timestamp,
    hours_back_    integer,
    qa_id_default_ integer
) RETURNS void AS
$BODY$
DECLARE 
    qaid_found smallint;
BEGIN

  IF qa_id_default_ IS NULL THEN 
    INSERT INTO workque (observationid,priority,process_start,qa_start,qa_stop,service_start,service_stop, qa_id) 
      VALUES(observationid_ ,priority_, NULL, NULL, NULL, NULL, NULL, NULL);
    RETURN;
  END IF;

  SELECT 
    q.qa_id into qaid_found
  FROM 
    workque q,
    observations o
  WHERE q.observationid = o.observationid and 
    q.qa_start is null and q.qa_id is not null and
    o.stationid=stationid_ and o.typeid=typeid_ and 
    o.obstime between (obstime_ - make_interval( hours=> hours_back_ )) and obstime_ 
    order by obstime desc limit 1;
    
  IF NOT FOUND THEN
    INSERT INTO workque (observationid,priority,process_start,qa_start,qa_stop,service_start,service_stop, qa_id) 
      VALUES(observationid_ ,priority_, NULL, NULL, NULL, NULL, NULL, qa_id_default_::smallint);
    RETURN; 
  END IF;

  INSERT INTO workque (observationid,priority,process_start,qa_start,qa_stop,service_start,service_stop, qa_id) 
    VALUES(observationid_ , pri_,NULL,NULL,NULL,NULL,NULL, qaid_found);
END;
$BODY$
LANGUAGE 'plpgsql';

END;
