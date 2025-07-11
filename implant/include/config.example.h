#ifndef CONFIG_H
#define CONFIG_H

#define C2_URL     "http://127.0.0.1:8000" // Change this to your C2 server URL (IP and PORT)
#define PSK        "supersecret32bytekeygoeshere123456"  // 32 bytes, this is useless for now

#define WEBSERVER_PORT 8080 // Where your implant's web server will be


// you can leave these if you want
#define HEARTBEAT_INTERVAL 15   // Heartbeat interval in seconds
#define PAYLOAD_INTERVAL   30  // Payload interval in seconds
#define REPORT_INTERVAL    60  // Report interval in seconds
#define SYNC_INTERVAL      90 // Sync interval in seconds

#endif
