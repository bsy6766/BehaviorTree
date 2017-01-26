#include <gtest\gtest.h>
#include <string>
#include "../src/BehaviorTree.h"

// A sample node that returns SUCCESS state and does nothing.
class SuccessNode : public BehaviorTree::Node
{
public:
	SuccessNode() : Node() {}
	~SuccessNode() = default;
	virtual const BehaviorTree::NODE_STATE update(const float delta = 0) override
	{
		//Do nothing
		return BehaviorTree::NODE_STATE::SUCCESS;
	}
	std::string tag;
};

// A sample node that returns FAILURE state and does nothing.
class FailureNode : public BehaviorTree::Node
{
public:
	FailureNode() : Node() {}
	~FailureNode() = default;
	virtual const BehaviorTree::NODE_STATE update(const float delta = 0) override
	{
		//Do nothing
		return BehaviorTree::NODE_STATE::FAILURE;
	}
	std::string tag;
};

// A sample node that run for certain amount of time and does nothing.
class RunningNode : public BehaviorTree::Node
{
public:
	RunningNode(const float duration) : Node(), duration(duration), elapsedTime(0) {}
	~RunningNode() = default;

	float duration;
	float elapsedTime;

	virtual const BehaviorTree::NODE_STATE update(const float delta = 0) override
	{
		//Do nothing
		if (this->elapsedTime < this->duration)
		{
			this->elapsedTime += delta;
			if (this->elapsedTime >= this->duration)
			{
				return BehaviorTree::NODE_STATE::SUCCESS;
			}
			else
			{
				return BehaviorTree::NODE_STATE::RUNNING;
			}
		}
		else
		{
			return BehaviorTree::NODE_STATE::SUCCESS;
		}
	}
	std::string tag;
};

template<class T>
std::unique_ptr<T> create()
{
	return std::unique_ptr<T>(new T());
}

void createTwoSuccessNodeVector(std::vector<std::unique_ptr<BehaviorTree::Node>>& children)
{
	std::unique_ptr<SuccessNode> successNode1 = create<SuccessNode>();
	std::unique_ptr<SuccessNode> successNode2 = create<SuccessNode>();
	children.push_back(std::move(successNode1));
	children.push_back(std::move(successNode2));
}

//========================================== CREATION TEST ==========================================
TEST(CREATION_TEST, COMPOSITE_NODES)
{
	BehaviorTree::Selector* selector = new BehaviorTree::Selector(nullptr);
	ASSERT_NE(selector, nullptr);
	delete selector;

	BehaviorTree::RandomSelector* randomSelector = new BehaviorTree::RandomSelector(nullptr);
	ASSERT_NE(randomSelector, nullptr);
	delete randomSelector;

	BehaviorTree::Sequence* sequence = new BehaviorTree::Sequence(nullptr);
	ASSERT_NE(sequence, nullptr);
	delete sequence;

	BehaviorTree::RandomSequence* randomSequence = new BehaviorTree::RandomSequence(nullptr);
	ASSERT_NE(randomSequence, nullptr);
	delete randomSequence;
}

TEST(CREATION_TEST, DECORATIVE_NODES)
{
	BehaviorTree::Inverter* inverter = new BehaviorTree::Inverter();
	ASSERT_NE(inverter, nullptr);
	delete inverter;

	BehaviorTree::Succeeder* succeeder = new BehaviorTree::Succeeder();
	ASSERT_NE(succeeder, nullptr);
	delete succeeder;

	BehaviorTree::Failer* failer = new BehaviorTree::Failer();
	ASSERT_NE(failer, nullptr);
	delete failer;

	BehaviorTree::Repeater* repeater = new BehaviorTree::Repeater(0);
	ASSERT_NE(repeater, nullptr);
	delete repeater;

	BehaviorTree::RepeatUntilFail* repeatUntilFail = new BehaviorTree::RepeatUntilFail(0);
	ASSERT_NE(repeatUntilFail, nullptr);
	delete repeatUntilFail;

	BehaviorTree::RepeatUntilSuccess* repeatUntilSuccess = new BehaviorTree::RepeatUntilSuccess(0);
	ASSERT_NE(repeatUntilSuccess, nullptr);
	delete repeatUntilSuccess;

	BehaviorTree::Limiter* limiter = new BehaviorTree::Limiter(0);
	ASSERT_NE(limiter, nullptr);
	delete limiter;

	BehaviorTree::DelayTime* delayTime = new BehaviorTree::DelayTime(0, true);
	ASSERT_NE(delayTime, nullptr);
	delete delayTime;

	BehaviorTree::TimeLimit* timeLimit = new BehaviorTree::TimeLimit(0);
	ASSERT_NE(timeLimit, nullptr);
	delete timeLimit;
}
//=====================================================================================================

