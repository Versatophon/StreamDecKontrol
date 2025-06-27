# StreamDeck Control

## Prerequisties

Install LibUSB/LibHidRaw
```bash
sudo apt install libhidapi-hidraw0 libhidapi-libusb0
```

Install SDL3
```bash
sudo apt install SDL3-dev
```

Install TurboJpeg
```bash
sudo apt install libturbojpeg-dev
```

Install FreeImage
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
