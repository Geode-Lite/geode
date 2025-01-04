#include <Geode/DefaultInclude.hpp>
#include <Geode/modify/TOSPopup.hpp>

using namespace geode::prelude;

// this makes sure that the tos popup doesn't appear
class CustomTOSPopup;

struct CustomTOSPopup : Modify<CustomTOSPopup, TOSPopup> {
    GEODE_FORWARD_COMPAT_DISABLE_HOOKS("CustomTOSPopup hider")

    virtual bool init() {
        if (!TOSPopup::init()) return false;
        TOSPopup::keyBackClicked();
        return true;
    }
};

