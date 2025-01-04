#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class WarningPopup : public Popup<> {
protected:
    bool setup();
    void keyBackClicked() override;

    TextArea* m_textArea = nullptr;
    cocos2d::extension::CCScale9Sprite* m_darkenedArea = nullptr;
    CCMenuItemSpriteExtra* m_okBtn = nullptr;

public:
    static WarningPopup* create();
    void onClose(CCObject*) override;
};