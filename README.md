# ESP32 Device Control Async API

A flexible and secure ESP32-based device control system with dynamic WiFi configuration capabilities and a RESTful API interface.

## Features

- **Dual WiFi Mode Support**
  - Access Point (AP) Mode: Creates its own network at 192.168.10.1
  - Station (STA) Mode: Connects to existing WiFi networks with configurable static IP
  - Dynamic mode switching via API endpoints

- **Secure API Implementation**
  - Bearer token authentication for all endpoints
  - JSON-based request/response format
  - Async request handling for improved performance

- **Persistent Configuration**
  - WiFi credentials storage using ESP32 Preferences
  - Mode persistence across device restarts
  - Configurable static IP settings

## Use Cases

1. **IoT Device Control**
   - Remote device management through REST API
   - Secure access control with token authentication
   - Easy integration with home automation systems

2. **Network-Flexible Deployments**
   - Initial setup in AP mode for configuration
   - Seamless transition to existing networks
   - Fallback AP mode for maintenance

3. **Industrial Control Systems**
   - Reliable device state management
   - Secure command interface
   - Static IP support for stable addressing

## Project Structure

```
├── doc/                    # Detailed documentation
│   ├── API.md             # API endpoint documentation
│   ├── architecture.md     # System architecture details
│   ├── setup.md           # Build and deployment guide
│   └── troubleshooting.md # Common issues and solutions
├── include/               # Header files
├── lib/                  # Project dependencies
├── src/                  # Source code
│   └── main.cpp          # Main application logic
├── test/                 # Test files
└── platformio.ini        # PlatformIO configuration
```

## Quick Start

1. Clone the repository
2. Install PlatformIO
3. Build and flash to your ESP32 device
4. Connect to the device's AP network (default IP: 192.168.10.1)
5. Configure WiFi settings via the API

For detailed setup instructions, see [doc/setup.md](doc/setup.md).

## Prerequisites

- ESP32 Development Board
- PlatformIO IDE or VS Code with PlatformIO extension
- Basic understanding of REST APIs
- WiFi network for STA mode operation

## Building and Deployment

```powershell
# Build the project
pio run

# Upload to ESP32
pio run --target upload

# Monitor serial output
pio device monitor
```

## Documentation

- [API Documentation](doc/API.md)
- [Architecture Overview](doc/architecture.md)
- [Setup Guide](doc/setup.md)
- [Troubleshooting](doc/troubleshooting.md)

## Development Status

This project is actively maintained and follows semantic versioning. Check the documentation for the latest features and updates.

## Contributing

1. Fork the repository
2. Create a feature branch
3. Submit a pull request

## License

This project is released under the MIT License. See the LICENSE file for details.

## Contact

For issues and feature requests, please use the GitHub issue tracker.