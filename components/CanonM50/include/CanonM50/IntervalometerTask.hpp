#ifndef __INTERVALOMETERTASK_H__
#define __INTERVALOMETERTASK_H__

#include <CanonM50/CanonM50.hpp>

#include <atomic>

using TaskHandle_t = void*;

class IntervalometerTask {
  /// Static task function. Arg should be ptr to instance of this class.
  static void s_intervalometerTask(void* arg);

 public:
  IntervalometerTask();
  ~IntervalometerTask();

  /**
   * Begin task execution.
   *
   * @param stackDepth Stack size for the created task in bytes
   * @param priority Task priority
   * @return true Task has been started successfully.
   * @return false Failed to start task or task already running.
   */
  bool begin(uint32_t stackDepth, uint32_t priority);

  /// End task.
  void end();

  /**
   * Begin a new sequence of captures with the provided camera.
   * Currently requires the camera to be set to the requested exposure length.
   * Bulb operation not implemented. Aborts the current sequence if in progress.
   *
   * @param camera Camera object to trigger image capture.
   * @param imageDuration Exposure time for capture.
   * @param interval Time to wait after capture ends before beginning the next.
   * @param count Number of captures in this sequence.
   */
  void runSequence(CanonM50* camera,
                   float imageDuration,
                   float interval,
                   int count);

  /// Abort the currently running sequence.
  void abortSequence();

  /// Is there a sequence in progress.
  bool inSequence() const;

  /// Remaining captures in current sequence.
  int remainingCount() const;

 private:
  /// Function executed in task loop.
  void doTask();

  TaskHandle_t m_taskHandle;

  CanonM50* m_camera;
  std::atomic<float> m_waitMs;
  std::atomic<int> m_count;
};

#endif  // __INTERVALOMETERTASK_H__