# Photo Booth

`Photo Booth` is a C++20 project for IMGS.361 Image Processing. The application uses OpenCV to acquire live camera frames, apply image-processing operations, display the processed preview, and save captured images.

The project builds on the earlier semester work while keeping the existing application structure. Camera acquisition, configuration, image processing, and application control remain separate.

## Applications

The project builds three executables:

- `live_preview` — displays the live camera feed.
- `photo_booth` — runs the interactive image-processing pipeline.
- `capture_single_image` — captures a single image for testing.

The photo booth implements ten substantive image-processing capabilities:

1. **Channel swapping** — swaps the red and blue channels.
2. **Dynamic contrast enhancement** — stretches each channel using percentile-based low/high values.
3. **Grayscale conversion** — converts the image to grayscale while preserving the project's three-channel BGR representation.
4. **Gaussian blur** — applies a configurable odd-sized Gaussian spatial filter.
5. **Canny edge detection** — detects image edges and converts the result back to three-channel BGR.
6. **Image inversion** — computes the photographic negative of the image.
7. **Image quantization** — reduces each channel to a selectable number of intensity levels.
8. **Arbitrary-angle rotation** — rotates the image using inverse coordinate mapping.
9. **Harris and FAST corner detection** — detects image corners using either the Harris corner response or the FAST feature detector and displays detected locations as a red overlay.
10. **Minimum-distance and cosine classification** — represents images with normalized color histograms and compares the live image against reference-image classes using either Euclidean distance or cosine similarity.


The two additional Week 10 processing functions are Gaussian blur and Canny edge detection.

## Keyboard controls

Click the preview window to ensure it has focus, then use the following keys.

| Key | Action |
|---|---|
| `C` | Toggle dynamic contrast |
| `I` | Toggle image inversion |
| `N` | Toggle quantization |
| `[` | Decrease quantization levels |
| `]` | Increase quantization levels |
| `R` | Toggle rotation |
| `,` | Decrease rotation angle by 5 degrees |
| `.` | Increase rotation angle by 5 degrees |
| `B` | Toggle Gaussian blur |
| `E` | Toggle Canny edge detection |
| `SPACE` | Save the currently processed frame |
| `Q` or `Esc` | Quit |

Quantization levels are bounded between 2 and 256. Rotation is bounded between -180 and 180 degrees.

## Processing pipeline

The application processes each camera frame in this order:

1. Channel swapping, if enabled in the configuration.
2. Dynamic contrast, if enabled.
3. Inversion, if enabled.
4. Gaussian blur, if enabled.
5. Canny edge detection, if enabled.
6. Quantization, if enabled.
7. Rotation, if enabled.

The operations can be combined. Since each enabled operation receives the previous operation's output, changing the enabled effects can change the final image.

Canny edge detection converts the image to grayscale internally, detects edges, and converts the result back to three-channel BGR. This keeps it compatible with the existing image-processing pipeline.

## Project organization

```text
Photo-Booth-Week-10/
├── CMakeLists.txt
├── config.toml
├── README.md
├── apps/
│   ├── capture_single_image.cpp
│   ├── live_preview.cpp
│   └── photo_booth.cpp
├── include/
│   └── photo_booth/
│       ├── AppConfig.hpp
│       ├── ImageCapture.hpp
│       └── ImageProcessing.hpp
└── src/
    ├── AppConfig.cpp
    ├── ImageCapture.cpp
    └── ImageProcessing.cpp
```

`ImageProcessing.cpp` contains reusable image-processing functions. `photo_booth.cpp` controls the processing order, keyboard input, preview, and image capture.

## Requirements

- CMake 3.30 or later
- A C++20 compiler
- OpenCV with the required `core`, `videoio`, `highgui`, `imgcodecs`, and `imgproc` modules
- Boost with `program_options`
- Eigen3
- toml++ 3.4.0

The project's CMake configuration downloads toml++ using `FetchContent`.

## Build

Run these commands from the project root:

```bash
cmake -S . -B build
cmake --build build
```

## Run

Run the photo booth from the project root:

```bash
./build/bin/photo_booth
```

To use a different TOML configuration file:

```bash
./build/bin/photo_booth webcam.toml
```

On Windows, run the corresponding executable from `build/bin`, such as `photo_booth.exe`.

## Capture

Press `SPACE` to save the currently processed frame using the output filename specified in `config.toml`. The saved image contains the processing effects, not the text drawn over the preview window.

## Design approach

The project continues to use ordinary image-processing functions instead of introducing a new processing framework.

To add an operation:

1. Declare the function in `include/photo_booth/ImageProcessing.hpp`.
2. Implement it in `src/ImageProcessing.cpp`.
3. Add an enable flag to `ProcessingState`.
4. Call the operation from `processFrame()`.
5. Add a keyboard case in `handleKey()`.
6. Update the preview text and README controls.


Camera properties such as image dimensions, frame rate, and FOURCC are requests rather than guarantees. The hardware, operating system, driver, and OpenCV backend may ignore a requested property or select a nearby supported mode. Use `cameraInfo()` after opening the device to inspect the values reported by the backend.

## Suggested evolution during the course

The contrast between `live_preview` and `photo_booth` provides a simple starting lesson in application structure. `live_preview` demonstrates direct use of an acquired frame. `photo_booth` demonstrates how a growing application benefits from an explicit processing pipeline.

As capabilities accumulate, students should continue adding simple processing functions and explicit configuration/runtime state. `processFrame()` and `handleKey()` are expected to become somewhat more crowded as the project grows; that pressure can provide a concrete reason to refactor toward additional classes or modules when those abstractions become useful. Potential additions include quantization, dynamic contrast enhancement, histogram operations, spatial and frequency-domain filtering, sharpening, geometric transformations, perspective correction, artistic filters, feature detection, segmentation, and capture/review behavior.

## License

This project is licensed under the GNU General Public License v3.0. See `LICENSE` for details.

## Contact

### Author

Carl Salvaggio, Ph.D.  
Professor of Imaging Science  
Director, Digital Imaging and Remote Sensing (DIRS) Laboratory

### E-mail

carl.salvaggio@rit.edu

### Organization

Chester F. Carlson Center for Imaging Science  
Rochester Institute of Technology  
Rochester, New York, 14623  
United States
