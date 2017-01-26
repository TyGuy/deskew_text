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
  l_float32 deg2rad = deg2rad = 3.1415926535 / 180.;
  char *outText;

  std::string infileBase = argv[1];
  std::string infile = infileBase + ".png";
  const char *inputfile = infile.c_str();
  Pix *image = pixRead(inputfile);

  double pangle;
  double pconf;

  // Find skew and deskew
  // Pix *newImage = pixFindSkewAndDeskew(image, 0, &pangle, &pconf);

  // Find skew angle
  Pix *otsuImage = NULL;
  image = pixConvertRGBToGray(image, 0.0f, 0.0f, 0.0f);
  l_int32 status = pixOtsuAdaptiveThreshold(image, 2000, 2000, 0, 0, 0.0f, NULL, &otsuImage);
  Pix *binImage = pixConvertTo1(otsuImage, DEFAULT_BINARY_THRESHOLD);
  const char * outfile0 = (infileBase + "_leptbin.png").c_str();

  double maxVariance = -1.0;

  pixWrite(outfile0, binImage, IFF_PNG);

  for(int i = -45; i < 46; i++) {
    Pix *rot = pixRotate(binImage, deg2rad * i, L_ROTATE_AREA_MAP, L_BRING_IN_WHITE, 0, 0);

    double variance = getLinesBitVariance(rot);
    if (variance > maxVariance) {
      pangle = i;
      maxVariance = variance;
    }
    // printf("variance: %f\n", variance);
    // printf("degrees: %d\n", i);

    pixDestroy(&rot);
  }

  printf("angle: %f\n", pangle);
  Pix *newImage = pixRotate(binImage, deg2rad * pangle, L_ROTATE_AREA_MAP, L_BRING_IN_WHITE, 0, 0);

  const char * outfile1 = (infileBase + "_deskew.png").c_str();
  pixWrite(outfile1, newImage, IFF_PNG);

  // destroy stuff
  pixDestroy(&image);
  pixDestroy(&newImage);
  pixDestroy(&binImage);
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
//   l_float32 pangle;
//   l_float32 pconf;
//
//   // Find skew and deskew
//   // Pix *newImage = pixFindSkewAndDeskew(image, 0, &pangle, &pconf);
//
//   // Find skew angle
//   Pix *binImage = pixConvertTo1(image, DEFAULT_BINARY_THRESHOLD);
//   const char * outfile0 = (infileBase + "_leptbin.png").c_str();
//   pixWrite(outfile0, binImage, IFF_PNG);
//   pixFindSkew(binImage, &pangle, &pconf);
//   double variance = getLinesBitVariance(binImage);
//   printf("variance: %f\n", variance);
//   Pix *newImage = pixRotate(binImage, deg2rad * pangle, L_ROTATE_AREA_MAP, L_BRING_IN_WHITE, 0, 0);
//
//   printf("angle: %f\n", pangle);
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
