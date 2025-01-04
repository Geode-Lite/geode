#include "WarningPopup.hpp"

WarningPopup* WarningPopup::create() {
    auto ret = new WarningPopup();
    if (ret->initAnchored(350.f, 200.f)) {
        ret->autorelease();
        return ret;
    }
    
    delete ret;
    return nullptr;
}

bool WarningPopup::setup() {
    auto director = CCDirector::get();
    auto winSize = director->getWinSize();

    m_bgSprite->setSpriteFrame(CCSpriteFrame::create("GJ_square02.png", {0, 0, 80, 80}));
    m_bgSprite->setContentSize(m_mainLayer->getContentSize());

    m_noElasticity = true;
    m_reverseKeyBack = true;

    setTitle("Geode Lite");

    m_textArea = TextArea::create("Geode Lite is a fan port of Geode to GD Lite. Some mods may have issues or crashes.\nGeode lead developers will not help with issues caused by Geode Lite.", "bigFont.fnt", 1.f, 630.f, {0.5, 1.F}, 30.f, false);
    m_textArea->setScale(0.5f);

    m_darkenedArea = CCScale9Sprite::create("square02_001.png");
    m_darkenedArea->setContentSize({320.f, 100.f});
    m_darkenedArea->setOpacity(60);

    m_button1 = ButtonSprite::create("I Understand","goldFont.fnt","GJ_button_01.png");
    m_okBtn = CCMenuItemSpriteExtra::create(m_button1, this, menu_selector(WarningPopup::onClose));
    m_okBtn->setPosition(m_buttonMenu->convertToNodeSpace({winSize.width / 2, getContentSize().height - 100.f + 25.f}));

    m_closeBtn->removeFromParentAndCleanup(true);
    m_buttonMenu->addChildAtPosition(m_okBtn, Anchor::Bottom, {0, 25});
    m_mainLayer->addChildAtPosition(m_darkenedArea, Anchor::Center, {0, 6});
    m_mainLayer->addChildAtPosition(m_textArea, Anchor::Center, {0, 15});

    return true;
}

void WarningPopup::onClose(CCObject* obj) {
    Popup::onClose(obj);
    Mod::get()->setSavedValue<bool>("viewedPopup", true);
}

void WarningPopup::keyBackClicked() {
    return;
}