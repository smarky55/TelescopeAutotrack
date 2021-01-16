#ifndef _INCLUDE_STEPPER_H
#define _INCLUDE_STEPPER_H

#include <stdint.h>

void stepperTask(void* params);

enum class StepperInstruction {
  SetSpeed,
  SetAccelleration,
  SetDirection,
  SetJerk
};

struct StepperMessage {
  StepperMessage() {}
  StepperMessage(StepperInstruction _inst, int32_t _iVal)
      : inst(_inst), iVal(_iVal) {}
  StepperMessage(StepperInstruction _inst, float _fVal)
      : inst(_inst), fVal(_fVal) {}
  StepperInstruction inst;
  int32_t iVal;
  float fVal;
};

class StepperState {
 public:
  StepperState() = default;
  ~StepperState() = default;

  void setSpeed(float speed);

  void setAccelleration(float acc);

  void setDirection(int32_t dir);
  int8_t direction() const;

  void setJerk(float jerk);

  uint64_t getNextTimeout(uint64_t deltaT);

  bool sleep() const;

 private:
  float m_targetSpeed;      // Steps per s
  float m_currentVelocity;  // Steps per s
  float m_accelleration;    // Steps per s^2
  int8_t m_direction;       // -1, 0, 1
  float m_jerk;             // Steps per s
};

template <typename T>
class Queue;

class StepperController {
 public:
  StepperController(Queue<StepperMessage>*);
  ~StepperController() = default;

  // Set speed in steps per second
  void setSpeed(float speed);

  // Set accelleration in steps per second^2
  void setAccelleration(float acc);

  void setDirection(int8_t dir);

  // Max instantaneous change in speed
  // e.g if (|targetSpeed - currentSpeed| < jerk) currentSpeed = targetSpeed
  void setJerk(float jerk);

 private:
  Queue<StepperMessage>* m_queue;
};

#endif  // _INCLUDE_STEPPER_H