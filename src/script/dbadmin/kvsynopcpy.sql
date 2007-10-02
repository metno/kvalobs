-- 
-- This sql script is used to make a copy of a kvsynopd database.
-- 

.mode insert data
select * from data where obstime>=date('now', '-7  day');

.mode insert synop
select * from synop where obstime>=date('now', '-31  day');
