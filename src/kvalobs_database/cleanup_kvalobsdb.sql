select timestamp 'today'- interval'180 day' AS "Deleting data before!";
select * from data where  tbtime <(timestamp 'today'-interval'180 day');
select timestamp 'today'- interval'31 day' AS "Deleting workstatistik before!";
select * from data where  tbtime <(timestamp 'today'-interval'31 day');