#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>

#include "script.hpp"

using namespace geode::prelude;

class MyLayer : public CCLayer {
public:
	std::unique_ptr<Script> script;

	bool init() override {
		if (!CCLayer::init()) return false;

		this->setKeypadEnabled(true);

		auto root = CCNode::create();
		root->setLayout(
			SimpleColumnLayout::create()
			->setMainAxisScaling(AxisScaling::Grow)
			->setCrossAxisScaling(AxisScaling::Grow)
		);
		root->setAnchorPoint(ccp(0.5, 0.5));

		this->addChildAtPosition(root, Anchor::Center);
		this->ignoreAnchorPointForPosition(true);

		log::info("-- loading script --");
		script = loadScript(root, "C:\\Users\\Mateus\\Coding\\qjs-solid\\scripts\\dist.mjs");
		this->scheduleUpdate();
		this->ignoreAnchorPointForPosition(true);

		return true;
	}

	void keyBackClicked() override {
		CCDirector::get()->popScene();
	}

	void update(float dt) override {
		if (script) script->tick();
	}

	CREATE_FUNC(MyLayer);
};

class $modify(MyMenuLayer, MenuLayer) {
	bool init() {
		if (!MenuLayer::init()) {
			return false;
		}

		auto myButton = CCMenuItemSpriteExtra::create(
			CCSprite::createWithSpriteFrameName("accountBtn_myLists_001.png"),
			this,
			menu_selector(MyMenuLayer::onMyButton)
		);

		auto menu = this->getChildByID("bottom-menu");
		menu->addChild(myButton);

		myButton->setID("my-button"_spr);

		menu->updateLayout();

		return true;
	}

	void onMyButton(CCObject*) {
		pushSceneWithLayer(MyLayer::create());
	}
};