# Node-RED Flow Documentation — Smart-Vac AIAP System

<img width="600" height="308" alt="image" src="https://github.com/user-attachments/assets/a31c6abb-1043-4f56-a8c0-97fac69e323e" />

## Overview
This Node-RED flow processes AI classification data from the ESP32 (Smart-Vac device) via MQTT and performs:
- Debug monitoring
- Data transformation
- Logging to InfluxDB
- Actuation logic
- Downlink back to the IoT device
- WhatsApp alert when clogged

------------------------------------------------------------

1) MQTT INPUT (AIAP)

Topic Structure:
AIAP/<clientid>/<device>

Example:
AIAP/57054/M

Payload Example:
{
  "label": "cleaning",
  "button": true
}

Why QoS 2?

We use Quality of Service Level 2 (QoS 2) because it guarantees EXACTLY ONCE delivery.

This is important because:
- Prevents duplicate InfluxDB entries
- Prevents repeated WhatsApp alerts
- Ensures reliable downlink actuation
- Handles unstable WiFi reconnections safely

QoS 2 works using a four-step handshake:
PUBREC → PUBREL → PUBCOMP

------------------------------------------------------------

2) DEBUG NODE (INPUT)

After MQTT In, the message flows into a Debug node.

Purpose:
- Confirms MQTT is connected
- Displays raw msg.topic
- Displays raw msg.payload
- Helps verify incoming structure

------------------------------------------------------------

3) EXTRACT & TRANSFORM (FUNCTION NODE)

<img width="525" height="484" alt="image" src="https://github.com/user-attachments/assets/b50a3c06-1018-45d5-834e-bc87a15141f9" />


This node:
- Extracts msg.payload.label → state
- Converts button value
- Splits msg.topic to extract clientid and device
- Rebuilds msg.payload into structured JSON

Output format:
[
  {
    "state": "cleaning",
    "button": "true"
  },
  {
    "clientid": "57054",
    "device": "M"
  }
]

Why this step:
- Normalizes data format
- Separates fields and tags
- Supports multiple devices
- Prepares data for logging and actuation

------------------------------------------------------------

4) FLOW SPLIT

After transformation, the flow splits into two branches:

A) Logging Path → InfluxDB  
B) Actuation Path → Decision Logic  

Logging always happens independently of alert logic.

------------------------------------------------------------

5) LOGGING PATH (INFLUXDB)

The message is formatted and written into InfluxDB.

Stored data:
- state
- button
- clientid (tag)
- device (tag)
- timestamp

Why InfluxDB?
- Optimized for time-series data
- Tracks vacuum health trends
- Enables Grafana dashboard visualization
- Helps detect repeated clog patterns

------------------------------------------------------------

6) ACTUATION NODE (DECISION LOGIC)

<img width="451" height="631" alt="image" src="https://github.com/user-attachments/assets/b04c4dcd-e750-46af-99cb-165d857338db" />


This node determines what response to send back to the ESP32.

Inputs:
let state = msg.payload[0].state;
let button = msg.payload[0].button;

Logic:

If state == "clogged":
    If button == true:
        response = "150% Power activated"
    Else:
        response = "Clogged"

Else if state == "Charge":
    response = "Charging"

Else if state == "Cleaning":
    response = "It is ON"

Else if state == "Off":
    response = "It is turned off"

Else:
    response = "Unknown state"

The node then sets:
msg.payload = response

------------------------------------------------------------

7) MQTT DOWNLINK (BACK TO DEVICE)

One branch publishes the response back to the ESP32.

Purpose:
- Update OLED display
- Trigger buzzer
- Control LEDs
- Activate boost mode

This creates a closed-loop system:
Device → Cloud Logic → Device

------------------------------------------------------------

8) SWITCH NODE (FILTER)

The second branch goes into a Switch node.

Condition:
state == "clogged"

Only clogged messages continue forward.

Why?
- Prevents WhatsApp spam
- Only alerts on fault conditions
- Keeps notifications meaningful

------------------------------------------------------------

9) WHATSAPP ALERT
    
<img width="413" height="226" alt="image" src="https://github.com/user-attachments/assets/ce3c1063-6bf9-4413-94b5-fc681ae68abf" />

If clogged condition passes the Switch node,
a WhatsApp message is sent via API.

Example messages:
"Vacuum is clogged — please check filter."
"Vacuum is clogged — 150% Power activated."

------------------------------------------------------------

COMPLETE FLOW SUMMARY

MQTT In (QoS2)
    ↓
Debug
    ↓
Extract & Transform
    ↓
Split
    ├── Logging → InfluxDB
    └── Actuation
            ├── MQTT Downlink → ESP32
            └── Switch (clogged only)
                    ↓
                 WhatsApp

------------------------------------------------------------

Design Justification:
- QoS 2 ensures reliability and prevents duplication
- Transform node standardizes data structure
- Logging separated from actuation improves modularity
- Switch prevents notification flooding
- Downlink maintains device-cloud synchronization
- Modular design supports scalability