//========================================== ZERO CHILD TEST ==========================================
// Creates Composite node and run single update to see if it returns ERROR state with 0 child.

TEST(ZERO_CHILD_TEST, SELECTOR)
{
	BehaviorTree::Selector* selector = new BehaviorTree::Selector(nullptr);
	ASSERT_NE(selector, nullptr);

	const std::vector<std::unique_ptr<BehaviorTree::Node>>& children = selector->getChildren();
	ASSERT_EQ(children.size(), 0);

	const BehaviorTree::NODE_STATE state = selector->update(0.0f);
	ASSERT_EQ(state, BehaviorTree::NODE_STATE::ERROR);

	delete selector;
}

TEST(ZERO_CHILD_TEST, RANDOM_SELECTOR)
{
	BehaviorTree::RandomSelector* randomSelector = new BehaviorTree::RandomSelector(nullptr);
	ASSERT_NE(randomSelector, nullptr);

	const std::vector<std::unique_ptr<BehaviorTree::Node>>& children = randomSelector->getChildren();
	ASSERT_EQ(children.size(), 0);

	const BehaviorTree::NODE_STATE state = randomSelector->update(0.0f);
	ASSERT_EQ(state, BehaviorTree::NODE_STATE::ERROR);

	delete randomSelector;
}

TEST(ZERO_CHILD_TEST, SEQUENCE)
{
	BehaviorTree::Sequence* sequence = new BehaviorTree::Sequence(nullptr);
	ASSERT_NE(sequence, nullptr);

	const std::vector<std::unique_ptr<BehaviorTree::Node>>& children = sequence->getChildren();
	ASSERT_EQ(children.size(), 0);

	const BehaviorTree::NODE_STATE state = sequence->update(0.0f);
	ASSERT_EQ(state, BehaviorTree::NODE_STATE::ERROR);

	delete sequence;
}

TEST(ZERO_CHILD_TEST, RANDOM_SEQUENCE)
{
	BehaviorTree::RandomSequence* randomSequence = new BehaviorTree::RandomSequence(nullptr);
	ASSERT_NE(randomSequence, nullptr);

	const std::vector<std::unique_ptr<BehaviorTree::Node>>& children = randomSequence->getChildren();
	ASSERT_EQ(children.size(), 0);

	const BehaviorTree::NODE_STATE state = randomSequence->update(0.0f);
	ASSERT_EQ(state, BehaviorTree::NODE_STATE::ERROR);

	delete randomSequence;
}
//=====================================================================================================

//========================================= SINGLE CHILD TEST =========================================
// Creates Composite node with single child and run update to see if it returns correct result.

TEST(SINGLE_CHILD_TEST, SELECTOR)
{
	std::unique_ptr<SuccessNode> successNode = create<SuccessNode>();
	BehaviorTree::Selector* selector = new BehaviorTree::Selector(std::move(successNode));
	ASSERT_NE(selector, nullptr);
	ASSERT_EQ(successNode, nullptr);

	const std::vector<std::unique_ptr<BehaviorTree::Node>>& children = selector->getChildren();
	ASSERT_EQ(children.size(), 1);

	const BehaviorTree::NODE_STATE state = selector->update(0.0f);
	ASSERT_EQ(state, BehaviorTree::NODE_STATE::SUCCESS);

	delete selector;
}

