-- Inntil klima har fatt i gang sitt mottakssytem tar vi vare pa data og
-- modell_data i 8 maneder.
-- Den originale rydde filen er tatt vare pa som pgclean.sql.20060320.


--
-- This sql script cleans up the kvalobs database. It deletes records 
-- that is to old from the tables data, text_data, rejectdecode, workstatistik 
-- and model_data.
--
-- To old means:
-- data and text_data: older that 8 months, use tbtime.
-- rejectdecode:       older than 1 month, use ttbtime.
-- workstatistik:      older than 1 month, use tbtime.
-- model_data:         older than 8 months, use obstime.
--
-- This clean up script is run once a day by cron.
-- It is run at every night in a low activity period, ex kl 1:25.
--

--
-- Deletes data that is too old, ie older than 8 months'.
-- It is deleted from both data and text_data tables.
--
select timestamp 'today'- interval'250 day' AS "Deleting data before!";
delete from data where tbtime<=(timestamp 'today'-interval'3 months');
delete from text_data where tbtime<=(timestamp 'today'-interval'3 months');

delete from data_history where tbtime<(timestamp 'today'-interval'3 months');
delete from text_data_history where tbtime<(timestamp 'today'-interval'3 months');

--
-- Deletes from rejectdecode all records that is more than 1 month old.
--
select timestamp 'today'- interval'31 day' AS "Deleting rejectdecode before!";
delete from rejectdecode where tbtime<=(timestamp 'today'-interval'31 day');

--
-- Deletes from workstatistik all records that is more than 1 month old.
--
select timestamp 'today'- interval'31 day' AS "Deleting workstatistik before!";
delete from workstatistik  where tbtime<=(timestamp 'today'-interval'31 day');

--
-- Deletes from model_data all records that is more than 8 months old.
--
select timestamp 'today'- interval'3 months' AS "Deleting model_data before!";
delete from model_data  where obstime<=(timestamp 'today'-interval'3 months');
