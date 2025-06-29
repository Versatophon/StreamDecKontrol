# StreamDeck Control

## Prerequisties

Install LibUSB/LibHidRaw (for device communication)
```bash
sudo apt install libhidapi-hidraw0 libhidapi-libusb0
```

Install SDL3 (for gui display)
```bash
sudo apt install SDL3-dev
```

Install TurboJpeg (for jpeg compression decompresssion transformation)
```bash
sudo apt install libturbojpeg-dev
```

Install FreeImage (for other image formats: png, gif, ...)
```bash
sudo apt install libfreeimage-dev
```

## Build

Open a terminal in `prj` directory, then call:
```bash
make StreamDecKontrol
```

## Launch

Open a terminal in `bin` directory, then call:
```bash
.\StreamDecKontrol
```