TEST(SINGLE_CHILD_TEST, RANDOM_SELECTOR)
{
	std::unique_ptr<SuccessNode> successNode = create<SuccessNode>();
	BehaviorTree::RandomSelector* randomSelector = new BehaviorTree::RandomSelector(std::move(successNode));
	ASSERT_NE(randomSelector, nullptr);
	ASSERT_EQ(successNode, nullptr);

	const std::vector<std::unique_ptr<BehaviorTree::Node>>& children = randomSelector->getChildren();
	ASSERT_EQ(children.size(), 1);

	const BehaviorTree::NODE_STATE state = randomSelector->update(0.0f);
	ASSERT_EQ(state, BehaviorTree::NODE_STATE::SUCCESS);

	delete randomSelector;
}

TEST(SINGLE_CHILD_TEST, SEQUENCE)
{
	std::unique_ptr<SuccessNode> successNode = create<SuccessNode>();
	BehaviorTree::Sequence* sequence = new BehaviorTree::Sequence(std::move(successNode));
	ASSERT_NE(sequence, nullptr);
	ASSERT_EQ(successNode, nullptr);

	const std::vector<std::unique_ptr<BehaviorTree::Node>>& children = sequence->getChildren();
	ASSERT_EQ(children.size(), 1);

	const BehaviorTree::NODE_STATE state = sequence->update(0.0f);
	ASSERT_EQ(state, BehaviorTree::NODE_STATE::SUCCESS);

	delete sequence;
}

TEST(SINGLE_CHILD_TEST, RANDOM_SEQUENCE)
{
	std::unique_ptr<SuccessNode> successNode = create<SuccessNode>();
	BehaviorTree::RandomSequence* randomSequence = new BehaviorTree::RandomSequence(std::move(successNode));
	ASSERT_NE(randomSequence, nullptr);
	ASSERT_EQ(successNode, nullptr);

	const std::vector<std::unique_ptr<BehaviorTree::Node>>& children = randomSequence->getChildren();
	ASSERT_EQ(children.size(), 1);

	const BehaviorTree::NODE_STATE state = randomSequence->update(0.0f);
	ASSERT_EQ(state, BehaviorTree::NODE_STATE::SUCCESS);

	delete randomSequence;
}
//=====================================================================================================

//=========================================== ADD CHILD TEST ==========================================
// Creates Composite node with empty child and add new child with addChild() function. Then run update to see if it returns correct result.
TEST(ADD_CHILD_TEST, SELECTOR)
{
	std::unique_ptr<SuccessNode> successNode = create<SuccessNode>();
	BehaviorTree::Selector* selector = new BehaviorTree::Selector(nullptr);
	ASSERT_NE(selector, nullptr);

	const std::vector<std::unique_ptr<BehaviorTree::Node>>& emptyChildren = selector->getChildren();
	ASSERT_EQ(emptyChildren.size(), 0);

	selector->addChild(std::move(successNode));
	ASSERT_EQ(successNode, nullptr);

	const std::vector<std::unique_ptr<BehaviorTree::Node>>& children = selector->getChildren();
	ASSERT_EQ(children.size(), 1);

	const BehaviorTree::NODE_STATE state = selector->update(0.0f);
	ASSERT_EQ(state, BehaviorTree::NODE_STATE::SUCCESS);

	delete selector;
}

TEST(ADD_CHILD_TEST, RANDOM_SELECTOR)
{
	std::unique_ptr<SuccessNode> successNode = create<SuccessNode>();
	BehaviorTree::RandomSelector* randomSelector = new BehaviorTree::RandomSelector(nullptr);
	ASSERT_NE(randomSelector, nullptr);

	const std::vector<std::unique_ptr<BehaviorTree::Node>>& emptyChildren = randomSelector->getChildren();
	ASSERT_EQ(emptyChildren.size(), 0);

	randomSelector->addChild(std::move(successNode));
	ASSERT_EQ(successNode, nullptr);

	const std::vector<std::unique_ptr<BehaviorTree::Node>>& children = randomSelector->getChildren();
	ASSERT_EQ(children.size(), 1);

	const BehaviorTree::NODE_STATE state = randomSelector->update(0.0f);
	ASSERT_EQ(state, BehaviorTree::NODE_STATE::SUCCESS);

	delete randomSelector;
}

