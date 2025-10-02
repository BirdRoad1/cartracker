# CarTracker

A simple Raspberry Pi 3-based geolocation tracker.

https://cartracker.jlmsz.com/

<img src="https://github.com/user-attachments/assets/2fb9218f-85f8-4b04-bd75-c67f1cea32f6" width="400">


### What is this?

This project leverages open Wi-Fi networks such as Xfinity's [hotspots](https://finder.wifi.xfinity.com/) available throughout the US. The tracker connects to certain open networks and uses the internet connection to send a list of nearby SSIDs to a server, which then uses Google's Geolocation API to derive a rough location.

This project supports any 2.4/5Ghz Wi-Fi networks that are open and do not have captive portals.

#### Use case

- Anti-theft
- Vehicle location monitoring
- Asset tracking
- SSID collection (for Wigle.net or similar)

#### Limitations

The functionality of this project depends on how many open Wi-Fi hotspots are in your area.

## Dependencies

Install dependencies on the Pi using apt:
```
sudo apt install cmake make g++ libcurl4-openssl-dev iw
```

### Requirements

- [Raspberry Pi 3 Model A+ or similar](https://www.raspberrypi.com/products/raspberry-pi-3-model-a-plus/) - $25
- MicroSD card - $7
- MicroUSB cable - $5?
- USB power bank or other power supply - $10
- Access to open networks such as XFINITY hotspots
  - To use XFINITY hotspots, you must be an Xfinity postpaid internet subscriber or have the Xfinity NOW pass ($10/month)

Total: $47 USD

### Components

#### cartrackerd

A daemon written in C++ that handles automatic connections to XFINITY hotspots and sending nearby SSIDs (and RSSI levels), bluetooth devices, and other data to the server.

#### cartracker-server

A server written in Node.js that accepts data from the daemon and uses it to find the Pi's location. The daemon also stores data in a database and offers a web UI for administration and data viewing purposes.
