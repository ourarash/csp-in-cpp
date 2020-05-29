#include "csp.h"

#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <optional>
#include <thread>
// const std::optional<std::string>& param = std::nullopt
void Channel::Write(int data) {
  std::unique_lock<std::mutex> ul(_mutex);
  _req = !_req;
  _data = data;
  _status = ChannelStatus::w_pend;
  ul.unlock();
  _cl.notify_all();
  ul.lock();

  _cl.wait(ul, [=]() { return _ack != _prev_ack; });
  _status = ChannelStatus::idle;
  // _handshake_phase = !_handshake_phase;
  _prev_ack = _ack;
}

int Channel::Read() {
  std::unique_lock<std::mutex> ul(_mutex);
  _status = ChannelStatus::r_pend;
  // if blocked, ul.unlock() is automatically called.
  _cl.wait(ul, [=]() { return _req != _prev_req; });
  // if unblocks, ul.lock() is automatically called
  int data = _data;

  if (_receive_counter == _number_of_receivers - 1) {
    _ack = !_ack;
    _status = ChannelStatus::idle;
    _receive_counter = 0;
    _prev_req = _req;
    ul.unlock();
    _cl.notify_all();
    ul.lock();
  } else {
    _status = ChannelStatus::s12m_pend;
    _receive_counter++;
    _cl.wait(ul, [=]() { return _receive_counter == 0; });
  }
  return data;
}