TEST(ADD_CHILD_TEST, SEQUENCE)
{
	std::unique_ptr<SuccessNode> successNode = create<SuccessNode>();
	BehaviorTree::Sequence* sequence = new BehaviorTree::Sequence(nullptr);
	ASSERT_NE(sequence, nullptr);

	const std::vector<std::unique_ptr<BehaviorTree::Node>>& emptyChildren = sequence->getChildren();
	ASSERT_EQ(emptyChildren.size(), 0);

	sequence->addChild(std::move(successNode));
	ASSERT_EQ(successNode, nullptr);

	const std::vector<std::unique_ptr<BehaviorTree::Node>>& children = sequence->getChildren();
	ASSERT_EQ(children.size(), 1);

	const BehaviorTree::NODE_STATE state = sequence->update(0.0f);
	ASSERT_EQ(state, BehaviorTree::NODE_STATE::SUCCESS);

	delete sequence;
}

TEST(ADD_CHILD_TEST, RANDOM_SEQUENCE)
{
	std::unique_ptr<SuccessNode> successNode = create<SuccessNode>();
	BehaviorTree::RandomSequence* randomSequence = new BehaviorTree::RandomSequence(nullptr);
	ASSERT_NE(randomSequence, nullptr);

	const std::vector<std::unique_ptr<BehaviorTree::Node>>& emptyChildren = randomSequence->getChildren();
	ASSERT_EQ(emptyChildren.size(), 0);

	randomSequence->addChild(std::move(successNode));
	ASSERT_EQ(successNode, nullptr);

	const std::vector<std::unique_ptr<BehaviorTree::Node>>& children = randomSequence->getChildren();
	ASSERT_EQ(children.size(), 1);

	const BehaviorTree::NODE_STATE state = randomSequence->update(0.0f);
	ASSERT_EQ(state, BehaviorTree::NODE_STATE::SUCCESS);

	delete randomSequence;
}
//=====================================================================================================

//=========================================== CHILDREN TEST ===========================================
// Create Composite node with children and run update to see if it returns correct result
TEST(CHILDREN_TEST, SELECTOR)
{
	std::vector<std::unique_ptr<BehaviorTree::Node>> newChildren;
	createTwoSuccessNodeVector(newChildren);
	BehaviorTree::Selector* selector = new BehaviorTree::Selector(newChildren);
	ASSERT_EQ(newChildren.at(0), nullptr);
	ASSERT_EQ(newChildren.at(1), nullptr);
	ASSERT_NE(selector, nullptr);

	const std::vector<std::unique_ptr<BehaviorTree::Node>>& children = selector->getChildren();
	ASSERT_EQ(children.size(), 2);

	const BehaviorTree::NODE_STATE state = selector->update(0.0f);
	ASSERT_EQ(state, BehaviorTree::NODE_STATE::SUCCESS);

	delete selector;
}

TEST(CHILDREN_TEST, RANDOM_SELECTOR)
{
	std::vector<std::unique_ptr<BehaviorTree::Node>> newChildren;
	createTwoSuccessNodeVector(newChildren);
	BehaviorTree::RandomSelector* randomSelector = new BehaviorTree::RandomSelector(newChildren);
	ASSERT_EQ(newChildren.at(0), nullptr);
	ASSERT_EQ(newChildren.at(1), nullptr);
	ASSERT_NE(randomSelector, nullptr);

	const std::vector<std::unique_ptr<BehaviorTree::Node>>& children = randomSelector->getChildren();
	ASSERT_EQ(children.size(), 2);

	const BehaviorTree::NODE_STATE state = randomSelector->update(0.0f);
	ASSERT_EQ(state, BehaviorTree::NODE_STATE::SUCCESS);

	delete randomSelector;
}

