# CarTracker

A simple Raspberry Pi Zero W 2-based GPS tracker.

### Project vision

This project should leverage Xfinity's [hotspots](https://finder.wifi.xfinity.com/) available throughout the US. The tracker should automatically connect to the nearest hotspot and gather a list of nearby SSIDs and potentially bluetooth devices, and send them to the server. The server should then be able to use Google or other APIs to resolve a rough GPS location from the SSIDs.

#### Use case

- Anti-theft
- Vehicle location monitoring
- Asset tracking
- SSID collection (for Wigle.net or similar)

#### Limitations

The functionality of this project depends on how many XFINITY hotspots are in your area.

Due to the lack of a GPS module, the location updates may not be very precise. I'll make a follow-up post after developing this project and testing it out for a few weeks.

The Raspberry Pi's processor is weak, and relying on a battery means our code must be as lightweight as possible and unecessary services should be disabled.

## Dependencies

Install dependencies using apt:
```
sudo apt install libwpa-client-dev
```

### Requirements

- [Raspberry Pi Zero W 2](https://www.raspberrypi.com/products/raspberry-pi-zero-2-w/) - $15
- [MicroSD card](https://www.amazon.com/SanDisk-Endurance-microSDXC-Adapter-Monitoring/dp/B07P4HBRMV) - $23
- MicroUSB cable - $5?
- USB power bank or other power supply - $10
- Access to XFINITY hotspots
  - You must be an Xfinity postpaid subscriber or buy the Xfinity NOW pass ($10/month)

### cartrackerd

A daemon written in C++ that handles automatic connections to XFINITY hotspots and sending nearby SSIDs (and RSSI levels), bluetooth devices, and other data to the server.

### cartracker-server

A server written in Node.js that accepts data from the daemon and uses it to find the Pi's location. The daemon also stores data in a database and offers a web UI for administration and data viewing purposes.
