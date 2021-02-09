#ifndef __TELESCOPE_H__
#define __TELESCOPE_H__

#include <cstdint>

class TMC2209Stepper;

class Telescope {
 public:
  Telescope(TMC2209Stepper* raStepper, double raAccelleration);
  ~Telescope() = default;

  void setTargetRASpeed(double degsPerSecond);

  void setTracking(bool tracking);
  void setTrackRate(int64_t trackRate);

  void tick();

 private:
  TMC2209Stepper* m_raStepper;
  int64_t m_raAccelleration;
  int64_t m_targetRAVelocity = 0;
  int64_t m_currentRAVelocity = 0;
  int64_t m_trackRate = 4167;
  bool m_tracking = false;
};

#endif  // __TELESCOPE_H__