TEST(CHILDREN_TEST, SEQUENCE)
{
	std::vector<std::unique_ptr<BehaviorTree::Node>> newChildren;
	createTwoSuccessNodeVector(newChildren);
	BehaviorTree::Sequence* sequence = new BehaviorTree::Sequence(newChildren);
	ASSERT_EQ(newChildren.at(0), nullptr);
	ASSERT_EQ(newChildren.at(1), nullptr);
	ASSERT_NE(sequence, nullptr);

	const std::vector<std::unique_ptr<BehaviorTree::Node>>& children = sequence->getChildren();
	ASSERT_EQ(children.size(), 2);

	const BehaviorTree::NODE_STATE state = sequence->update(0.0f);
	ASSERT_EQ(state, BehaviorTree::NODE_STATE::SUCCESS);

	delete sequence;
}

TEST(CHILDREN_TEST, RANDOM_SEQUENCE)
{
	std::vector<std::unique_ptr<BehaviorTree::Node>> newChildren;
	createTwoSuccessNodeVector(newChildren);
	BehaviorTree::RandomSequence* randomSequence = new BehaviorTree::RandomSequence(newChildren);
	ASSERT_EQ(newChildren.at(0), nullptr);
	ASSERT_EQ(newChildren.at(1), nullptr);
	ASSERT_NE(randomSequence, nullptr);

	const std::vector<std::unique_ptr<BehaviorTree::Node>>& children = randomSequence->getChildren();
	ASSERT_EQ(children.size(), 2);

	const BehaviorTree::NODE_STATE state = randomSequence->update(0.0f);
	ASSERT_EQ(state, BehaviorTree::NODE_STATE::SUCCESS);

	delete randomSequence;
}
//=====================================================================================================

//========================================= ADD CHILDREN TEST =========================================
// Create Composite node with empty child and add new children with addChildren(). Then run update to see if it returns correct result
TEST(ADD_CHILDREN_TEST, SELECTOR)
{
	BehaviorTree::Selector* selector = new BehaviorTree::Selector(nullptr);
	ASSERT_NE(selector, nullptr);

	const std::vector<std::unique_ptr<BehaviorTree::Node>>& emptyChildren = selector->getChildren();
	ASSERT_EQ(emptyChildren.size(), 0);

	std::vector<std::unique_ptr<BehaviorTree::Node>> newChildren;
	createTwoSuccessNodeVector(newChildren);
	selector->addChildren(newChildren);
	ASSERT_EQ(newChildren.at(0), nullptr);
	ASSERT_EQ(newChildren.at(1), nullptr);

	const BehaviorTree::NODE_STATE state = selector->update(0.0f);
	ASSERT_EQ(state, BehaviorTree::NODE_STATE::SUCCESS);

	delete selector;
}

TEST(ADD_CHILDREN_TEST, RANDOM_SELECTOR)
{
	BehaviorTree::RandomSelector* randomSelector = new BehaviorTree::RandomSelector(nullptr);
	ASSERT_NE(randomSelector, nullptr);

	const std::vector<std::unique_ptr<BehaviorTree::Node>>& emptyChildren = randomSelector->getChildren();
	ASSERT_EQ(emptyChildren.size(), 0);

	std::vector<std::unique_ptr<BehaviorTree::Node>> newChildren;
	createTwoSuccessNodeVector(newChildren);
	randomSelector->addChildren(newChildren);
	ASSERT_EQ(newChildren.at(0), nullptr);
	ASSERT_EQ(newChildren.at(1), nullptr);

	const BehaviorTree::NODE_STATE state = randomSelector->update(0.0f);
	ASSERT_EQ(state, BehaviorTree::NODE_STATE::SUCCESS);

	delete randomSelector;
}

