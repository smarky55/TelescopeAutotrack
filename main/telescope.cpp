#include "telescope.hpp"

#include <TMCStepper.h>
#include <esp_timer.h>
#include <cmath>

constexpr int64_t SecToUS = 1'000'000;
constexpr int64_t tRef = 1'398'101;  // 2^24/fClk in us

constexpr int64_t fclk = 12'000'000;
constexpr int64_t ratio = 288;
constexpr int64_t microsteps = 256;
constexpr int64_t stepsPerRev = 200;

constexpr int64_t vActMult =
    ((1 << 24) * ratio * microsteps * stepsPerRev * 1000) / (fclk * 360);

Telescope::Telescope(TMC2209Stepper* raStepper, double raAccelleration)
    : m_raStepper(raStepper), m_raAccelleration(raAccelleration * SecToUS) {}

void Telescope::setTargetRASpeed(double degsPerSecond) {
  m_targetRAVelocity = degsPerSecond * SecToUS;
}

void Telescope::setTracking(bool tracking) {
  m_tracking = tracking;
}

void Telescope::setTrackRate(int64_t trackRate) {
  m_trackRate = trackRate;
}

void Telescope::tick() {
  static int64_t lastTime = esp_timer_get_time();
  int64_t currentTime = esp_timer_get_time();
  int64_t deltaTime = currentTime - lastTime;

  if (deltaTime == 0) {
    // Don't do anything if dt is 0
    return;
  }
  lastTime = currentTime;

  int64_t targetV = m_targetRAVelocity;
  if (m_tracking && m_targetRAVelocity == 0) {
    targetV = m_trackRate;
  }

  if (m_currentRAVelocity == targetV) {
    // Speed is right, do nothing
    return;
  }

  int64_t dV = (deltaTime * m_raAccelleration) / SecToUS;

  if (targetV < m_currentRAVelocity) {
    m_currentRAVelocity = std::max(m_currentRAVelocity - dV, targetV);
  } else {
    m_currentRAVelocity = std::min(m_currentRAVelocity + dV, targetV);
  }

  int64_t vActUs = (m_currentRAVelocity * vActMult) / 1000;
  printf(
      "vActUs %12lld vact %6lld deltaTime %10lld dV %10lld currentV %10lld "
      "targetV %10lld\n",
      vActUs, (int64_t)std::round(vActUs / 1000000.0), deltaTime, dV,
      m_currentRAVelocity, targetV);
  m_raStepper->VACTUAL((int64_t)std::round(vActUs / 1000000.0));
}
