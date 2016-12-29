#include "multisense_image_utils.hpp"

bool multisense_image_utils::
removeSpeckles(cv::Mat& ioImage, const double iMaxDisparityDiff,
               const int iMaxBlobSize) {
  cv::Mat img(ioImage.rows, ioImage.cols, CV_16SC1, ioImage.data);
  cv::filterSpeckles(img, 0, iMaxBlobSize, iMaxDisparityDiff);
  return true;
}

bool multisense_image_utils::
removeSmall(cv::Mat& ioImage, const uint16_t iValueThresh,
            const int iSizeThresh) {
  const int w = ioImage.cols;
  const int h = ioImage.rows;

  // allocate output labels image
  std::vector<int> labelImage(w*h);
  std::fill(labelImage.begin(), labelImage.end(), 0);

  // allocate equivalences table
  std::vector<int> equivalences(w*h);
  std::fill(equivalences.begin(), equivalences.end(), 0);

  // utility function
  auto collapse = [&](const int iIndex) {
    int out = iIndex;
    while (equivalences[out] != out) out = equivalences[out];
    return out;
  };

  int curLabel = 0;

  // loop over image pixels
  for (int i = 0; i < h; ++i) {
    int* labels = labelImage.data() + i*w;
    const uint16_t* im = ioImage.ptr<uint16_t>(i);
    for (int j = 0; j < w; ++j, ++im, ++labels) {

      // ignore pixels below thresh
      if (*im <= iValueThresh) continue;

      // look at neighbors (for 4-connectedness)
      int labelAbove = (i>0) ? collapse(labels[-w]) : 0;
      int labelLeft = (j>0) ? collapse(labels[-1]) : 0;

      if (labelAbove>0) {
        *labels = labelAbove;
        if ((labelLeft>0) && (labelAbove!=labelLeft)) {
          equivalences[labelLeft] = labelAbove;
        }
      }
      else {
        if (labelLeft>0) {
          *labels = labelLeft;
        }
        else {
          ++curLabel;
          *labels = curLabel;
          equivalences[curLabel] = curLabel;
        }
      }
    }
  }

  // collapse label equivalences
  ++curLabel;
  for (int i = 0; i < curLabel; ++i) {
    equivalences[i] = collapse(i);
  }

  // remap labels image and count connected components
  std::vector<int> counts(curLabel);
  std::fill(counts.begin(), counts.end(), 0);
  for (int i = 0; i < w*h; ++i) {
    int& label = labelImage[i];
    label = equivalences[label];
    ++counts[label];
  }

  // remap once more
  std::vector<uint8_t> valid(counts.size());
  for (int i = 0; i < curLabel; ++i) {
    valid[i] = (counts[i] < iSizeThresh) ? 0 : 1;
  }
  const int* labels = labelImage.data();
  for (int i = 0; i < h; ++i) {
    uint16_t* im = ioImage.ptr<uint16_t>(i);
    for (int j = 0; j < w; ++j, ++im, ++labels) {
      if (valid[*labels] == 0) *im = 0;
    }
  }

  return true;
}

float multisense_image_utils::computeIntensity(unsigned char * rgb, int row, int col, int width) {

  return 0.2126 * rgb[3 * (row * width + col) + 0] + 0.7152 * rgb[3 * (row * width + col) + 1] + 0.0722 * rgb[3 * (row * width + col) + 2];
}

void multisense_image_utils::filterLowTexture(unsigned short * disparity, unsigned char * rgb, int width, int height, int windowSize, double threshold, bool removeHorizontalEdges) {

  for (int i=windowSize; i<height-windowSize; i++) {
    for (int j=windowSize; j<width-windowSize; j++) {

      float tl = computeIntensity(rgb, i-windowSize, j-windowSize, width);
      float br = computeIntensity(rgb, i+windowSize, j+windowSize, width);

      float tr = computeIntensity(rgb, i-windowSize, j+windowSize, width);
      float bl = computeIntensity(rgb, i+windowSize, j-windowSize, width);

      float tv = computeIntensity(rgb, i-windowSize, j, width);
      float bv = computeIntensity(rgb, i+windowSize, j, width);

      float lh = computeIntensity(rgb, i, j-windowSize, width);
      float rh = computeIntensity(rgb, i, j+windowSize, width);

      float d1 = fabs(tl - br);
      float d2 = fabs(tr - bl);
      float d3 = fabs(tv - bv);
      float d4 = fabs(lh - rh);

      if (d1 < threshold && d2 < threshold && d3 < threshold && d4 < threshold) {
        disparity[i * width + j] = 0;
      }

      float cos = d3 / sqrt(d4*d4 + d3*d3);

      if (removeHorizontalEdges && cos >= 0.99) {
        disparity[i * width + j] = 0;
      }
    }
  }
}

void multisense_image_utils::sobelEdgeFilter(unsigned short *  disparity, unsigned char * rgb, int width, int height, int windowSize, double threshold, bool removeHorizontalEdges) {

  for (int i=windowSize; i<height-windowSize; i++) {
    for (int j=windowSize; j<width-windowSize; j++) {

      float tl = computeIntensity(rgb, i-windowSize, j-windowSize, width);
      float br = computeIntensity(rgb, i+windowSize, j+windowSize, width);

      float tr = computeIntensity(rgb, i-windowSize, j+windowSize, width);
      float bl = computeIntensity(rgb, i+windowSize, j-windowSize, width);

      float tv = computeIntensity(rgb, i-windowSize, j, width);
      float bv = computeIntensity(rgb, i+windowSize, j, width);

      float lh = computeIntensity(rgb, i, j-windowSize, width);
      float rh = computeIntensity(rgb, i, j+windowSize, width);

      float gx = -1 * tl + 1 * tr - 2 * lh + 2 * rh - 1 * bl + 1 * br;
      float gy = -1 * tl + 1 * bl - 2 * tv + 2 * bv - 1 * tr + 1 * br;

      float g = sqrt(gx*gx + gy*gy);

      if (g<threshold) {
        disparity[i * width + j] = 0;
      }

      float d3 = fabs(tv - bv);
      float d4 = fabs(lh - rh);
      float cos = d3 / sqrt(d4*d4 + d3*d3);

      if (removeHorizontalEdges && cos >= 0.99) {
        disparity[i * width + j] = 0;
      }
    }
  }
}
