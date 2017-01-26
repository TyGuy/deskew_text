# Deskew Text images (using Leptonica)
We just deskewed all over the place.

## Installation
```bash
brew install leptonica
git clone git@github.com:TyGuy/deskew_text.git
```

## Usage:
```bash
make clean && make
# ./deskew <filename_base>
# NOTE: .png file extension is assumed, don't add it
./deskew sample_image
```

## Dev:
real simple, just edit files, run make, then the executable.

## TODO:
Make usage easier:

* don't assume png, make it so you pass in full file name, and just ensure it's a raster image.
* print more helpful text
* let user customize it more (right now skew angle assumed between -45 and 45 degrees)

Other Ideas:
* de-warp
* use otsu threshold for binarization instead of

## External Resources:
* https://tpgit.github.io/Leptonica/skew_8c_source.html

## Contributors:
Jeff Rafter, Tyler Davis

## License:
MIT
