BEGIN;

CREATE ROLE kvalobs INHERIT LOGIN PASSWORD 'kvalobs12' IN ROLE kv_write, kv_read ;

END;
