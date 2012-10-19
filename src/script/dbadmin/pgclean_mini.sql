-- 
-- Deletes from workstatistik all records that is more than 1 month old.
-- 
select timestamp 'today'- interval'31 day' AS "Deleting workstatistik before!";
delete from workstatistik  where tbtime<=(timestamp 'today'-interval'31 day'); 
