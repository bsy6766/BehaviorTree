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

BTree::Node::~Node()
{}

BTree::CompositeNode::CompositeNode() : Node(), maxChildren(BTree::INFINITE_CHILDREN), runningChildIndex(-1)
{}

BTree::CompositeNode::CompositeNode(Node* child) : Node(), maxChildren(BTree::INFINITE_CHILDREN), runningChildIndex(-1)
{
	this->addChild(child);
}

BTree::CompositeNode::CompositeNode(const std::vector<Node*>& children) : Node(), maxChildren(BTree::INFINITE_CHILDREN), runningChildIndex(-1)
{
	this->addChildren(children);
}

void BTree::CompositeNode::addChild(Node* child)
{
	if (child != nullptr)
	{
		this->children.push_back(child);
	}
}

void BTree::CompositeNode::addChildren(const std::vector<Node*>& children)
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

const std::vector<BTree::Node*>& BTree::CompositeNode::getChildren()
{
	return this->children;
}

void BTree::CompositeNode::clearChildren(const bool cleanUp)
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

void BTree::CompositeNode::setMaxChildren(const int maxChildren, const bool cleanUpRemains)
{
	//can't be 0
	if (maxChildren == 0)
	{
		return;
	}

	//make infinite
	if (maxChildren == BTree::INFINITE_CHILDREN)
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

	std::vector<BTree::Node*> newChildren(this->children.begin(), this->children.begin() + size);
	std::vector<BTree::Node*> remaining(this->children.begin() + size, this->children.end());

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

BTree::CompositeNode::~CompositeNode()
{
	for (auto child : this->children)
	{
		if (child != nullptr)
		{
			delete child;
		}
	}
}



BTree::Selector::Selector(Node* child) : BTree::CompositeNode(child) {}

BTree::Selector::Selector(const std::vector<Node*>& children) : BTree::CompositeNode(children) {}

BTree::Selector::~Selector() {}

const BTree::NODE_STATUS BTree::Selector::update(const float delta)
{
	int start = 0;
	int size = static_cast<int>(this->children.size());

	//check if this selector has running child and make sure it's in the range
	if (this->runningChildIndex >= 0 && this->runningChildIndex < size)
	{
		//has running child
		BTree::NODE_STATUS status = this->children.at(this->runningChildIndex)->update(delta);

		if (status == BTree::NODE_STATUS::RUNNING)
		{
			//still running
			return status;
		}
		else if (status == BTree::NODE_STATUS::SUCCESS)
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
			BTree::NODE_STATUS status = this->children.at(i)->update(delta);
			if (status != BTree::NODE_STATUS::FAILURE)
			{
				if (status == BTree::NODE_STATUS::RUNNING)
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

	return BTree::NODE_STATUS::FAILURE;
}

BTree::Selector* BTree::Selector::clone()
{
	BTree::Selector* newSelector = nullptr;

	std::vector<Node*> childrenClones;

	for (auto child : this->children)
	{
		childrenClones.push_back(child->clone());
	}

	newSelector = new Selector(childrenClones);
	
	return newSelector;
}




BTree::RandomSelector::RandomSelector(Node* child) : BTree::Selector(child) {}

BTree::RandomSelector::RandomSelector(const std::vector<Node*>& children) : BTree::Selector(children) {}

BTree::RandomSelector::~RandomSelector() {}

const BTree::NODE_STATUS BTree::RandomSelector::update(const float delta)
{
	//No need to shuffle children if there's only one child
	if (this->children.size() > 1 && this->runningChildIndex < 0)
	{
		auto engine = std::default_random_engine{};
		std::shuffle(std::begin(this->children), std::end(this->children), engine);
	}
	
	return BTree::Selector::update(delta);
}

BTree::RandomSelector* BTree::RandomSelector::clone()
{
	return static_cast<BTree::RandomSelector*>(BTree::Selector::clone());
}




BTree::Sequence::Sequence(Node* child) : BTree::CompositeNode(child)
{}

BTree::Sequence::Sequence(const std::vector<Node*>& children) : BTree::CompositeNode(children)
{}

BTree::Sequence::~Sequence() {}

const BTree::NODE_STATUS BTree::Sequence::update(const float delta)
{
	int start = 0;
	int size = static_cast<int>(this->children.size());

	//check if this selector has running child and make sure it's in the range
	if (this->runningChildIndex >= 0 && this->runningChildIndex < size)
	{
		//has running child
		BTree::NODE_STATUS status = this->children.at(this->runningChildIndex)->update(delta);
		if (status != BTree::NODE_STATUS::RUNNING)
		{
			//not running anymore. Clear index.
			this->runningChildIndex = -1;
		}
		//else, status was not running anymore. Either success, failure or invalid
		else if (status != BTree::NODE_STATUS::SUCCESS)
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
			BTree::NODE_STATUS status = this->children.at(i)->update(delta);
			if (status != BTree::NODE_STATUS::SUCCESS)
			{
				if (status == BTree::NODE_STATUS::RUNNING)
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

	return BTree::NODE_STATUS::SUCCESS;
}

BTree::Sequence* BTree::Sequence::clone()
{
	BTree::Sequence* newSequence = nullptr;

	std::vector<Node*> childrenClones;

	for (auto child : this->children)
	{
		childrenClones.push_back(child->clone());
	}

	newSequence = new Sequence(childrenClones);

	return newSequence;
}



BTree::RandomSequence::RandomSequence(Node* child) : BTree::Sequence(child)
{}

BTree::RandomSequence::RandomSequence(const std::vector<Node*>& children) : BTree::Sequence(children)
{}

BTree::RandomSequence::~RandomSequence() {}

const BTree::NODE_STATUS BTree::RandomSequence::update(const float delta)
{
	//No need to shuffle children if there's only one child.
	if (this->children.size() > 1)
	{
		auto engine = std::default_random_engine{};
		std::shuffle(std::begin(this->children), std::end(this->children), engine);
	}

	return BTree::Sequence::update(delta);
}

BTree::RandomSequence* BTree::RandomSequence::clone()
{
	return static_cast<BTree::RandomSequence*>(BTree::Sequence::clone());
}





BTree::DecoratorNode::DecoratorNode(Node* child)
{
	addChild(child);
}

BTree::DecoratorNode::~DecoratorNode()
{
	if (this->child != nullptr)
	{
		delete this->child;
	}
}

void BTree::DecoratorNode::addChild(Node* child)
{
	if (child != nullptr)
	{
		this->child = child;
	}
}

BTree::Inverter::Inverter(Node* child) : BTree::DecoratorNode(child)
{}

const BTree::NODE_STATUS BTree::Inverter::update(const float delta)
{
	BTree::NODE_STATUS status = this->child->update(delta);

	if (status == BTree::NODE_STATUS::RUNNING || status == BTree::NODE_STATUS::INVALID)
	{
		return status;
	}
	else
	{
		return status == BTree::NODE_STATUS::SUCCESS ? BTree::NODE_STATUS::FAILURE : BTree::NODE_STATUS::SUCCESS;
	}

}

BTree::Inverter* BTree::Inverter::clone()
{
	return new Inverter(this->child->clone());
}




BTree::Succeeder::Succeeder(Node* child) : BTree::DecoratorNode(child)
{}

const BTree::NODE_STATUS BTree::Succeeder::update(const float delta)
{
	this->child->update(delta);
	return BTree::NODE_STATUS::SUCCESS;
}

BTree::Succeeder* BTree::Succeeder::clone()
{
	return new Succeeder(this->child->clone());
}



BTree::Failer::Failer(Node* child) : BTree::DecoratorNode(child)
{}

const BTree::NODE_STATUS BTree::Failer::update(const float delta)
{
	this->child->update(delta);
	return BTree::NODE_STATUS::FAILURE;
}

BTree::Failer* BTree::Failer::clone()
{
	return new Failer(this->child->clone());
}




BTree::Repeater::Repeater(Node* child, const int repeat) : BTree::DecoratorNode(child), repeat(repeat)
{}

const BTree::NODE_STATUS BTree::Repeater::update(const float delta)
{
	for (int i = 0; i < repeat; i++)
	{
		BTree::NODE_STATUS status = this->child->update(delta);
		if (status == BTree::NODE_STATUS::SUCCESS || status == BTree::NODE_STATUS::FAILURE)
		{
			continue;
		}
		else
		{
			return status;
		}
	}

	return BTree::NODE_STATUS::SUCCESS;
}

BTree::Repeater* BTree::Repeater::clone()
{
	return new Repeater(this->child->clone(), this->repeat);
}




BTree::Locker::Locker(Node* child, const float duration) : BTree::DecoratorNode(child), elapsedTime(0), duration(duration) {}

const BTree::NODE_STATUS BTree::Locker::update(const float delta)
{
	//cocos2d::log("BTree::Locker");
	if (statusResult == BTree::NODE_STATUS::NONE || statusResult == BTree::NODE_STATUS::RUNNING)
	{
		//keep update node if it none(fresh execution) or running(continue updating)
		statusResult = child->update(delta);
	}

	if (statusResult == BTree::NODE_STATUS::RUNNING || statusResult == BTree::NODE_STATUS::INVALID)
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
			return BTree::NODE_STATUS::RUNNING;
		}
		else
		{
			//end.
			BTree::NODE_STATUS result = statusResult;
			reset();
			return result;
		}
	}
}

BTree::Locker* BTree::Locker::clone()
{
	return new Locker(this->child->clone(), this->duration);
}

void BTree::Locker::reset()
{
	statusResult = BTree::NODE_STATUS::NONE;
	elapsedTime = 0;
}