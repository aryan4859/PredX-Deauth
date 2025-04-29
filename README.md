# PredX-Deauth

<p align="center">
  <img src="./assets/logo.png" alt="PredX Logo" width="200"/>
</p>


PredX-Deauth is a powerful tool designed for Wi-Fi network testing and stress testing using deauthentication attacks. The tool provides a web-based interface for scanning nearby Wi-Fi networks and launching deauthentication (deauth) attacks on specific targets. This project is intended for educational and ethical purposes only.

## ⚠️ Legal Disclaimer

This tool is intended for **authorized network testing** and **educational purposes only**. Unauthorized use of this tool to disrupt networks you do not own or manage is **illegal** and may result in severe penalties. The authors take no responsibility for any misuse of this tool.

---

## Features

- **Scan Wi-Fi Networks**: Discover all nearby Wi-Fi networks.
- **Deauthentication Attack**: Disconnect devices from a target network by sending deauth packets.
- **Web-Based Interface**: Intuitive and interactive UI for managing scans and attacks.
- **Responsive Design**: Works seamlessly on both desktop and mobile devices.

---

## Requirements

- **Hardware**:
  - ESP8266-based microcontroller (e.g., NodeMCU or Wemos D1 Mini).

- **Software**:
  - [PlatformIO](https://platformio.org/) (Recommended for development and flashing).
  - LittleFS file system for ESP8266.

---

## Installation

### 1. Clone the Repository
```bash
git clone https://github.com/yourusername/PredX-Deauth.git
cd PredX-Deauth
```

### 2. Configure the Environment
Ensure you have PlatformIO installed. If not, install it via:
```bash
pip install platformio
```

### 3. Upload the Code
- Connect your ESP8266 to your computer.
- Open the project in PlatformIO and run:
  ```bash
  pio run --target upload
  ```

### 4. Upload the Web Files
- Place your web files (e.g., `index.html`) in the `data/` folder.
- Upload them to the ESP8266 using:
  ```bash
  pio run --target uploadfs
  ```

---

## Usage

1. **Power On**:
   - Power on the ESP8266 and connect to the Wi-Fi network named `PredX` (default password: `predx1337`).

2. **Access the Web Interface**:
   - Open a browser and navigate to `http://192.168.4.1`.

3. **Scan for Networks**:
   - Use the **Scan Networks** button to discover nearby Wi-Fi networks.

4. **Launch an Attack**:
   - Select a network from the dropdown or enter its SSID manually.
   - Click **Start Attack** to begin the deauth attack.
   - Use **Stop Attack** to stop the attack.

---

## File Structure

```
PredX-Deauth/
├── src/
│   └── main.cpp          # Main firmware code
├── data/
│   └── index.html        # Web interface files
├── platformio.ini        # PlatformIO configuration
└── README.md             # Project documentation
```

---

## Web Interface

The web interface is designed to be user-friendly and responsive. It includes the following features:
- **Scan Networks**: Displays all nearby Wi-Fi networks.
- **Start/Stop Attack**: Buttons to control deauthentication attacks.
- **Status Updates**: Real-time feedback on the attack status.

---

## Troubleshooting

### 1. Failed to Open `index.html`
- Make sure `index.html` is inside the `data/` folder.
- Run `pio run --target uploadfs` to upload the web files to the ESP8266.

### 2. ESP8266 Fails to Start
- Ensure the ESP8266 is powered correctly.
- Check the serial logs for error messages.

### 3. Attack Not Working
- Make sure the `sendDeauth` function is properly implemented.
- Test the tool in a controlled environment with proper permissions.

---

## Contributions

Contributions are welcome! If you'd like to contribute, please:
1. Fork the repository.
2. Create a new branch for your feature/bugfix.
3. Submit a pull request.

---

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

---

## Author

- **Aryan4859** - [GitHub Profile](https://github.com/aryan4859)

Feel free to raise issues or feature requests in the [Issues Section](https://github.com/aryan4859/PredX-Deauth/issues).

---