#include "BehaviorTree.h"

MGD::BTree::Node::~Node()
{}

MGD::BTree::CompositeNode::CompositeNode() : Node(), maxChildren(MGD::BTree::INFINITE_CHILDREN), runningChildIndex(-1)
{}

MGD::BTree::CompositeNode::CompositeNode(Node* child) : Node(), maxChildren(MGD::BTree::INFINITE_CHILDREN), runningChildIndex(-1)
{
	this->addChild(child);
}

MGD::BTree::CompositeNode::CompositeNode(const std::vector<Node*>& children) : Node(), maxChildren(MGD::BTree::INFINITE_CHILDREN), runningChildIndex(-1)
{
	this->addChildren(children);
}

void MGD::BTree::CompositeNode::addChild(Node* child)
{
	if (child != nullptr)
	{
		this->children.push_back(child);
	}
}

void MGD::BTree::CompositeNode::addChildren(const std::vector<Node*>& children)
{
	if (this->maxChildren > 0)
	{
		int currentChildrenSize = static_cast<int>(this->children.size());

		int addingChildrenSize = static_cast<int>(children.size());

		if (currentChildrenSize + addingChildrenSize > this->maxChildren)
		{
			return;
		}
	}

	for (auto child : children)
	{
		if (child != nullptr)
		{
			this->addChild(child);
		}
	}
}

const std::vector<MGD::BTree::Node*>& MGD::BTree::CompositeNode::getChildren()
{
	return this->children;
}

void MGD::BTree::CompositeNode::clearChildren(const bool cleanUp)
{
	if (cleanUp)
	{
		for (auto child : this->children)
		{
			if (child != nullptr)
			{
				delete child;
			}
		}
	}

	this->children.clear();
}

void MGD::BTree::CompositeNode::setMaxChildren(const int maxChildren, const bool cleanUpRemains)
{
	//can't be 0
	if (maxChildren == 0)
	{
		return;
	}

	//make infinite
	if (maxChildren == MGD::BTree::INFINITE_CHILDREN)
	{
		this->maxChildren = maxChildren;
		return;
	}

	//always can increase size without issue
	if (maxChildren >= this->maxChildren)
	{
		this->maxChildren = maxChildren;
		return;
	}

	//can be resized without resizing children vector
	int currentChildrenSize = static_cast<int>(this->children.size());
	if (currentChildrenSize <= maxChildren)
	{
		this->maxChildren = maxChildren;
		return;
	}
	
	//new size is not infiite, 0, or bigger than current size.
	size_t size = static_cast<size_t>(maxChildren);

	std::vector<MGD::BTree::Node*> newChildren(this->children.begin(), this->children.begin() + size);
	std::vector<MGD::BTree::Node*> remaining(this->children.begin() + size, this->children.end());

	if (cleanUpRemains)
	{
		for (auto node : remaining)
		{
			if (node != nullptr)
			{
				delete node;
			}
		}
	}

	this->children = newChildren;
}

MGD::BTree::CompositeNode::~CompositeNode()
{
	for (auto child : this->children)
	{
		if (child != nullptr)
		{
			delete child;
		}
	}
}



MGD::BTree::Selector::Selector(Node* child) : MGD::BTree::CompositeNode(child) {}

MGD::BTree::Selector::Selector(const std::vector<Node*>& children) : MGD::BTree::CompositeNode(children) {}

MGD::BTree::Selector::~Selector() {}

const MGD::BTree::NODE_STATUS MGD::BTree::Selector::update(const float delta)
{
	int start = 0;
	int size = static_cast<int>(this->children.size());

	//check if this selector has running child and make sure it's in the range
	if (this->runningChildIndex >= 0 && this->runningChildIndex < size)
	{
		//has running child
		MGD::BTree::NODE_STATUS status = this->children.at(this->runningChildIndex)->update(delta);

		if (status == MGD::BTree::NODE_STATUS::RUNNING)
		{
			//still running
			return status;
		}
		else if (status == MGD::BTree::NODE_STATUS::SUCCESS)
		{
			//success. remove running child
			this->runningChildIndex = -1;
			//because it's selector, end ehre
			return status;
		}
		else
		{
			//else, it failed or invalid. keep continue if there's more
			start = runningChildIndex;
		}
	}
	//else, there was no running child

	//if there was a running child and it failed, continue from it and execute next child if there's any
	for (int i = start; i < size; i++)
	{
		if (this->children.at(i) != nullptr)
		{
			MGD::BTree::NODE_STATUS status = this->children.at(i)->update(delta);
			if (status != MGD::BTree::NODE_STATUS::FAILURE)
			{
				if (status == MGD::BTree::NODE_STATUS::RUNNING)
				{
					//Set this node as running child
					this->runningChildIndex = i;
				}

				return status;
			}
			else
			{
				continue;
			}
		}
	}

	return MGD::BTree::NODE_STATUS::FAILURE;
}

