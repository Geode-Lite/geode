#include <Geode/Geode.hpp>
using namespace geode::prelude;

class WarningPopup : public FLAlertLayer {
protected:
    $override
    virtual bool init();

    $override
    virtual void keyBackClicked();

    cocos2d::extension::CCScale9Sprite* m_background = nullptr;
    cocos2d::extension::CCScale9Sprite* m_darkenedArea = nullptr;
    cocos2d::CCLabelBMFont* m_title = nullptr;
    TextArea* m_textArea = nullptr;
    CCMenuItemSpriteExtra* m_okBtn = nullptr;

public:
    static WarningPopup* create();
    
    void onClose(CCObject*);
};

WarningPopup* WarningPopup::create() {
    auto ret = new WarningPopup();
    if(ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool WarningPopup::init() {
    if(!FLAlertLayer::init(150))
        return false;

    auto director = CCDirector::get();
    auto winSize = director->getWinSize();

    m_noElasticity = true;
    m_reverseKeyBack = true;

    m_background = CCScale9Sprite::create("GJ_square01.png");
    m_background->setContentSize({350.f, 200.f});
    m_background->setPosition(winSize / 2);

    m_mainLayer->addChild(m_background, -2);

    m_darkenedArea = CCScale9Sprite::create("GJ_square01.png");
    m_darkenedArea->setContentSize({320.f, 100.f});
    m_darkenedArea->setPosition(winSize.width / 2, winSize.height / 2 + 6);
    m_darkenedArea->setColor({0, 0, 0});
    m_darkenedArea->setOpacity(60);

    m_mainLayer->addChild(m_darkenedArea, -1);

    m_title = CCLabelBMFont::create("Geode lite", "goldFont.fnt");
    m_title->setPosition(winSize.width / 2, m_background->getPositionY() + 100.f - 16.f); // idk
    m_title->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
    m_title->setScale(0.8);

    m_mainLayer->addChild(m_title);

    m_textArea = TextArea::create("<cy>Geode lite</c> is a fan port of <cp>Geode</c> to Geometry Dash <cy>Lite</c>.\nas a result some mods may be <cr>incompatible</c> and <cr>crash</c>", "bigFont.fnt", 1.f, 630.f, {0.5, 1.F}, 42.f, false);
    m_textArea->setPosition({winSize.width / 2, winSize.height / 2 + 15.f});
    m_textArea->setScale(0.5f);

    m_buttonMenu = CCMenu::create();

    m_mainLayer->addChild(m_buttonMenu);


    m_button1 = ButtonSprite::create("I UNDERSTAND",0,0,1.0,false,"goldFont.fnt","GJ_button_01.png",0.0);
    m_okBtn = CCMenuItemSpriteExtra::create(m_button1, this, menu_selector(WarningPopup::onClose));
    m_okBtn->setPosition(m_buttonMenu->convertToNodeSpace({winSize.width / 2, m_background->getPositionY() - 100.f + 25.f}));

    m_buttonMenu->addChild(m_okBtn);

    m_mainLayer->addChild(m_textArea);


    return true;
}

void WarningPopup::onClose(CCObject*) {
    Mod::get()->setSavedValue<bool>("viewedPopup", true);
    setKeypadEnabled(false);
    setTouchEnabled(false);
    removeFromParentAndCleanup(true);
}

void WarningPopup::keyBackClicked() {
    return;
}
