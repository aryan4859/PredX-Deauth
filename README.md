# PredX Guardian

PredX Guardian is an ESP32-based, passive Wi-Fi security monitor. It observes management frames on one configured channel, logs deauthentication and disassociation activity, and flags access points advertising the same SSID with different security. It does not transmit deauthentication frames.

## Features

- **Passive frame monitoring**: Count deauthentication and disassociation frames on the configured channel.
- **SSID security mismatch alerts**: Identify same-name access points that advertise different security modes; verify alerts against a trusted inventory.
- **Bounded event log**: Keep the latest 32 events in RAM, with burst aggregation and a clear-log control.
- **Responsive dashboard and JSON API**: View observed APs, signal strength, alerts, and monitor status.
- **Safe simulator**: Add a clearly labeled sample alert without sending any radio frames.
- **Fixed-channel operation**: The monitoring access point and packet monitor use channel 6 by default; other channels are not monitored.

## Requirements

- An ESP32 development board supported by PlatformIO's `esp32dev` target.
- PlatformIO Core or the PlatformIO IDE extension.

## Build and flash

```shell
pio run
pio run --target upload
```

The dashboard is served directly from firmware; no filesystem upload is required.

## Usage

1. Connect to the board's `PredX-Guardian` access point (default password: `predx1337`; change it in `lib/definitions.h` before deployment).
2. Open `http://192.168.4.1`.
3. The dashboard reports management frames and beacon/probe-response observations on channel 6.
4. Use **Add simulated event** to preview the event log. Simulated entries are labeled and do not represent captured traffic.
5. Use **Clear log** to clear the in-memory event history.

The read-only JSON state is available at `/api/state`. Event history and AP observations are volatile and reset when the board reboots. The monitor stays on a single channel to keep its local dashboard available.

SSID/security mismatches and bursts of management frames are indicators for investigation, not proof of malicious activity. Legitimate roaming, mesh networks, and normal client behavior can produce similar observations. This monitor does not validate protected management frames (802.11w/PMF) and is not a replacement for router/AP telemetry.

## Troubleshooting

### Dashboard is not reachable

- Connect to the board's `PredX-Guardian` access point and browse to `http://192.168.4.1`.
- Check the serial monitor for access-point or monitor initialization errors.

### No access points or events appear

- The monitor observes only channel 6 by default. Change `MONITOR_CHANNEL` in `lib/definitions.h` and rebuild to observe another channel.
- Keep the dashboard client connected to the board's access point while monitoring.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
