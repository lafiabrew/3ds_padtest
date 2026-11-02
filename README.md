# PadTest (`3ds_padtest`)

> **Stick Drift & Screen Tester** — Nintendo 3DS Homebrew (`CTR-P-PADT`, UniqueId `0x0D3E7`) by [**lafiabrew**](https://github.com/orgs/lafiabrew/repositories)

Nintendo 3DS Circle Pad drift & deadzone analyzer, button tester, dead-pixel screen pattern cycler, and JSON report exporter.

## Build & Verify

```bash
# Run host C11 ASan/UBSan unit & UI simulation tests
make test

# Build ARM11 3DS .elf, .smdh, .3dsx, and .cia (via devkitARM or rootless Podman container)
make 3ds && make verify-3ds

# Serve FBI QR code or push .cia over LAN
make qr
```
