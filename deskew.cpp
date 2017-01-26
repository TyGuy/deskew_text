#include <leptonica/allheaders.h>
#include <string>

static const l_int32 DEFAULT_BINARY_THRESHOLD = 130;

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
double getLinesBitVariance(Pix *image) {
  l_int32 w, h, d, i, j;
  void **lines;

  lines = pixGetLinePtrs(image, NULL);

  pixGetDimensions(image, &w, &h, &d);
  double line_means[h];


  if (d == 1) {
    for (i = 0; i < h; i++) {  /* scan over image */
      double line_total = 0.0;

      for (j = 0; j < w; j++) {
        line_total += GET_DATA_BIT(lines[i], j);
      }
      line_means[i] = line_total / w;
    }

    return getVariance(line_means, h);

  } else {
    return -42.0;
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
  Pix *scaleImage = pixScale(binImage, 0.2, 0.2);

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
    double varMin = getLinesBitVariance(rotMin);
    pixDestroy(&rotMin);

    Pix *rotMax = pixRotate(scaleImage, deg2rad * maxAngle, L_ROTATE_AREA_MAP, L_BRING_IN_WHITE, 0, 0);
    double varMax = getLinesBitVariance(rotMax);
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

  pixWrite(output, newImage, IFF_PNG);
  printf("Output: %s\n", output);

  // destroy stuff
  pixDestroy(&image);
  pixDestroy(&otsuImage);
  pixDestroy(&binImage);
  pixDestroy(&scaleImage);
  pixDestroy(&newImage);
}