TEST(ADD_CHILDREN_TEST, SEQUENCE)
{
	BehaviorTree::Sequence* sequence = new BehaviorTree::Sequence(nullptr);
	ASSERT_NE(sequence, nullptr);

	const std::vector<std::unique_ptr<BehaviorTree::Node>>& emptyChildren = sequence->getChildren();
	ASSERT_EQ(emptyChildren.size(), 0);

	std::vector<std::unique_ptr<BehaviorTree::Node>> newChildren;
	createTwoSuccessNodeVector(newChildren);
	sequence->addChildren(newChildren);
	ASSERT_EQ(newChildren.at(0), nullptr);
	ASSERT_EQ(newChildren.at(1), nullptr);

	const BehaviorTree::NODE_STATE state = sequence->update(0.0f);
	ASSERT_EQ(state, BehaviorTree::NODE_STATE::SUCCESS);

	delete sequence;
}

TEST(ADD_CHILDREN_TEST, RANDOM_SEQUENCE)
{
	BehaviorTree::RandomSequence* randomSequence = new BehaviorTree::RandomSequence(nullptr);
	ASSERT_NE(randomSequence, nullptr);

	const std::vector<std::unique_ptr<BehaviorTree::Node>>& emptyChildren = randomSequence->getChildren();
	ASSERT_EQ(emptyChildren.size(), 0);

	std::vector<std::unique_ptr<BehaviorTree::Node>> newChildren;
	createTwoSuccessNodeVector(newChildren);
	randomSequence->addChildren(newChildren);
	ASSERT_EQ(newChildren.at(0), nullptr);
	ASSERT_EQ(newChildren.at(1), nullptr);

	const BehaviorTree::NODE_STATE state = randomSequence->update(0.0f);
	ASSERT_EQ(state, BehaviorTree::NODE_STATE::SUCCESS);

	delete randomSequence;
}
//=====================================================================================================

//=========================================== SELECTOR TEST ===========================================
TEST(SELECTOR_TEST, SINGLE_SUCCESS_CHILD)
{
	std::vector<std::unique_ptr<BehaviorTree::Node>> newChildren;
	newChildren.push_back(std::move(create<SuccessNode>()));
	ASSERT_EQ(newChildren.size(), 1);

	BehaviorTree::Selector* selector = new BehaviorTree::Selector(newChildren);
	ASSERT_NE(selector, nullptr);

	BehaviorTree::NODE_STATE state = selector->update(0.0f);
	ASSERT_EQ(state, BehaviorTree::NODE_STATE::SUCCESS);

	delete selector;
}

TEST(SELECTOR_TEST, ALL_SUCCESS_CHILDREN)
{
	std::vector<std::unique_ptr<BehaviorTree::Node>> newChildren;
	for (int i = 0; i < 5; i++)
	{
		newChildren.push_back(std::move(create<SuccessNode>()));
	}
	ASSERT_EQ(newChildren.size(), 5);

	BehaviorTree::Selector* selector = new BehaviorTree::Selector(newChildren);
	ASSERT_NE(selector, nullptr);

	BehaviorTree::NODE_STATE state = selector->update(0.0f);
	ASSERT_EQ(state, BehaviorTree::NODE_STATE::SUCCESS);

	delete selector;
}

TEST(SELECTOR_TEST, SINGLE_FAILURE_CHILD)
{
	std::vector<std::unique_ptr<BehaviorTree::Node>> newChildren;
	newChildren.push_back(std::move(create<FailureNode>()));
	ASSERT_EQ(newChildren.size(), 1);

	BehaviorTree::Selector* selector = new BehaviorTree::Selector(newChildren);
	ASSERT_NE(selector, nullptr);

	BehaviorTree::NODE_STATE state = selector->update(0.0f);
	ASSERT_EQ(state, BehaviorTree::NODE_STATE::FAILURE);

	delete selector;
}

TEST(SELECTOR_TEST, ALL_FAILURE_CHILDREN)
{
	std::vector<std::unique_ptr<BehaviorTree::Node>> newChildren;
	for (int i = 0; i < 5; i++)
	{
		newChildren.push_back(std::move(create<FailureNode>()));
	}
	ASSERT_EQ(newChildren.size(), 5);

	BehaviorTree::Selector* selector = new BehaviorTree::Selector(newChildren);
	ASSERT_NE(selector, nullptr);

	BehaviorTree::NODE_STATE state = selector->update(0.0f);
	ASSERT_EQ(state, BehaviorTree::NODE_STATE::FAILURE);

	delete selector;
}

