BEGIN;

CREATE TABLE default_missing (
	paramid INT UNIQUE,
	value FLOAT NOT NULL,
	controlinfo CHAR(16) NOT NULL
);
INSERT INTO default_missing VALUES (NULL, -32767, '0000003000000000');
INSERT INTO default_missing VALUES (105, 0, '0000000000000000');
REVOKE ALL ON default_missing FROM public;
GRANT ALL ON default_missing TO kv_admin;
GRANT SELECT ON default_missing TO kv_read;
GRANT SELECT, UPDATE, INSERT ON default_missing TO kv_write;

CREATE VIEW default_missing_values AS
select
	p.paramid,
	m.value,
	m.controlinfo
from
	param p,
	default_missing m
where
	p.paramid=m.paramid or (
		m.paramid is null and
		p.paramid not in (
			select paramid from default_missing where paramid=p.paramid
		)
	);
REVOKE ALL ON default_missing_values FROM public;
GRANT ALL ON default_missing_values TO kv_admin;
GRANT SELECT ON default_missing_values TO kv_read;
GRANT SELECT ON default_missing_values TO kv_write;

COMMIT;
