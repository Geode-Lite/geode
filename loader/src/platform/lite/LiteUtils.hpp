#pragma once

#include <Geode/loader/Event.hpp>

namespace geode::lite {
  struct IsLiteEvent : geode::Event {
    IsLiteEvent() {}

    bool isLite = false;
  };

  inline bool isLite() {
      auto evt = IsLiteEvent();
      evt.post();
      return evt.isLite;
  }
}
