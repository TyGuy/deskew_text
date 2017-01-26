#include <stdio.h>
#include <string>
#include <leptonica/allheaders.h>

static const l_int32 DEFAULT_BINARY_THRESHOLD = 130;
static const l_int32 MIN_SEGMENT_HEIGHT = 3;

double getMean(double *data, int size) {
  double sum = 0.0;

  for(int i = 0; i < size; i++) {
    double a = data[i];
    sum += a;
  }

  return sum/size;
 }

double getVariance(double *data, int size) {
  double mean = getMean(data, size);
  double temp = 0;

  for(int i = 0; i < size; i++) {
    double a = data[i];
    temp += (a-mean)*(a-mean);
  }
  return temp/size;
}

// could use better naming
double getLinesBitVariance(Pix *image, l_int32 w, l_int32 h) {
  l_int32 i, j;
  double line_means[h];
  void **lines;
  lines = pixGetLinePtrs(image, NULL);

  for (i = 0; i < h; i++) {  /* scan over image */
    double line_total = 0.0;

    for (j = 0; j < w; j++) {
      line_total += GET_DATA_BIT(lines[i], j);
    }
    line_means[i] = line_total / w;
  }

  return getVariance(line_means, h);
}

void printSegments(Pix *image, const char * output, l_int32 w, l_int32 h) {
  l_int32 i, j, count = 0;
  void **lines;

  lines = pixGetLinePtrs(image, NULL);

  int top = -1;

  for (i = 0; i < h; i++) {  /* scan over image */
    int black = 0;

    for (j = 0; j < w; j++) {
      if (GET_DATA_BIT(lines[i], j) > 0) {
        black = 1;
        break;
      }
    }

    if (black == 0) {
      // If we have a segment running, we need to end it
      if (top >= 0 && i - top >= MIN_SEGMENT_HEIGHT) {
        count += 1;
        printf("\nSegment (top:%d, height:%d)", top, (i - 1) - top);

        // change extension
        std::string segment = output;
        size_t lastindex = segment.find_last_of(".");
        std::string baseName = segment.substr(0, lastindex);
        std::string segFileName = baseName + "-" + std::to_string(count) + ".png";
        Box *cropBox = boxCreate(0, top, w, (i - 1) - top);
        Pix *cropImage = pixClipRectangle(image, cropBox, NULL);
        pixWrite(segFileName.c_str(), cropImage, IFF_PNG);
        boxDestroy(&cropBox);
        pixDestroy(&cropImage);
        top = -1;

      }
    } else {
      // If we are in white space, start a new segment
      if (top == -1) {
        top = i;
      }
    }
  }
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("./deskew <input.png> <output.png>\n");
    return 1;
  }

  // Read input image
  const char *input = argv[1];
  const char *output = argv[2];
  Pix *image = pixRead(input);

  // Binarize using Otsu Threshold
  Pix *otsuImage = NULL;
  image = pixConvertRGBToGray(image, 0.0f, 0.0f, 0.0f);
  l_int32 status = pixOtsuAdaptiveThreshold(image, 2000, 2000, 0, 0, 0.0f, NULL, &otsuImage);
  Pix *binImage = pixConvertTo1(otsuImage, DEFAULT_BINARY_THRESHOLD);
  // TODO: add in a connected component analysis to despeckle/denoise before this step
  Pix *scaleImage = pixScale(binImage, 0.2, 0.2);

  l_int32 w, h, d;
  pixGetDimensions(scaleImage, &w, &h, &d);

  // Brute force, rotate and check variance from -45 deg to 45 deg
  l_float32 deg2rad = deg2rad = 3.1415926535 / 180.;
  double angle = 0;
  double min = -45;
  double max = 45;
  double accuracy = 1; // Fine tune the final angle accuracy
  while (max > min + accuracy) {
    double minAngle = angle - ((angle - min) / 2);
    double maxAngle = max - ((max - angle ) / 2);

    Pix *rotMin = pixRotate(scaleImage, deg2rad * minAngle, L_ROTATE_AREA_MAP, L_BRING_IN_WHITE, 0, 0);
    double varMin = getLinesBitVariance(rotMin, w, h);
    pixDestroy(&rotMin);

    Pix *rotMax = pixRotate(scaleImage, deg2rad * maxAngle, L_ROTATE_AREA_MAP, L_BRING_IN_WHITE, 0, 0);
    double varMax = getLinesBitVariance(rotMax, w, h);
    pixDestroy(&rotMax);

    if (varMin > varMax) {
      max = angle;
    } else if (varMax > varMin) {
      min = angle;
    } else {
      break;
    }

    printf("\ndegrees: %f, varMin: %f, varMax: %f\n", angle, varMin, varMax);
    angle = max - ((max - min) / 2);
  }

  // We found the rotation with the most variance
  printf("\n----------------\n");
  printf("Angle: %f\n", angle);

  // TODO: could save this rotation by using the final angle in the variance lookup
  Pix *newImage = pixRotate(binImage, deg2rad * angle, L_ROTATE_AREA_MAP, L_BRING_IN_WHITE, 0, 0);
  pixGetDimensions(newImage, &w, &h, &d);
  // TODO: could run a gaussian filter before this step
  printSegments(newImage, output, w, h);

  pixWrite(output, newImage, IFF_PNG);
  printf("\nOutput: %s\n", output);

  // destroy stuff
  pixDestroy(&image);
  pixDestroy(&otsuImage);
  pixDestroy(&binImage);
  pixDestroy(&scaleImage);
  pixDestroy(&newImage);
}