MGD::BTree::Selector* MGD::BTree::Selector::clone()
{
	MGD::BTree::Selector* newSelector = nullptr;

	std::vector<Node*> childrenClones;

	for (auto child : this->children)
	{
		childrenClones.push_back(child->clone());
	}

	newSelector = new Selector(childrenClones);
	
	return newSelector;
}




MGD::BTree::RandomSelector::RandomSelector(Node* child) : MGD::BTree::Selector(child) {}

MGD::BTree::RandomSelector::RandomSelector(const std::vector<Node*>& children) : MGD::BTree::Selector(children) {}

MGD::BTree::RandomSelector::~RandomSelector() {}

const MGD::BTree::NODE_STATUS MGD::BTree::RandomSelector::update(const float delta)
{
	//No need to shuffle children if there's only one child
	if (this->children.size() > 1 && this->runningChildIndex < 0)
	{
		auto engine = std::default_random_engine{};
		std::shuffle(std::begin(this->children), std::end(this->children), engine);
	}
	
	return MGD::BTree::Selector::update(delta);
}

MGD::BTree::RandomSelector* MGD::BTree::RandomSelector::clone()
{
	return static_cast<MGD::BTree::RandomSelector*>(MGD::BTree::Selector::clone());
}




MGD::BTree::Sequence::Sequence(Node* child) : MGD::BTree::CompositeNode(child)
{}

MGD::BTree::Sequence::Sequence(const std::vector<Node*>& children) : MGD::BTree::CompositeNode(children)
{}

MGD::BTree::Sequence::~Sequence() {}

const MGD::BTree::NODE_STATUS MGD::BTree::Sequence::update(const float delta)
{
	int start = 0;
	int size = static_cast<int>(this->children.size());

	//check if this selector has running child and make sure it's in the range
	if (this->runningChildIndex >= 0 && this->runningChildIndex < size)
	{
		//has running child
		MGD::BTree::NODE_STATUS status = this->children.at(this->runningChildIndex)->update(delta);
		if (status != MGD::BTree::NODE_STATUS::RUNNING)
		{
			//not running anymore. Clear index.
			this->runningChildIndex = -1;
		}
		//else, status was not running anymore. Either success, failure or invalid
		else if (status != MGD::BTree::NODE_STATUS::SUCCESS)
		{
			//if was not success, which means it failed or invalid. End seqeunce.
			return status;
		}
		//else, it succeeded
		start = runningChildIndex;
	}
	//else, there was no running child

	//if there was a running child and it succeded, continue from it and execute next child if there's any
	for (int i = start; i < size; i++)
	{
		if (this->children.at(i) != nullptr)
		{
			MGD::BTree::NODE_STATUS status = this->children.at(i)->update(delta);
			if (status != MGD::BTree::NODE_STATUS::SUCCESS)
			{
				if (status == MGD::BTree::NODE_STATUS::RUNNING)
				{
					//Set this node as running child
					this->runningChildIndex = i;
				}

				return status;
			}
			else
			{
				continue;
			}
		}
	}

	return MGD::BTree::NODE_STATUS::SUCCESS;
}

MGD::BTree::Sequence* MGD::BTree::Sequence::clone()
{
	MGD::BTree::Sequence* newSequence = nullptr;

	std::vector<Node*> childrenClones;

	for (auto child : this->children)
	{
		childrenClones.push_back(child->clone());
	}

	newSequence = new Sequence(childrenClones);

	return newSequence;
}



MGD::BTree::RandomSequence::RandomSequence(Node* child) : MGD::BTree::Sequence(child)
{}

MGD::BTree::RandomSequence::RandomSequence(const std::vector<Node*>& children) : MGD::BTree::Sequence(children)
{}

MGD::BTree::RandomSequence::~RandomSequence() {}

const MGD::BTree::NODE_STATUS MGD::BTree::RandomSequence::update(const float delta)
{
	//No need to shuffle children if there's only one child.
	if (this->children.size() > 1)
	{
		auto engine = std::default_random_engine{};
		std::shuffle(std::begin(this->children), std::end(this->children), engine);
	}

	return MGD::BTree::Sequence::update(delta);
}

