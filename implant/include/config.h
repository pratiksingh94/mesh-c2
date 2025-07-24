// if you see this, that means i forgot properly add this in .gitignore

#ifndef CONFIG_H
#define CONFIG_H

#define C2_URL     "http://10.216.9.89:8000" // Change this to your C2 server URL
#define PSK        "supersecret32bytekeygoeshere123456"  // 32 bytes

#define WEBSERVER_PORT 8080

#define HEARTBEAT_INTERVAL 15   // Heartbeat interval in seconds
#define PAYLOAD_INTERVAL   30  // Payload interval in seconds
#define REPORT_INTERVAL    60  // Report interval in seconds
#define SYNC_INTERVAL      90 // Sync interval in seconds

#endif
