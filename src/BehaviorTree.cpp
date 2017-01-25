///////////////////////////////////////////////////////////////////////
// Behavior Tree
// Copyright (c) 2017 Seung Youp Baek <bsy6766@gmail.com>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any
// damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any
// purpose, including commercial applications, and to alter it and
// redistribute it freely, subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented; you
//     must not claim that you wrote the original software. If you use
//     this software in a product, an acknowledgment in the product
//     documentation would be appreciated but is not required.
//
//  2. Altered source versions must be plainly marked as such, and
//     must not be misrepresented as being the original software.
//
//  3. This notice may not be removed or altered from any source
//     distribution.
//
///////////////////////////////////////////////////////////////////////

#include "BehaviorTree.h"

BehaviorTree::Node::~Node()
{}

BehaviorTree::CompositeNode::CompositeNode() : Node(), maxChildren(BehaviorTree::INFINITE_CHILDREN), runningChildIndex(-1)
{}

BehaviorTree::CompositeNode::CompositeNode(Node* child) : Node(), maxChildren(BehaviorTree::INFINITE_CHILDREN), runningChildIndex(-1)
{
	this->addChild(child);
}

BehaviorTree::CompositeNode::CompositeNode(const std::vector<Node*>& children) : Node(), maxChildren(BehaviorTree::INFINITE_CHILDREN), runningChildIndex(-1)
{
	this->addChildren(children);
}

void BehaviorTree::CompositeNode::addChild(Node* child)
{
	if (child != nullptr)
	{
		this->children.push_back(child);
	}
}

void BehaviorTree::CompositeNode::addChildren(const std::vector<Node*>& children)
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

const std::vector<BehaviorTree::Node*>& BehaviorTree::CompositeNode::getChildren()
{
	return this->children;
}

void BehaviorTree::CompositeNode::clearChildren(const bool cleanUp)
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

