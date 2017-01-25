#include "graph_controller.h"

namespace Regression {

GraphController::GraphController(Responder * parentResponder, HeaderViewController * headerViewController, Store * store) :
  InteractiveCurveViewController(parentResponder, headerViewController, store, &m_view),
  m_bannerView(BannerView()),
  m_view(GraphView(store, &m_cursor, &m_bannerView, &m_cursorView)),
  m_store(store),
  m_initialisationParameterController(InitialisationParameterController(this, m_store)),
  m_predictionParameterController(PredictionParameterController(this, m_store, &m_cursor)),
  m_selectedDotIndex(-1)
{
  m_store->setCursor(&m_cursor);
}

ViewController * GraphController::initialisationParameterController() {
  return &m_initialisationParameterController;
}

bool GraphController::isEmpty() {
  if (m_store->numberOfPairs() < 2 || isinf(m_store->slope()) || isnan(m_store->slope())) {
    return true;
  }
  return false;
}

const char * GraphController::emptyMessage() {
  if (m_store->numberOfPairs() == 0) {
    return "Aucune donnee a tracer";
  }
  return "Pas assez de donnees pour un regression";
}

BannerView * GraphController::bannerView() {
  return &m_bannerView;
}

CurveView * GraphController::curveView() {
  return &m_view;
}

InteractiveCurveViewRange * GraphController::interactiveCurveViewRange() {
  return m_store;
}

bool GraphController::handleEnter() {
  if (m_selectedDotIndex == -1) {
    stackController()->push(&m_predictionParameterController);
    return true;
  }
  return false;
}

void GraphController::reloadBannerView() {
  m_bannerView.setLegendAtIndex((char *)"y = ax+b", 0);
  char buffer[k_maxNumberOfCharacters + Float::bufferSizeForFloatsWithPrecision(Constant::LargeNumberOfSignificantDigits)];
  const char * legend = "a = ";
  float slope = m_store->slope();
  int legendLength = strlen(legend);
  strlcpy(buffer, legend, legendLength+1);
  Float(slope).convertFloatToText(buffer+legendLength, Float::bufferSizeForFloatsWithPrecision(Constant::LargeNumberOfSignificantDigits), Constant::LargeNumberOfSignificantDigits);
  m_bannerView.setLegendAtIndex(buffer, 1);

  legend = "b = ";
  float yIntercept = m_store->yIntercept();
  legendLength = strlen(legend);
  strlcpy(buffer, legend, legendLength+1);
  Float(yIntercept).convertFloatToText(buffer+legendLength, Float::bufferSizeForFloatsWithPrecision(Constant::LargeNumberOfSignificantDigits), Constant::LargeNumberOfSignificantDigits);
  m_bannerView.setLegendAtIndex(buffer, 2);

  legend = "x = ";
  float x = m_cursor.x();
  // Display a specific legend if the mean dot is selected
  if (m_selectedDotIndex == m_store->numberOfPairs()) {
    legend = "x^ = ";
    x = m_store->meanOfColumn(0);
  }
  legendLength = strlen(legend);
  strlcpy(buffer, legend, legendLength+1);
  Float(x).convertFloatToText(buffer+legendLength, Float::bufferSizeForFloatsWithPrecision(Constant::LargeNumberOfSignificantDigits), Constant::LargeNumberOfSignificantDigits);
  m_bannerView.setLegendAtIndex(buffer, 3);

  legend = "y = ";
  float y = m_cursor.y();
  if (m_selectedDotIndex == m_store->numberOfPairs()) {
    legend = "y^ = ";
    y = m_store->meanOfColumn(1);
  }
  legendLength = strlen(legend);
  strlcpy(buffer, legend, legendLength+1);
  Float(y).convertFloatToText(buffer+legendLength, Float::bufferSizeForFloatsWithPrecision(Constant::LargeNumberOfSignificantDigits), Constant::LargeNumberOfSignificantDigits);
  m_bannerView.setLegendAtIndex(buffer, 4);

  m_bannerView.layoutSubviews();
}

void GraphController::initRangeParameters() {
  m_store->setDefault();
}

void GraphController::initCursorParameters() {
  float x = (m_store->xMin() + m_store->xMax())/2.0f;
  float y = m_store->yValueForXValue(x);
  m_store->moveCursorTo(x, y);
  m_selectedDotIndex = -1;
}

int GraphController::moveCursorHorizontally(int direction) {
  if (m_selectedDotIndex >= 0) {
    int dotSelected = m_store->nextDot(direction, m_selectedDotIndex);
    if (dotSelected >= 0 && dotSelected < m_store->numberOfPairs()) {
      m_selectedDotIndex = dotSelected;
      return m_store->moveCursorTo(m_store->get(0, m_selectedDotIndex), m_store->get(1, m_selectedDotIndex));
    }
    if (dotSelected == m_store->numberOfPairs()) {
      m_selectedDotIndex = dotSelected;
      return m_store->moveCursorTo(m_store->meanOfColumn(0), m_store->meanOfColumn(1));
    }
    return -1;
  } else {
    float x = direction > 0 ? m_cursor.x() + m_store->xGridUnit()/InteractiveCurveViewRange::k_numberOfCursorStepsInGradUnit :
      m_cursor.x() - m_store->xGridUnit()/InteractiveCurveViewRange::k_numberOfCursorStepsInGradUnit;
    float y = m_store->yValueForXValue(x);
    return m_store->moveCursorTo(x, y);
  }
}

int GraphController::moveCursorVertically(int direction) {
  float yRegressionCurve = m_store->yValueForXValue(m_cursor.x());
  if (m_selectedDotIndex >= 0) {
    if ((yRegressionCurve - m_cursor.y() > 0) == (direction > 0)) {
      m_selectedDotIndex = -1;
      return m_store->moveCursorTo(m_cursor.x(), yRegressionCurve);
    } else {
      return -1;
    }
  } else {
    int dotSelected = m_store->closestVerticalDot(direction, m_cursor.x());
    if (dotSelected >= 0 && dotSelected < m_store->numberOfPairs()) {
      m_selectedDotIndex = dotSelected;
      return m_store->moveCursorTo(m_store->get(0, m_selectedDotIndex), m_store->get(1, m_selectedDotIndex));
    }
    if (dotSelected == m_store->numberOfPairs()) {
      m_selectedDotIndex = dotSelected;
      return m_store->moveCursorTo(m_store->meanOfColumn(0), m_store->meanOfColumn(1));
    }
    return -1;
  }
}

uint32_t GraphController::modelVersion() {
  return m_store->storeChecksum();
}

uint32_t GraphController::rangeVersion() {
  return m_store->rangeChecksum();
}

}