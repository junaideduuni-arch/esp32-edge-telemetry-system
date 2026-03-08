┌─────────────────────────────────────────────────────────────┐
│                     ESP32 EDGE DEVICE                       │
│-------------------------------------------------------------│
│ Hardware                                                    │
│ • ESP32 Microcontroller                                     │
│ • Temperature Sensor (/simulated sensor/NTC /DHT22)         │                              
│                                                             │
│ Firmware Logic (C++ / Arduino Framework)                    │
│                                                             │
│ 1. Initialize Sensor                                        │
│ 2. Connect to WiFi                                          │
│ 3. Connect MQTT Client                                      │
│ 4. Read Temperature Data                                    │
│ 5. Publish Temperature Data                                 │ 
│                                                             │
│ Decision Logic                                              │
│                                                             │
│ IF WiFi Connected AND MQTT Connected                        │
│    → Create JSON Telemetry Payload                          │
│    → Publish MQTT Message                                   │
│                                                             │
│ ELSE                                                        │
│    → Store payload locally in SPIFFS                        │
│    → Retry on next wake cycle                               │
│                                                             │
│ Local Buffer Example                                        │
│ /spiffs/cache.json                                          │
│                                                             │
│ Power Optimization                                          │
│ • Deep Sleep Mode                                           │
│ • Wake every 5 minutes                                      │
│                                                             │
│ Wake Cycle                                                  │
│ Wake → Read Sensor → Send Data → Sleep                      │
│                                                             │
│ MQTT Topic                                                  │
│ hospital/pharmacy/temp                                      │
│                                                             │
│ Payload Example                                             │
│ {                                                           │
│  "device_id":"esp32_01",                                    │
│  "temperature":4.8,                                         │
│  "timestamp":"2026-03-08T12:00:00Z"                         │
│ }                                                           │
└─────────────────────────────────────────────────────────────┘
                            │
                            │ WiFi Network
                            ▼
┌─────────────────────────────────────────────────────────────┐
│              LOCAL MQTT BROKER (MOSQUITTO)                  │
│                     Running on Local PC                     │
│-------------------------------------------------------------│
│ Broker Software                                             │
│ Mosquitto MQTT Broker                                       │
│                                                             │
│ Network Configuration                                       │
│ Host: Local PC                                              │
│ Port: 1883                                                  │
│                                                             │
│ Broker Responsibilities                                     │
│ • Receive telemetry from ESP32                              │
│ • Manage publish/subscribe topics                           │
│ • Maintain local data flow even if internet fails           │
│                                                             │
│ Security Layer                                              │
│ • Username/Password Authentication                          │
│ • Optional TLS Encryption                                   │
│                                                             │
│ Topic Example                                               │
│ hospital/pharmacy/temperature                               │
│                                                             │
│ MQTT Bridge / Client                                        │
│ Sends data to automation pipeline (n8n)                     │
└─────────────────────────────────────────────────────────────┘
                            │
                            │ MQTT Subscription
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                 SELF HOSTED N8N AUTOMATION                  │
│                     Running Locally                         │
│-------------------------------------------------------------│
│ Workflow Pipeline                                           │
│                                                             │
│  MQTT Trigger Node                                          │
│      │                                                      │
│      ▼                                                      │
│  JavaScript Function Node                                   │
│  (Telemetry Processing Logic)                               │
│                                                             │
│  Example Logic                                              │
│  - Parse incoming payload                                   │
│  - Extract device_id and temperature                        │
│  - Check alert threshold                                    │
│                                                             │
│  IF temperature > 5°C                                       │                                                                                                          
│     → Trigger Alert                                         │
│                                                             │
│ Parallel Workflow                                           │
│                                                             │
|                  ┌───────────────|                          |         
|                  ▼               ▼                          |         
|              Store Data      Send Alert                     |         
│                                                             │
│ Nodes Used                                                  │
│ • MQTT Trigger Node                                         │
│ • JavaScript Code Node                                      │
│ • Supabase Node                                             │
│ • HTTP Request Node                                         │
│                                                             │
│ HTTP Request Node                                           │
│ Sends WhatsApp Alert via Typebot API                        │
└─────────────────────────────────────────────────────────────┘
                            │
                            │ Database Insert
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                  SUPABASE POSTGRES DATABASE                 │
│-------------------------------------------------------------│
│ Table: temperature_logs                                     │
│                                                             │
│ Columns                                                     │
│                                                             │
│ id                UUID (Primary Key)                        │
│ device_id         TEXT                                      │
│ temperature       FLOAT                                     │
│ created_at        TIMESTAMP                                 │
│                                                             │
│ Indexing Strategy                                           │
│ INDEX(device_id)                                            │
│ INDEX(created_at)                                           │
│                                                             │
│ Advantages                                                  │
│ • Fast time-series queries                                  │
│ • Supports 100k+ rows easily                                │
│ • Compatible with Grafana                                   │
└─────────────────────────────────────────────────────────────┘
                            │
                            │ Query Data
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                    GRAFANA DASHBOARD                        │
│-------------------------------------------------------------│
│ Data Source                                                 │
│ PostgreSQL (Supabase)                                       │
│                                                             │
│ Dashboard Panels                                            │
│                                                             │
│ 1. Temperature Time-Series Chart                            │
│    Shows sensor data vs time                                │
│                                                             │
│ 2. Current Temperature Gauge                                │
│    Displays latest value                                    │
│                                                             │
│                                                             │
│ Example Query                                               │
│                                                             │
│ SELECT                                                      │
│ created_at,                                                 │
│ temperature                                                 │
│ FROM temperature_logs                                       │
│ ORDER BY created_at DESC                                    │
│ LIMIT 100000                                                │
└─────────────────────────────────────────────────────────────┘
