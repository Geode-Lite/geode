#include "LiteUtils.hpp"

using namespace geode::prelude;
using namespace geode::lite;

$on_mod(Loaded) {
    new EventListener<EventFilter<IsLiteEvent>>(+[](IsLiteEvent* e) {
        e->isLite = true;
        return ListenerResult::Stop;
    });
}
