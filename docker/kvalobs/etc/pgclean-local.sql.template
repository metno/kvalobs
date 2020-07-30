-- This sql script cleans up the kvalobs database. It deletes records
-- that is too old from the table workstatistik

-- This clean up script is run once a day by cron.
-- It is run at every night in a low activity period, ex kl 1:25.

--
-- Deletes from workstatistik all records that is more than 1 month old.
--
select timestamp 'today'- interval'31 day' AS "Deleting workstatistik before!";
delete from workstatistik  where tbtime<=(timestamp 'today'-interval'31 day');


