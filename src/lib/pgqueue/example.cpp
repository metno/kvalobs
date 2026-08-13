#include "pgmessage.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    try {
        // -------------------------------------------------------------------
        // 1. Build the cluster — pass every node you know about.
        //    PgCluster probes each one, identifies primary vs replica,
        //    and caches the result for 60 seconds (configurable).
        // -------------------------------------------------------------------
        PgCluster cluster({
            "host=pg-primary  dbname=weather_prod user=app password=secret",
            "host=pg-replica1 dbname=weather_prod user=app password=secret",
        });

        // Optional: inspect what was found.
        for (const auto& n : cluster.topology()) {
            std::cout << (n.is_primary ? "PRIMARY" : "REPLICA")
                      << " : " << n.conninfo << "\n";
        }

        // -------------------------------------------------------------------
        // 2. Create a PgMessaging instance.
        //    Internally opens two connections:
        //      primary_ → writes (publish, commit_offset, ...)
        //      replica_ → reads  (poll, get_offset, consumer_lag)
        // -------------------------------------------------------------------
        PgMessaging mq(cluster);

        // -------------------------------------------------------------------
        // 3. Setup (writes go to primary)
        // -------------------------------------------------------------------
        mq.create_topic("weather_raw");
        mq.register_consumer("aggregator", "weather_raw");

        // -------------------------------------------------------------------
        // 4. Producer — publishes to primary
        // -------------------------------------------------------------------
        std::string csv =
            "station_id,temp,humidity,ts\n"
            "42,18.5,72,2026-06-20T08:00:00Z";

        long long id = mq.publish("weather_raw", csv);
        std::cout << "Published id=" << id << "\n";

        // Dedup publish — second call returns same id, no duplicate row.
        long long id2 = mq.publish_dedup("weather_raw", csv);
        std::cout << "Dedup id=" << id2 << " (same as " << id << ")\n";

        // -------------------------------------------------------------------
        // 5. Consumer poll loop — reads from replica, commits to primary
        // -------------------------------------------------------------------
        while (true) {
            // poll() reads from replica_
            auto batch = mq.poll("aggregator", "weather_raw", 1000);

            if (batch.empty()) {
                std::cout << "No new messages, sleeping 5s...\n";
                std::this_thread::sleep_for(std::chrono::seconds(5));
                continue;
            }

            for (const auto& msg : batch) {
                // Send to external system here.
                std::cout << "id=" << msg.id
                          << " topic=" << msg.topic
                          << " created_at=" << msg.created_at << "\n"
                          << msg.data << "\n\n";
            }

            // commit_offset() writes to primary_
            mq.commit_offset("aggregator", "weather_raw", batch.back().id);
            std::cout << "Committed offset=" << batch.back().id << "\n";

            if (static_cast<int>(batch.size()) < 1000)
                break;  // caught up
        }

        // consumer_lag() reads from replica_
        std::cout << "Lag: " << mq.consumer_lag("aggregator", "weather_raw") << "\n";

        // -------------------------------------------------------------------
        // 6. Force a re-probe (e.g. after a failover event)
        // -------------------------------------------------------------------
        cluster.reprobe();

    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}