MGD::BTree::RandomSequence* MGD::BTree::RandomSequence::clone()
{
	return static_cast<MGD::BTree::RandomSequence*>(MGD::BTree::Sequence::clone());
}





MGD::BTree::DecoratorNode::DecoratorNode(Node* child)
{
	addChild(child);
}

MGD::BTree::DecoratorNode::~DecoratorNode()
{
	if (this->child != nullptr)
	{
		delete this->child;
	}
}

void MGD::BTree::DecoratorNode::addChild(Node* child)
{
	if (child != nullptr)
	{
		this->child = child;
	}
}

MGD::BTree::Inverter::Inverter(Node* child) : MGD::BTree::DecoratorNode(child)
{}

const MGD::BTree::NODE_STATUS MGD::BTree::Inverter::update(const float delta)
{
	MGD::BTree::NODE_STATUS status = this->child->update(delta);

	if (status == MGD::BTree::NODE_STATUS::RUNNING || status == MGD::BTree::NODE_STATUS::INVALID)
	{
		return status;
	}
	else
	{
		return status == MGD::BTree::NODE_STATUS::SUCCESS ? MGD::BTree::NODE_STATUS::FAILURE : MGD::BTree::NODE_STATUS::SUCCESS;
	}

}

MGD::BTree::Inverter* MGD::BTree::Inverter::clone()
{
	return new Inverter(this->child->clone());
}




MGD::BTree::Succeeder::Succeeder(Node* child) : MGD::BTree::DecoratorNode(child)
{}

const MGD::BTree::NODE_STATUS MGD::BTree::Succeeder::update(const float delta)
{
	this->child->update(delta);
	return MGD::BTree::NODE_STATUS::SUCCESS;
}

MGD::BTree::Succeeder* MGD::BTree::Succeeder::clone()
{
	return new Succeeder(this->child->clone());
}



MGD::BTree::Failer::Failer(Node* child) : MGD::BTree::DecoratorNode(child)
{}

const MGD::BTree::NODE_STATUS MGD::BTree::Failer::update(const float delta)
{
	this->child->update(delta);
	return MGD::BTree::NODE_STATUS::FAILURE;
}

MGD::BTree::Failer* MGD::BTree::Failer::clone()
{
	return new Failer(this->child->clone());
}




MGD::BTree::Repeater::Repeater(Node* child, const int repeat) : MGD::BTree::DecoratorNode(child), repeat(repeat)
{}

const MGD::BTree::NODE_STATUS MGD::BTree::Repeater::update(const float delta)
{
	for (int i = 0; i < repeat; i++)
	{
		MGD::BTree::NODE_STATUS status = this->child->update(delta);
		if (status == MGD::BTree::NODE_STATUS::SUCCESS || status == MGD::BTree::NODE_STATUS::FAILURE)
		{
			continue;
		}
		else
		{
			return status;
		}
	}

	return MGD::BTree::NODE_STATUS::SUCCESS;
}

MGD::BTree::Repeater* MGD::BTree::Repeater::clone()
{
	return new Repeater(this->child->clone(), this->repeat);
}




MGD::BTree::Locker::Locker(Node* child, const float duration) : MGD::BTree::DecoratorNode(child), elapsedTime(0), duration(duration) {}

const MGD::BTree::NODE_STATUS MGD::BTree::Locker::update(const float delta)
{
	//cocos2d::log("BTree::Locker");
	if (statusResult == MGD::BTree::NODE_STATUS::NONE || statusResult == MGD::BTree::NODE_STATUS::RUNNING)
	{
		//keep update node if it none(fresh execution) or running(continue updating)
		statusResult = child->update(delta);
	}

	if (statusResult == MGD::BTree::NODE_STATUS::RUNNING || statusResult == MGD::BTree::NODE_STATUS::INVALID)
	{
		//if running, keep update. If invalid, gg.
		return statusResult;
	}
	else
	{
		//result was either success or fail. Count time
		elapsedTime += delta;
		//cocos2d::log("BTree::Locker waiting... %f / %f", elapsedTime, duration);
		if (elapsedTime < duration)
		{
			//lock. Return running to keep run this node
			return MGD::BTree::NODE_STATUS::RUNNING;
		}
		else
		{
			//end.
			MGD::BTree::NODE_STATUS result = statusResult;
			reset();
			return result;
		}
	}
}

MGD::BTree::Locker* MGD::BTree::Locker::clone()
{
	return new Locker(this->child->clone(), this->duration);
}

void MGD::BTree::Locker::reset()
{
	statusResult = MGD::BTree::NODE_STATUS::NONE;
	elapsedTime = 0;
}