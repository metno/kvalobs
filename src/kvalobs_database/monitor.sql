-- Add a function that give access to pg_stat_replication without using a susperuser.
-- This is needed for postgres < versjon 10
-- In versions >= 10 one can use the role pg_monitor.


CREATE FUNCTION func_stat_replication() RETURNS SETOF pg_stat_replication as
$$ select * from pg_stat_replication; $$
LANGUAGE sql SECURITY DEFINER;


REVOKE EXECUTE ON FUNCTION func_stat_replication() FROM public;

-- Grant EXECUTE to the user that you want to give access to this function
-- ex. monitor
GRANT EXECUTE ON FUNCTION func_stat_replication() to monitor;



-- SELECT * FROM func_stat_replication();

-- Versions >= 10
-- We could grant access to all stats and monitor features as
--
-- GRANT pg_monitor TO monitor;