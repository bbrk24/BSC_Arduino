#pragma once

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 5
#endif

class Buffer {
public:
  constexpr Buffer() noexcept : m_data{0}, m_lastIndex(0) {}

  void addPoint(float value) noexcept {
    m_data[m_lastIndex] = value;
    m_lastIndex = (m_lastIndex + 1) % BUFFER_SIZE;
  }

  bool isDecreasing() const noexcept {
    int firstIndex = (m_lastIndex + 1) % BUFFER_SIZE;
    int prevIndex = firstIndex;
    for (int i = 1; i < BUFFER_SIZE; ++i) {
      int nextIndex = (firstIndex + i) % BUFFER_SIZE;
      if (m_data[prevIndex] > m_data[nextIndex]) {
        return false;
      }
      prevIndex = nextIndex;
    }
    return true;
  }
private:
  float m_data[BUFFER_SIZE];
  int m_lastIndex;
};
