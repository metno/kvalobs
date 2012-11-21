-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
-- 
-- kvalobs - quality assurance system for meteorological observations.
--
-- Copyright (C) 2007 met.no
--
--  Contact information:
--  Norwegian Meteorological Institute
--  Box 43 Blindern
--  0313 OSLO
--  NORWAY
--  E-mail: kvoss@met.no
--
--  This is free software; you can redistribute it and/or modify
--  it under the terms of the GNU General Public License as published by
--  the Free Software Foundation; either version 2 of the License, or
--  (at your option) any later version.
--
-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

-- Kvalobs admin role
CREATE ROLE kv_admin NOLOGIN SUPERUSER;

-- Kvalobs read/write/delete
CREATE ROLE kv_write NOLOGIN;

--Kvalobs read only role.
CREATE ROLE kv_read NOLOGIN;

--A role to be used only by the kvalobs software
--Set a default password. This shuold be altered by an administrator.
CREATE ROLE kvproc INHERIT LOGIN PASSWORD 'kvproc' IN ROLE kv_write ;

--A kvalobs administration role.
--Set a default password. This shuold be altered by an administrator.
CREATE ROLE kvadmin INHERIT LOGIN PASSWORD 'kvadmin12' IN ROLE kv_admin ;


--A role to use for replication.
--CREATE ROLE kvrep INHERIT LOGIN REPLICATION PASSWORD 'kvrep#21';