TEST(SELECTOR_TEST, SUCCESS_AFTER_FAILURE)
{
	std::vector<std::unique_ptr<BehaviorTree::Node>> newChildren;
	newChildren.push_back(std::move(create<FailureNode>()));
	newChildren.push_back(std::move(create<SuccessNode>()));
	ASSERT_EQ(newChildren.size(), 2);

	BehaviorTree::Selector* selector = new BehaviorTree::Selector(newChildren);
	ASSERT_NE(selector, nullptr);

	BehaviorTree::NODE_STATE state = selector->update(0.0f);
	ASSERT_EQ(state, BehaviorTree::NODE_STATE::SUCCESS);

	delete selector;
}

TEST(SELECTOR_TSET, RUNNING)
{
	RunningNode* runningNode = new RunningNode(3.0f);

	BehaviorTree::Selector* selector = new BehaviorTree::Selector(std::unique_ptr<BehaviorTree::Node>(runningNode));
	ASSERT_NE(selector, nullptr);

	BehaviorTree::NODE_STATE state;
	state = selector->update(1.0f);
	ASSERT_EQ(state, BehaviorTree::NODE_STATE::RUNNING);
	state = selector->update(1.5f);
	ASSERT_EQ(state, BehaviorTree::NODE_STATE::RUNNING);
	state = selector->update(2.5f);
	ASSERT_EQ(state, BehaviorTree::NODE_STATE::SUCCESS);
	state = selector->update(1.0f);
	ASSERT_EQ(state, BehaviorTree::NODE_STATE::SUCCESS);

	delete selector;
}

TEST(SELECTOR_TEST, NESTED_SELECTOR)
{
	BehaviorTree::Selector* rootSelector = new BehaviorTree::Selector(nullptr);
	ASSERT_NE(rootSelector, nullptr);

	BehaviorTree::Selector* nestedSelector = new BehaviorTree::Selector(nullptr);
	ASSERT_NE(nestedSelector, nullptr);

	rootSelector->addChild(std::move(create<FailureNode>()));
	rootSelector->addChild(std::unique_ptr<BehaviorTree::Selector>(nestedSelector));
	ASSERT_EQ(rootSelector->getChildren().size(), 2);

	// It will pass first node(Failure) and run selector
	BehaviorTree::NODE_STATE state = rootSelector->update(0);
	if (BehaviorTree::IGNORE_ERROR)
	{
		ASSERT_EQ(state, BehaviorTree::NODE_STATE::FAILURE);
	}
	else
	{
		ASSERT_EQ(state, BehaviorTree::NODE_STATE::ERROR);
	}

	delete rootSelector;
}
//=====================================================================================================

//==================================== RANDOM SELECTOR TEST ===========================================
TEST(RANDOM_SELECTOR_TEST, SINGLE_SUCCESS_CHILD)
{
	std::vector<std::unique_ptr<BehaviorTree::Node>> newChildren;
	newChildren.push_back(std::move(create<SuccessNode>()));
	ASSERT_EQ(newChildren.size(), 1);

	BehaviorTree::RandomSelector* randomSelector = new BehaviorTree::RandomSelector(newChildren);
	ASSERT_NE(randomSelector, nullptr);

	BehaviorTree::NODE_STATE state = randomSelector->update(0.0f);
	ASSERT_EQ(state, BehaviorTree::NODE_STATE::SUCCESS);

	delete randomSelector;
}

