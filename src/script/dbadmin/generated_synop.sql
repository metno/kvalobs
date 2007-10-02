--
-- Select for å finne alle stasjoner som vi genererer SYNOP for
-- og som skal legges inn i tabellen generated_types og typeid blir satt 
-- til -1*typeid.
--
select stationid from data where obstime>(timestamp 'today'-interval'7 day') 
       AND ((typeid>=300 AND typeid<400 and typeid!=302) OR  
	typeid=3 OR typeid=6) 	
	GROUP BY stationid order by stationid;
