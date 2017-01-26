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
  // const char * outfile0 = (infileBase + "_leptbin.png").c_str();
  // pixWrite(outfile0, binImage, IFF_PNG);

  // Brute force, rotate and check variance from -45 deg to 45 deg
  l_float32 deg2rad = deg2rad = 3.1415926535 / 180.;
  double angle = 0;
  double maxVariance = -1.0;
  double min = -45;
  double max = 45;
  double accuracy = 1;
  while (max > min + accuracy) {
    double minAngle = angle - ((angle - min) / 2);
    double maxAngle = max - ((max - angle ) / 2);

    Pix *rotMin = pixRotate(binImage, deg2rad * minAngle, L_ROTATE_AREA_MAP, L_BRING_IN_WHITE, 0, 0);
    double varMin = getLinesBitVariance(rotMin);
    pixDestroy(&rotMin);

    Pix *rotMax = pixRotate(binImage, deg2rad * maxAngle, L_ROTATE_AREA_MAP, L_BRING_IN_WHITE, 0, 0);
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

  /*
  // Get every mean in the range
  for(int i = -45; i < 46; i++) {
    Pix *rot = pixRotate(binImage, deg2rad * i, L_ROTATE_AREA_MAP, L_BRING_IN_WHITE, 0, 0);

    double variance = getLinesBitVariance(rot);
    if (variance > maxVariance) {
      angle = i;
      maxVariance = variance;
    }
    printf("degrees: %d; variance: %f\n", i, variance);
    pixDestroy(&rot);
  }
  */

  // We found the rotation with the most variance
  printf("\n----------------\n");
  printf("Angle: %f\n", angle);
  Pix *newImage = pixRotate(binImage, deg2rad * angle, L_ROTATE_AREA_MAP, L_BRING_IN_WHITE, 0, 0);

  // const char * outfile1 = (infileBase + "_deskew.png").c_str();
  pixWrite(output, newImage, IFF_PNG);
  printf("Output: %s\n", output);

  // destroy stuff
  pixDestroy(&image);
  pixDestroy(&otsuImage);
  pixDestroy(&binImage);
  pixDestroy(&newImage);
}


// int main(int argc, char *argv[]) {
//   l_float32 deg2rad = deg2rad = 3.1415926535 / 180.;
//   char *outText;
//   // const char *inputfile = "010001.bin.png";
//   // const char *inputfile = "IMG_2019.JPG";
//   std::string infileBase = argv[1];
//   std::string infile = infileBase + ".png";
//   const char *inputfile = infile.c_str();
//   Pix *image = pixRead(inputfile);
//
//   l_float32 angle;
//   l_float32 pconf;
//
//   // Find skew and deskew
//   // Pix *newImage = pixFindSkewAndDeskew(image, 0, &angle, &pconf);
//
//   // Find skew angle
//   Pix *binImage = pixConvertTo1(image, DEFAULT_BINARY_THRESHOLD);
//   const char * outfile0 = (infileBase + "_leptbin.png").c_str();
//   pixWrite(outfile0, binImage, IFF_PNG);
//   pixFindSkew(binImage, &angle, &pconf);
//   double variance = getLinesBitVariance(binImage);
//   printf("variance: %f\n", variance);
//   Pix *newImage = pixRotate(binImage, deg2rad * angle, L_ROTATE_AREA_MAP, L_BRING_IN_WHITE, 0, 0);
//
//   printf("angle: %f\n", angle);
//   printf("confidence: %f\n", pconf);
//
//   const char * outfile1 = (infileBase + "_deskew.png").c_str();
//   pixWrite(outfile1, newImage, IFF_PNG);
//
//   // destroy stuff
//   pixDestroy(&image);
//   pixDestroy(&newImage);
//   pixDestroy(&binImage);
// }