TEST(RANDOM_SELECTOR_TEST, ALL_SUCCESS_CHILDREN)
{
	std::vector<std::unique_ptr<BehaviorTree::Node>> newChildren;
	for (int i = 0; i < 5; i++)
	{
		newChildren.push_back(std::move(create<SuccessNode>()));
	}
	ASSERT_EQ(newChildren.size(), 5);

	BehaviorTree::RandomSelector* randomSelector = new BehaviorTree::RandomSelector(newChildren);
	ASSERT_NE(randomSelector, nullptr);

	BehaviorTree::NODE_STATE state = randomSelector->update(0.0f);
	ASSERT_EQ(state, BehaviorTree::NODE_STATE::SUCCESS);

	delete randomSelector;
}

TEST(RANDOM_SELECTOR_TEST, SINGLE_FAILURE_CHILD)
{
	std::vector<std::unique_ptr<BehaviorTree::Node>> newChildren;
	newChildren.push_back(std::move(create<FailureNode>()));
	ASSERT_EQ(newChildren.size(), 1);

	BehaviorTree::RandomSelector* randomSelector = new BehaviorTree::RandomSelector(newChildren);
	ASSERT_NE(randomSelector, nullptr);

	BehaviorTree::NODE_STATE state = randomSelector->update(0.0f);
	ASSERT_EQ(state, BehaviorTree::NODE_STATE::FAILURE);

	delete randomSelector;
}

TEST(RANDOM_SELECTOR_TEST, ALL_FAILURE_CHILDREN)
{
	std::vector<std::unique_ptr<BehaviorTree::Node>> newChildren;
	for (int i = 0; i < 5; i++)
	{
		newChildren.push_back(std::move(create<FailureNode>()));
	}
	ASSERT_EQ(newChildren.size(), 5);

	BehaviorTree::RandomSelector* randomSelector = new BehaviorTree::RandomSelector(newChildren);
	ASSERT_NE(randomSelector, nullptr);

	BehaviorTree::NODE_STATE state = randomSelector->update(0.0f);
	ASSERT_EQ(state, BehaviorTree::NODE_STATE::FAILURE);

	delete randomSelector;
}

TEST(RANDOM_SELECTOR_TEST, SHUFFLE_CHILD)
{
	SuccessNode* successNode = new SuccessNode();
	successNode->tag = "FirstNode";

	std::unique_ptr<BehaviorTree::Node> succesNodePtr(successNode);
	BehaviorTree::RandomSelector* randomSelector = new BehaviorTree::RandomSelector(std::move(succesNodePtr));
	ASSERT_NE(randomSelector, nullptr);

	randomSelector->update();

	const std::vector<std::unique_ptr<BehaviorTree::Node>>& child = randomSelector->getChildren();

	SuccessNode* shuffledNode = dynamic_cast<SuccessNode*>(child.at(0).get());
	ASSERT_EQ(shuffledNode->tag, "FirstNode");

	delete randomSelector;
}

TEST(RANDOM_SELECTOR_TEST, SHUFFLE_CHILDREN)
{
	std::vector<std::unique_ptr<BehaviorTree::Node>> newChildren;
	for (int i = 0; i < 20; i++)
	{
		newChildren.push_back(std::move(create<SuccessNode>()));
		dynamic_cast<SuccessNode*>(newChildren.back().get())->tag = std::to_string(i);
	}
	ASSERT_EQ(newChildren.size(), 20);

	BehaviorTree::RandomSelector* randomSelector = new BehaviorTree::RandomSelector(newChildren);
	ASSERT_NE(randomSelector, nullptr);

	randomSelector->update();

	const std::vector<std::unique_ptr<BehaviorTree::Node>>& children = randomSelector->getChildren();

	int count = 0;
	int misMatchCount = 0;

	std::cout << "Random Selector shuffled children." << std::endl;
	std::cout << "Before shuffle: ";

	for (int i = 0; i < 20; i++)
	{
		std::cout << i << " ";
	}

	std::cout << std::endl;
	std::cout << "After shuffle:  ";

	for (auto& child : children)
	{
		std::string tag = dynamic_cast<SuccessNode*>(child.get())->tag;
		std::cout << tag << " ";
		if (tag != std::to_string(count))
		{
			misMatchCount++;
		}
		count++;
	}
	std::cout << std::endl;

	delete randomSelector;
}
//=====================================================================================================