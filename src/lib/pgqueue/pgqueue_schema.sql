BEGIN;


CREATE OR REPLACE FUNCTION 
pgqueue_version()
RETURNS text AS
$BODY$
BEGIN
	RETURN '1.0.0';
END;
$BODY$
LANGUAGE plpgsql IMMUTABLE;

CREATE TABLE IF NOT EXISTS topic (
    topic      text NOT NULL,
    description text,
    PRIMARY KEY (topic)
);
REVOKE ALL ON topic FROM public;
GRANT ALL ON topic TO kv_admin;
GRANT SELECT ON topic TO kv_read;
GRANT SELECT, UPDATE, INSERT, DELETE ON topic TO kv_write;

INSERT INTO topic (topic, description) VALUES
    ('kvalobs.staging.raw', 'Raw data from the kvalobs system'),
    ('kvalobs.staging.checked', 'Checked data from the kvalobs system'),
    ('kvalobs.staging.checked.lowpri', 'Low priority checked data from the kvalobs system'),
    ('kvalobs.production.raw', 'Raw data from the kvalobs system'),
    ('kvalobs.production.checked', 'Checked data from the kvalobs system'),
    ('kvalobs.production.checked.lowpri', 'Low priority checked data from the kvalobs system'),
    ('kvalobs.development.raw', 'Raw data from the kvalobs system'),
    ('kvalobs.development.checked', 'Checked data from the kvalobs system'),
    ('kvalobs.development.checked.lowpri', 'Low priority checked data from the kvalobs system')
ON CONFLICT (topic) DO NOTHING;



CREATE TABLE IF NOT EXISTS staging (
    id         bigserial PRIMARY KEY,
    topic      text NOT NULL,
    data       text NOT NULL,
    crc        uuid GENERATED ALWAYS AS (md5(data)::uuid) STORED,
    created_at timestamptz NOT NULL DEFAULT now(),
    priority   int NOT NULL DEFAULT 0
);

-- Create foreign key constraint for kvstaging.topic referencing kvtopic.topic if it does not already exist
DO $$
BEGIN
    IF NOT EXISTS (
        SELECT 1
        FROM pg_constraint
        WHERE conname = 'fk_staging_topic'
          AND conrelid = 'staging'::regclass
    ) THEN
        ALTER TABLE staging 
        ADD CONSTRAINT fk_staging_topic 
        FOREIGN KEY (topic) REFERENCES topic (topic);
    END IF;
END;
$$;


CREATE INDEX IF NOT EXISTS idx_staging_topic_id   ON staging (topic, id);
CREATE INDEX IF NOT EXISTS idx_staging_crc        ON staging (crc);
CREATE INDEX IF NOT EXISTS idx_staging_created_at ON staging (created_at);
REVOKE ALL ON staging FROM public;
GRANT ALL ON staging TO kv_admin;
GRANT SELECT ON staging TO kv_read;
GRANT SELECT, UPDATE, INSERT, DELETE ON staging TO kv_write;
GRANT USAGE, SELECT ON SEQUENCE staging_id_seq TO kv_write,kv_admin;



CREATE TABLE IF NOT EXISTS production (
    id         bigserial PRIMARY KEY,
    topic      text NOT NULL,
    data       text NOT NULL,
    crc        uuid GENERATED ALWAYS AS (md5(data)::uuid) STORED,
    created_at timestamptz NOT NULL DEFAULT now(),
    priority   int NOT NULL DEFAULT 0
);

-- Create foreign key constraint if it does not exist for production.topic referencing topic.topic
DO $$
BEGIN
    IF NOT EXISTS (
        SELECT 1
        FROM pg_constraint
        WHERE conname = 'fk_production_topic'
          AND conrelid = 'production'::regclass
    ) THEN
        ALTER TABLE production 
        ADD CONSTRAINT fk_production_topic 
        FOREIGN KEY (topic) REFERENCES topic (topic);
    END IF;
END;
$$;

CREATE INDEX IF NOT EXISTS idx_production_topic_id   ON production (topic, id);
CREATE INDEX IF NOT EXISTS idx_production_crc        ON production (crc);
CREATE INDEX IF NOT EXISTS idx_production_created_at ON production (created_at);

