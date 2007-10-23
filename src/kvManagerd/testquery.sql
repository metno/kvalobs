select * from obs_pgm  OP1 WHERE OP1.typeid=3 AND OP1.fromtime=(
             SELECT MAX(OP2.fromtime) FROM obs_pgm OP2 WHERE
                    OP2.fromtime<='2006-10-05 09:00:00' AND
                    OP2.stationid  = OP1.stationid AND
                    OP2.typeid  = OP1.typeid AND
                    OP2.paramid = OP1.paramid AND
                    OP2.level = OP1.level
       ) ORDER BY OP1.stationid, paramid;
 


