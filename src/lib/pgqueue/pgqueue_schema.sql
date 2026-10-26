BEGIN;

-- Create the database and create the schema.
-- createdb -O kvqueue  -e pgqueue
-- psql -h localhost pgqueue kvqueue < pgqueue_schema_one.sql


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


CREATE TABLE IF NOT EXISTS messages (
    id         bigserial PRIMARY KEY,
    topic      text NOT NULL,
    data       bytea NOT NULL,
    content_type text NOT NULL default 'application/octet-str',
    crc        uuid GENERATED ALWAYS AS (md5(data)::uuid) STORED,
    created_at timestamptz NOT NULL DEFAULT now()
);

-- Create foreign key constraint if it does not exist for messages.topic referencing topic.topic
-- We could have set it in the CREATE TABLE statement directly, but this approach ensures the 
-- constraint exists even if the table was created earlier. It makes the schema idempotent.
DO $$
BEGIN
    IF NOT EXISTS (
        SELECT 1
        FROM pg_constraint
        WHERE conname = 'fk_messages_topic'
          AND conrelid = 'messages'::regclass
    ) THEN
        ALTER TABLE messages 
        ADD CONSTRAINT fk_messages_topic 
        FOREIGN KEY (topic) REFERENCES topic (topic);
    END IF;
END;
$$;

CREATE INDEX IF NOT EXISTS idx_messages_topic_id   ON messages (topic, id);
CREATE INDEX IF NOT EXISTS idx_messages_crc        ON messages (crc);
CREATE INDEX IF NOT EXISTS idx_messages_created_at ON messages (created_at);

REVOKE ALL ON messages FROM public;
GRANT ALL ON messages TO kv_admin;
GRANT SELECT ON messages TO kv_read;
GRANT SELECT, UPDATE, INSERT, DELETE ON messages TO kv_write;
GRANT USAGE, SELECT ON SEQUENCE messages_id_seq TO kv_write,kv_admin;


CREATE TABLE IF NOT EXISTS consumer_offsets (
    consumer_name text NOT NULL,
    topic         text NOT NULL,
    last_id       bigint NOT NULL DEFAULT 0,
    updated_at    timestamptz NOT NULL DEFAULT now(),
    PRIMARY KEY (consumer_name, topic)
);

-- Create foreign key constraint if it does not exist for consumer_offsets.topic referencing topic.topic
DO $$
BEGIN
    IF NOT EXISTS (
        SELECT 1
        FROM pg_constraint
        WHERE conname = 'fk_consumer_offsets_topic'
          AND conrelid = 'consumer_offsets'::regclass
    ) THEN
        ALTER TABLE consumer_offsets 
        ADD CONSTRAINT fk_consumer_offsets_topic 
        FOREIGN KEY (topic) REFERENCES topic (topic);
    END IF;
END;
$$;

REVOKE ALL ON consumer_offsets FROM public;
GRANT ALL ON consumer_offsets TO kv_admin;
GRANT SELECT ON consumer_offsets TO kv_read;
GRANT SELECT, UPDATE, INSERT, DELETE ON consumer_offsets TO kv_write,kv_read;


CREATE TABLE IF NOT EXISTS retention (
    topic      text NOT NULL,
    retention  interval NOT NULL,
    PRIMARY KEY (topic)
);

REVOKE ALL ON retention FROM public;
GRANT ALL ON retention TO kv_admin;
GRANT SELECT ON retention TO kv_read;
GRANT SELECT, UPDATE, INSERT, DELETE ON retention TO kv_write;
GRANT SELECT ON retention TO kv_read;


--
-- Function to be run by the pg_cron job to delete old data on the retention settings in kvretention.
--
CREATE OR REPLACE FUNCTION run_retention() RETURNS int AS $$
DECLARE
    r          record;
    batch_size int := 10000;
    deleted    int;
    n int := 0;
BEGIN
    FOR r IN
        SELECT topic, retention FROM retention WHERE topic <> 'default'
    LOOP
        LOOP
            DELETE FROM messages
            WHERE id IN (
                SELECT id FROM messages
                  WHERE topic = r.topic
                    AND created_at < now() - r.retention::interval
                  ORDER BY id LIMIT batch_size
            );

            GET DIAGNOSTICS deleted = ROW_COUNT;
            n := n + deleted;
            EXIT WHEN deleted < batch_size;

            PERFORM pg_sleep(0.1);  -- brief pause between batches to avoid I/O spikes
        END LOOP;
    END LOOP;

    -- Delete messages for topics not explicitly listed in the retention table based on the default retention setting
    FOR r IN
        SELECT COALESCE(retention, '1 hour') AS retention FROM retention WHERE topic = 'default'
    LOOP
        LOOP
            DELETE FROM messages
            WHERE id IN (
                SELECT id FROM messages
                WHERE topic NOT IN (SELECT topic FROM retention WHERE topic <> 'default')
                  AND created_at < now() - r.retention::interval
                ORDER BY id
                LIMIT batch_size
            );

            GET DIAGNOSTICS deleted = ROW_COUNT;
            n := n + deleted;
            EXIT WHEN deleted < batch_size;

            PERFORM pg_sleep(0.1);  -- brief pause between batches to avoid I/O spikes
        END LOOP;
    END LOOP;

    RETURN n;
END;
$$ LANGUAGE plpgsql;


--
-- Default value used by the pg_cron job to delete old data on the retention settings in kvretention.
--
INSERT INTO retention (topic, retention) VALUES
    ('default', '1 hour'),
    ('kvalobs.staging.raw', '12 hour'),
    ('kvalobs.staging.checked', '1 day'),
    ('kvalobs.staging.checked.lowpri', '1 day'),
    ('kvalobs.production.raw', '2 days'),
    ('kvalobs.production.checked', '3 days'),
    ('kvalobs.production.checked.lowpri', '3 days'),
    ('kvalobs.development.raw', '1 hour'),
    ('kvalobs.development.checked', '12 hours'),
    ('kvalobs.development.checked.lowpri', '12 hours')
ON CONFLICT DO NOTHING;

--
-- Default topics and their descriptions.
--
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



END;