REVOKE ALL ON production FROM public;
GRANT ALL ON production TO kv_admin;
GRANT SELECT ON production TO kv_read;
GRANT SELECT, UPDATE, INSERT, DELETE ON production TO kv_write;
GRANT USAGE, SELECT ON SEQUENCE production_id_seq TO kv_write,kv_admin;



CREATE TABLE IF NOT EXISTS development (
    id         bigserial PRIMARY KEY,
    topic      text NOT NULL,
    data       text NOT NULL,
    crc        uuid GENERATED ALWAYS AS (md5(data)::uuid) STORED,
    created_at timestamptz NOT NULL DEFAULT now(),
    priority   int NOT NULL DEFAULT 0
);


-- Create foreign key constraint for development.topic referencing topic.topic if it does not already exist
DO $$
BEGIN
    IF NOT EXISTS (
        SELECT 1
        FROM pg_constraint
        WHERE conname = 'fk_development_topic'
          AND conrelid = 'development'::regclass
    ) THEN
        ALTER TABLE development 
        ADD CONSTRAINT fk_development_topic 
        FOREIGN KEY (topic) REFERENCES topic (topic);
    END IF;
END;
$$;


CREATE INDEX IF NOT EXISTS idx_development_topic_id   ON development (topic, id);
CREATE INDEX IF NOT EXISTS idx_development_crc        ON development (crc);
CREATE INDEX IF NOT EXISTS idx_development_created_at ON development (created_at);

REVOKE ALL ON development FROM public;
GRANT ALL ON development TO kv_admin;
GRANT SELECT ON development TO kv_read;
GRANT SELECT, UPDATE, INSERT, DELETE ON development TO kv_write;
GRANT USAGE, SELECT ON SEQUENCE development_id_seq TO kv_write,kv_admin;

CREATE TABLE IF NOT EXISTS consumer_offsets (
    consumer_name text NOT NULL,
    topic         text NOT NULL,
    last_id       bigint NOT NULL DEFAULT 0,
    updated_at    timestamptz NOT NULL DEFAULT now(),
    PRIMARY KEY (consumer_name, topic)
);

REVOKE ALL ON consumer_offsets FROM public;
GRANT ALL ON consumer_offsets TO kv_admin;
GRANT SELECT ON consumer_offsets TO kv_read;
GRANT SELECT, UPDATE, INSERT, DELETE ON consumer_offsets TO kv_write,kv_read;


CREATE TABLE IF NOT EXISTS retention (
    table_name text NOT NULL,
    topic      text NOT NULL,
    retention  interval NOT NULL,
    PRIMARY KEY (table_name, topic)
);

--
-- Function to be run by the pg_cron job to delete old data on the retention settings in kvretention.
--
CREATE OR REPLACE FUNCTION run_retention() RETURNS void AS $$
DECLARE
    r          record;
    batch_size int := 10000;
    deleted    int;
BEGIN
    FOR r IN
        SELECT table_name, topic, retention FROM retention
    LOOP
        LOOP
            EXECUTE format(
                'DELETE FROM %I
                 WHERE id IN (
                     SELECT id FROM %I
                     WHERE topic = %L
                       AND created_at < now() - %L::interval
                     ORDER BY id
                     LIMIT %L
                 )',
                r.table_name,
                r.table_name,
                r.topic,
                r.retention,
                batch_size
            );

            GET DIAGNOSTICS deleted = ROW_COUNT;
            EXIT WHEN deleted < batch_size;

            PERFORM pg_sleep(0.1);  -- brief pause between batches to avoid I/O spikes
        END LOOP;
    END LOOP;
END;
$$ LANGUAGE plpgsql;


--
-- Default value used by the pg_cron job to delete old data on the retention settings in kvretention.
--
INSERT INTO retention (table_name, topic, retention) VALUES
    ('staging', 'kvalobs.staging.raw', '12 hour'),
    ('staging', 'kvalobs.staging.checked', '1 day'),
    ('staging', 'kvalobs.staging.checked.lowpri', '1 day'),
    ('production', 'kvalobs.production.raw', '2 days'),
    ('production', 'kvalobs.production.checked', '3 days'),
    ('production', 'kvalobs.production.checked.lowpri', '3 days'),
    ('development', 'kvalobs.development.raw', '1 hour'),
    ('development', 'kvalobs.development.checked', '12 hours'),
    ('development', 'kvalobs.development.checked.lowpri', '12 hours')
ON CONFLICT DO NOTHING;


END;

