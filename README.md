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

The Pi Zero 2 W does not support 5Ghz and xfinitywifi hotspots have [used 5Ghz](https://xdaforums.com/t/xfinity-hotspots-are-now-all-5ghz-connect-with-a-2-4ghz-phone.4293719/) for a while now. This means that we need a 5Ghz USB Wi-Fi dongle and a microUSB male to USB-A female adapter further raising our costs. 

## Dependencies

Install dependencies using apt:
```
sudo apt install libwpa-client-dev libcurl4-openssl-dev libssl-dev zlib1g-dev libc6-dev
```

### Requirements

- [Raspberry Pi Zero W 2](https://www.raspberrypi.com/products/raspberry-pi-zero-2-w/) - $15
- MicroSD card - $7
- MicroUSB cable - $5?
- USB power bank or other power supply - $10
- 5GHz USB Wi-Fi dongle - $8
- USB-A female to microUSB male adapter - $6
- Access to XFINITY hotspots
  - You must be an Xfinity postpaid subscriber or buy the Xfinity NOW pass ($10/month)

Total: $55 (as an Xfinity subscriber) or $65+$10/month.
### cartrackerd

A daemon written in C++ that handles automatic connections to XFINITY hotspots and sending nearby SSIDs (and RSSI levels), bluetooth devices, and other data to the server.

### cartracker-server

A server written in Node.js that accepts data from the daemon and uses it to find the Pi's location. The daemon also stores data in a database and offers a web UI for administration and data viewing purposes.
