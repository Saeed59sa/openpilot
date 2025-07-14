# Integration with Stock Features

In all supported cars:
* Stock Lane Keep Assist (LKA) and stock ALC are replaced by openpilot ALC, which only functions when openpilot is engaged by the user.
* Stock LDW is replaced by openpilot LDW.

Additionally, on specific supported cars (see ACC column in [supported cars](CARS.md)):
* Stock ACC is replaced by openpilot ACC.
* openpilot FCW operates in addition to stock FCW.

openpilot should preserve all other vehicle's stock features, including, but not limited to: FCW, Automatic Emergency Braking (AEB), auto high-beam, blind spot warning, and side collision warning.

## Automatic Car Detection and Harness Orientation

openpilot identifies your vehicle automatically using CAN messages and firmware data, a process known as fingerprinting. When the device boots, it listens on the CAN bus and matches the observed messages against known vehicle profiles. If your car is not recognized you may see a "Car Unrecognized" alert and openpilot will run in dashcam mode.

The harness box also detects its orientation on startup. If the orientation cannot be determined (`HarnessStatus.notConnected`), an alert will appear instructing you to check the harness connection. This detection ensures the CAN lines are routed correctly. If you encounter repeated failures, verify the harness is fully seated and not flipped.