void BehaviorTree::CompositeNode::setMaxChildren(const int maxChildren, const bool cleanUpRemains)
{
	//can't be 0
	if (maxChildren == 0)
	{
		return;
	}

	//make infinite
	if (maxChildren == BehaviorTree::INFINITE_CHILDREN)
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

	std::vector<BehaviorTree::Node*> newChildren(this->children.begin(), this->children.begin() + size);
	std::vector<BehaviorTree::Node*> remaining(this->children.begin() + size, this->children.end());

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

BehaviorTree::CompositeNode::~CompositeNode()
{
	for (auto child : this->children)
	{
		if (child != nullptr)
		{
			delete child;
		}
	}
}



BehaviorTree::Selector::Selector(Node* child) : BehaviorTree::CompositeNode(child) {}

BehaviorTree::Selector::Selector(const std::vector<Node*>& children) : BehaviorTree::CompositeNode(children) {}

BehaviorTree::Selector::~Selector() {}

const BehaviorTree::NODE_STATUS BehaviorTree::Selector::update(const float delta)
{
	int start = 0;
	int size = static_cast<int>(this->children.size());

	//check if this selector has running child and make sure it's in the range
	if (this->runningChildIndex >= 0 && this->runningChildIndex < size)
	{
		//has running child
		BehaviorTree::NODE_STATUS status = this->children.at(this->runningChildIndex)->update(delta);

		if (status == BehaviorTree::NODE_STATUS::RUNNING)
		{
			//still running
			return status;
		}
		else if (status == BehaviorTree::NODE_STATUS::SUCCESS)
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
			BehaviorTree::NODE_STATUS status = this->children.at(i)->update(delta);
			if (status != BehaviorTree::NODE_STATUS::FAILURE)
			{
				if (status == BehaviorTree::NODE_STATUS::RUNNING)
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

	return BehaviorTree::NODE_STATUS::FAILURE;
}

BehaviorTree::Selector* BehaviorTree::Selector::clone()
{
	BehaviorTree::Selector* newSelector = nullptr;

	std::vector<Node*> childrenClones;

	for (auto child : this->children)
	{
		childrenClones.push_back(child->clone());
	}

	newSelector = new Selector(childrenClones);
	
	return newSelector;
}




BehaviorTree::RandomSelector::RandomSelector(Node* child) : BehaviorTree::Selector(child) {}

BehaviorTree::RandomSelector::RandomSelector(const std::vector<Node*>& children) : BehaviorTree::Selector(children) {}

BehaviorTree::RandomSelector::~RandomSelector() {}

const BehaviorTree::NODE_STATUS BehaviorTree::RandomSelector::update(const float delta)
{
	//No need to shuffle children if there's only one child
	if (this->children.size() > 1 && this->runningChildIndex < 0)
	{
		auto engine = std::default_random_engine{};
		std::shuffle(std::begin(this->children), std::end(this->children), engine);
	}
	
	return BehaviorTree::Selector::update(delta);
}

BehaviorTree::RandomSelector* BehaviorTree::RandomSelector::clone()
{
	return static_cast<BehaviorTree::RandomSelector*>(BehaviorTree::Selector::clone());
}




BehaviorTree::Sequence::Sequence(Node* child) : BehaviorTree::CompositeNode(child)
{}

BehaviorTree::Sequence::Sequence(const std::vector<Node*>& children) : BehaviorTree::CompositeNode(children)
{}

BehaviorTree::Sequence::~Sequence() {}

const BehaviorTree::NODE_STATUS BehaviorTree::Sequence::update(const float delta)
{
	int start = 0;
	int size = static_cast<int>(this->children.size());

	//check if this selector has running child and make sure it's in the range
	if (this->runningChildIndex >= 0 && this->runningChildIndex < size)
	{
		//has running child
		BehaviorTree::NODE_STATUS status = this->children.at(this->runningChildIndex)->update(delta);
		if (status != BehaviorTree::NODE_STATUS::RUNNING)
		{
			//not running anymore. Clear index.
			this->runningChildIndex = -1;
		}
		//else, status was not running anymore. Either success, failure or invalid
		else if (status != BehaviorTree::NODE_STATUS::SUCCESS)
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
			BehaviorTree::NODE_STATUS status = this->children.at(i)->update(delta);
			if (status != BehaviorTree::NODE_STATUS::SUCCESS)
			{
				if (status == BehaviorTree::NODE_STATUS::RUNNING)
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

	return BehaviorTree::NODE_STATUS::SUCCESS;
}

BehaviorTree::Sequence* BehaviorTree::Sequence::clone()
{
	BehaviorTree::Sequence* newSequence = nullptr;

	std::vector<Node*> childrenClones;

	for (auto child : this->children)
	{
		childrenClones.push_back(child->clone());
	}

	newSequence = new Sequence(childrenClones);

	return newSequence;
}



BehaviorTree::RandomSequence::RandomSequence(Node* child) : BehaviorTree::Sequence(child)
{}

BehaviorTree::RandomSequence::RandomSequence(const std::vector<Node*>& children) : BehaviorTree::Sequence(children)
{}

BehaviorTree::RandomSequence::~RandomSequence() {}

const BehaviorTree::NODE_STATUS BehaviorTree::RandomSequence::update(const float delta)
{
	//No need to shuffle children if there's only one child.
	if (this->children.size() > 1)
	{
		auto engine = std::default_random_engine{};
		std::shuffle(std::begin(this->children), std::end(this->children), engine);
	}

	return BehaviorTree::Sequence::update(delta);
}

BehaviorTree::RandomSequence* BehaviorTree::RandomSequence::clone()
{
	return static_cast<BehaviorTree::RandomSequence*>(BehaviorTree::Sequence::clone());
}





BehaviorTree::DecoratorNode::DecoratorNode(Node* child)
{
	addChild(child);
}

BehaviorTree::DecoratorNode::~DecoratorNode()
{
	if (this->child != nullptr)
	{
		delete this->child;
	}
}

void BehaviorTree::DecoratorNode::addChild(Node* child)
{
	if (child != nullptr)
	{
		this->child = child;
	}
}

BehaviorTree::Inverter::Inverter(Node* child) : BehaviorTree::DecoratorNode(child)
{}

const BehaviorTree::NODE_STATUS BehaviorTree::Inverter::update(const float delta)
{
	BehaviorTree::NODE_STATUS status = this->child->update(delta);

	if (status == BehaviorTree::NODE_STATUS::RUNNING || status == BehaviorTree::NODE_STATUS::INVALID)
	{
		return status;
	}
	else
	{
		return status == BehaviorTree::NODE_STATUS::SUCCESS ? BehaviorTree::NODE_STATUS::FAILURE : BehaviorTree::NODE_STATUS::SUCCESS;
	}

}

BehaviorTree::Inverter* BehaviorTree::Inverter::clone()
{
	return new Inverter(this->child->clone());
}




BehaviorTree::Succeeder::Succeeder(Node* child) : BehaviorTree::DecoratorNode(child)
{}

const BehaviorTree::NODE_STATUS BehaviorTree::Succeeder::update(const float delta)
{
	this->child->update(delta);
	return BehaviorTree::NODE_STATUS::SUCCESS;
}

BehaviorTree::Succeeder* BehaviorTree::Succeeder::clone()
{
	return new Succeeder(this->child->clone());
}



BehaviorTree::Failer::Failer(Node* child) : BehaviorTree::DecoratorNode(child)
{}

const BehaviorTree::NODE_STATUS BehaviorTree::Failer::update(const float delta)
{
	this->child->update(delta);
	return BehaviorTree::NODE_STATUS::FAILURE;
}

BehaviorTree::Failer* BehaviorTree::Failer::clone()
{
	return new Failer(this->child->clone());
}




BehaviorTree::Repeater::Repeater(Node* child, const int repeat) : BehaviorTree::DecoratorNode(child), repeat(repeat)
{}

const BehaviorTree::NODE_STATUS BehaviorTree::Repeater::update(const float delta)
{
	for (int i = 0; i < repeat; i++)
	{
		BehaviorTree::NODE_STATUS status = this->child->update(delta);
		if (status == BehaviorTree::NODE_STATUS::SUCCESS || status == BehaviorTree::NODE_STATUS::FAILURE)
		{
			continue;
		}
		else
		{
			return status;
		}
	}

	return BehaviorTree::NODE_STATUS::SUCCESS;
}

BehaviorTree::Repeater* BehaviorTree::Repeater::clone()
{
	return new Repeater(this->child->clone(), this->repeat);
}




BehaviorTree::Locker::Locker(Node* child, const float duration) : BehaviorTree::DecoratorNode(child), elapsedTime(0), duration(duration) {}

const BehaviorTree::NODE_STATUS BehaviorTree::Locker::update(const float delta)
{
	//cocos2d::log("BehaviorTree::Locker");
	if (statusResult == BehaviorTree::NODE_STATUS::NONE || statusResult == BehaviorTree::NODE_STATUS::RUNNING)
	{
		//keep update node if it none(fresh execution) or running(continue updating)
		statusResult = child->update(delta);
	}

	if (statusResult == BehaviorTree::NODE_STATUS::RUNNING || statusResult == BehaviorTree::NODE_STATUS::INVALID)
	{
		//if running, keep update. If invalid, gg.
		return statusResult;
	}
	else
	{
		//result was either success or fail. Count time
		elapsedTime += delta;
		//cocos2d::log("BehaviorTree::Locker waiting... %f / %f", elapsedTime, duration);
		if (elapsedTime < duration)
		{
			//lock. Return running to keep run this node
			return BehaviorTree::NODE_STATUS::RUNNING;
		}
		else
		{
			//end.
			BehaviorTree::NODE_STATUS result = statusResult;
			reset();
			return result;
		}
	}
}

BehaviorTree::Locker* BehaviorTree::Locker::clone()
{
	return new Locker(this->child->clone(), this->duration);
}

void BehaviorTree::Locker::reset()
{
	statusResult = BehaviorTree::NODE_STATUS::NONE;
	elapsedTime = 0;
}