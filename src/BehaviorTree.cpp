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

BehaviorTree::CompositeNode::CompositeNode() : Node(), maxChildrenSize(BehaviorTree::INFINITE_CHILDREN), runningChildIndex(BehaviorTree::NO_RUNNING_CHILD)
{}

BehaviorTree::CompositeNode::CompositeNode(BehaviorTree::Node* child) : Node(), maxChildrenSize(BehaviorTree::INFINITE_CHILDREN), runningChildIndex(BehaviorTree::NO_RUNNING_CHILD)
{
	this->addChild(child);
}

BehaviorTree::CompositeNode::CompositeNode(const std::vector<Node*>& children) : Node(), maxChildrenSize(BehaviorTree::INFINITE_CHILDREN), runningChildIndex(BehaviorTree::NO_RUNNING_CHILD)
{
	this->addChildren(children);
}

const bool BehaviorTree::CompositeNode::addChild(BehaviorTree::Node* child)
{
	if (child != nullptr)
	{
		// Add to children
		this->children.push_back(child);
		return true;
	}
	else
	{
		return false;
	}
}

const bool BehaviorTree::CompositeNode::addChildren(const std::vector<Node*>& children)
{
	if (children.empty())
	{
		// Empty children. Reject
		return false;
	}

	if (this->maxChildrenSize != BehaviorTree::INFINITE_CHILDREN)
	{
		// There is maximum children can be added. Get current children size
		int currentChildrenSize = static_cast<int>(this->children.size());
		// Get size of children that need to be added
		int addingChildrenSize = static_cast<int>(children.size());

		if (currentChildrenSize + addingChildrenSize > this->maxChildrenSize)
		{
			// Total size after adding children exceeds maximu children size. Reject.
			return false;
		}
	}

	for (auto child : children)
	{
		// Add to children. No need to check 
		this->addChild(child);
	}

	return true;
}

const std::vector<BehaviorTree::Node*>& BehaviorTree::CompositeNode::getChildren()
{
	// Return children as reference
	return this->children;
}

void BehaviorTree::CompositeNode::clearChildren(const bool cleanUp)
{
	if (cleanUp)
	{
		// Delete all children instances.
		for (auto child : this->children)
		{
			if (child != nullptr)
			{
				delete child;
				child = nullptr;
			}
		}
	}

	// Clear vector
	this->children.clear();
}

const bool BehaviorTree::CompositeNode::setMaxChildrenSize(const int maxChildrenSize, const bool cleanUpRemains)
{
	//can't be 0
	if (maxChildrenSize == 0)
	{
		return false;
	}

	// Get current children size
	const int currentChildrenSize = static_cast<int>(this->children.size());
	if (maxChildrenSize == currentChildrenSize)
	{
		// Setting to same size. 
		return true;
	}

	//make infinite. Always valid operation.
	if (maxChildrenSize == BehaviorTree::INFINITE_CHILDREN)
	{
		this->maxChildrenSize = BehaviorTree::INFINITE_CHILDREN;
		return true;
	}

	//Check if new max children size is larger than current max children size
	if (maxChildrenSize >= this->maxChildrenSize)
	{
		// Valid. Set new max children size.
		this->maxChildrenSize = maxChildrenSize;
		return true;
	}

	// Check if max children size is current children size
	if (currentChildrenSize < maxChildrenSize)
	{
		//can be resized without resizing children vector
		this->maxChildrenSize = maxChildrenSize;
		return true;
	}
	
	//new maximum children size is not infiite, 0, or bigger and equal than current size.
	if (cleanUpRemains)
	{
		// Clean up reamaining nodes before resizing.
		int i = maxChildrenSize;
		int childrenSize = static_cast<int>(this->children.size());
		for (; i < childrenSize; i)
		{
			if (this->children.at(i) != nullptr)
			{
				delete this->children.at(i);
			}
		}
	}

	// Resize
	this->children.resize(maxChildrenSize);
}

const bool BehaviorTree::CompositeNode::isRunningChildIndexValid()
{
	int size = static_cast<int>(this->children.size());
	return (this->runningChildIndex < size) && (this->runningChildIndex != BehaviorTree::NO_RUNNING_CHILD);
}

const BehaviorTree::NODE_STATE BehaviorTree::CompositeNode::updateRunningChild(const float delta, int& start)
{
	int size = static_cast<int>(this->children.size());

	//check if this selector has running child and make sure it's in the range
	if (this->runningChildIndex < size)
	{
		// This selector has running child and it's in range of children size.
		// Update running node and get result.
		BehaviorTree::NODE_STATE state = this->children.at(this->runningChildIndex)->update(delta);

		if (state == BehaviorTree::NODE_STATE::RUNNING)
		{
			// Still running. Keep go on.
			return state;
		}
		else if (state == BehaviorTree::NODE_STATE::SUCCESS)
		{
			// Success. Reset running child index.
			this->runningChildIndex = BehaviorTree::NO_RUNNING_CHILD;
			// Because it's selector, ends here.
			return state;
		}
		else
		{
			// Else, status is FAILURE or ignorable ERROR.
			// Set start index after the running child index.
			start = this->runningChildIndex + 1;
			// Reset running child index
			this->runningChildIndex = BehaviorTree::NO_RUNNING_CHILD;
		}
	}
	// else, there was no running child
	return BehaviorTree::NODE_STATE::FAILURE;
}

const BehaviorTree::NODE_STATE BehaviorTree::CompositeNode::updateChildren(const int start, const float delta, const BehaviorTree::NODE_STATE continueCondition)
{
	// If running childe node returned FAILURE or ERROR, keep go on.
	int size = static_cast<int>(this->children.size());
	for (int i = start; i < size; i++)
	{
		if (this->children.at(i) != nullptr)
		{
			// Update node
			BehaviorTree::NODE_STATE state = this->children.at(i)->update(delta);
			// Check status
			if (state == continueCondition)
			{
				// Failed. Proceed to next 
				continue;
			}
			else if (state == BehaviorTree::NODE_STATE::ERROR)
			{
				// Error occured.
				if (BehaviorTree::IGNORE_ERROR)
				{
					// Can ignore error. 
					continue;
				}
				else
				{
					// Terminate. Return ERROR.
					return state;
				}
			}
			else
			{
				// Success or Running
				if (state == BehaviorTree::NODE_STATE::RUNNING)
				{
					// Set this node as running child
					this->runningChildIndex = i;
				}
				// Else if status was SUCCESS, return.
				return state;
			}
		}
	}

	// Everything failed.
	return BehaviorTree::NODE_STATE::FAILURE;
}

BehaviorTree::CompositeNode::~CompositeNode()
{
	// Delete children
	for (auto child : this->children)
	{
		if (child != nullptr)
		{
			delete child;
		}
	}

	this->children.clear();
}





BehaviorTree::Selector::Selector(BehaviorTree::Node* child) : BehaviorTree::CompositeNode(child) {}

BehaviorTree::Selector::Selector(const std::vector<Node*>& children) : BehaviorTree::CompositeNode(children) {}

BehaviorTree::Selector::~Selector() {}

const BehaviorTree::NODE_STATE BehaviorTree::Selector::update(const float delta)
{
	int start = 0;
	
	// Update running child if there's one.
	if (this->isRunningChildIndexValid())
	{
		const BehaviorTree::NODE_STATE runningChildstate = BehaviorTree::CompositeNode::updateRunningChild(delta, start);

		if (runningChildstate == BehaviorTree::NODE_STATE::SUCCESS || 
			runningChildstate == BehaviorTree::NODE_STATE::RUNNING)
		{
			// Stop and return
			return runningChildstate;
		}
		else if (runningChildstate == BehaviorTree::NODE_STATE::ERROR)
		{
			// It was error.
			if (BehaviorTree::IGNORE_ERROR == false)
			{
				// Can't ignore error. Return.
				return BehaviorTree::NODE_STATE::ERROR;
			}
		}
		// Else, it was either FAILURE or ERROR that can be ignored.
	}

	// Update children
	return this->updateChildren(start, delta, BehaviorTree::NODE_STATE::FAILURE);
}

/*
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
*/





BehaviorTree::RandomSelector::RandomSelector(BehaviorTree::Node* child) : BehaviorTree::Selector(child) {}

BehaviorTree::RandomSelector::RandomSelector(const std::vector<Node*>& children) : BehaviorTree::Selector(children) {}

BehaviorTree::RandomSelector::~RandomSelector() {}

const BehaviorTree::NODE_STATE BehaviorTree::RandomSelector::update(const float delta)
{
	// No need to shuffle children if there's only one child
	if (this->children.size() > 1 && this->runningChildIndex < 0)
	{
		auto engine = std::default_random_engine{};
		std::shuffle(std::begin(this->children), std::end(this->children), engine);
	}
	
	return BehaviorTree::Selector::update(delta);
}




BehaviorTree::Sequence::Sequence(BehaviorTree::Node* child) : BehaviorTree::CompositeNode(child)
{}

BehaviorTree::Sequence::Sequence(const std::vector<Node*>& children) : BehaviorTree::CompositeNode(children)
{}

BehaviorTree::Sequence::~Sequence() {}

const BehaviorTree::NODE_STATE BehaviorTree::Sequence::update(const float delta)
{
	int start = 0;

	// Update running child if there's one.
	if (this->isRunningChildIndexValid())
	{
		const BehaviorTree::NODE_STATE runningChildstate = BehaviorTree::CompositeNode::updateRunningChild(delta, start);

		if (runningChildstate == BehaviorTree::NODE_STATE::FAILURE ||
			runningChildstate == BehaviorTree::NODE_STATE::RUNNING)
		{
			// If failed, stop sequence.
			// If it's still running, keep running.
			return runningChildstate;
		}
		else if (runningChildstate == BehaviorTree::NODE_STATE::ERROR)
		{
			// Error occured
			if (BehaviorTree::IGNORE_ERROR == false)
			{
				// Can't ignore error. Return.
				return runningChildstate;
			}
			// Else, this error can be ignored and considered as SUCCESS. keep go on.
		}
		// Else, if status is SUCCESS, keep go on.
	}

	// Update children
	return this->updateChildren(start, delta, BehaviorTree::NODE_STATE::SUCCESS);
}




BehaviorTree::RandomSequence::RandomSequence(BehaviorTree::Node* child) : BehaviorTree::Sequence(child)
{}

BehaviorTree::RandomSequence::RandomSequence(const std::vector<Node*>& children) : BehaviorTree::Sequence(children)
{}

BehaviorTree::RandomSequence::~RandomSequence() {}

const BehaviorTree::NODE_STATE BehaviorTree::RandomSequence::update(const float delta)
{
	//No need to shuffle children if there's only one child.
	if (this->children.size() > 1)
	{
		auto engine = std::default_random_engine{};
		std::shuffle(std::begin(this->children), std::end(this->children), engine);
	}

	return BehaviorTree::Sequence::update(delta);
}





BehaviorTree::DecoratorNode::DecoratorNode(BehaviorTree::Node* child)
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

void BehaviorTree::DecoratorNode::addChild(BehaviorTree::Node* child, const bool overwrite)
{
	if (child == nullptr)
	{
		this->child = child;
	}
	else
	{
		if (overwrite)
		{
			delete this->child;
			this->child = nullptr;
			this->child = child;
		}
	}
}




BehaviorTree::Inverter::Inverter() : BehaviorTree::DecoratorNode(nullptr) {}

BehaviorTree::Inverter::Inverter(BehaviorTree::Node* child) : BehaviorTree::DecoratorNode(child) {}

const BehaviorTree::NODE_STATE BehaviorTree::Inverter::update(const float delta)
{
	// Update child
	if (this->child == nullptr)
	{
		// Doesn't have child. Return ERROR
		return BehaviorTree::NODE_STATE::ERROR;
	}

	BehaviorTree::NODE_STATE state = this->child->update(delta);

	if (state == BehaviorTree::NODE_STATE::RUNNING || state == BehaviorTree::NODE_STATE::ERROR)
	{
		return state;
	}
	else
	{
		// If status was SUCCESS or FAILURE, invert result.
		return state == BehaviorTree::NODE_STATE::SUCCESS ? BehaviorTree::NODE_STATE::FAILURE : BehaviorTree::NODE_STATE::SUCCESS;
	}

}





BehaviorTree::Succeeder::Succeeder() : BehaviorTree::DecoratorNode(nullptr) {}

BehaviorTree::Succeeder::Succeeder(BehaviorTree::Node* child) : BehaviorTree::DecoratorNode(child) {}

const BehaviorTree::NODE_STATE BehaviorTree::Succeeder::update(const float delta)
{
	if (this->child == nullptr)
	{
		return BehaviorTree::NODE_STATE::ERROR;
	}

	// Update child. We don't need the result
	this->child->update(delta);
	// Always return SUCCSS.
	return BehaviorTree::NODE_STATE::SUCCESS;
}





BehaviorTree::Failer::Failer() : BehaviorTree::DecoratorNode(nullptr) {}

BehaviorTree::Failer::Failer(BehaviorTree::Node* child) : BehaviorTree::DecoratorNode(child) {}

const BehaviorTree::NODE_STATE BehaviorTree::Failer::update(const float delta)
{
	if (this->child == nullptr)
	{
		return BehaviorTree::NODE_STATE::ERROR;
	}

	// Update child. We don't need the result
	this->child->update(delta);
	// Always return FAILURE.
	return BehaviorTree::NODE_STATE::FAILURE;
}




BehaviorTree::Repeat::Repeat(const int repeat) : BehaviorTree::DecoratorNode(nullptr)
{
	if (repeat < 0)
	{
		this->repeat = 0;
	}
}

BehaviorTree::Repeat::Repeat(BehaviorTree::Node* child, const int repeat) : BehaviorTree::DecoratorNode(child)
{
	if (repeat < 0)
	{
		this->repeat = 0;
	}
}

void BehaviorTree::Repeat::setRepeat(const int repeat)
{
	if (repeat < 0)
	{
		this->repeat = 0;
	}
	else
	{
		this->repeat = repeat;
	}
}




BehaviorTree::Repeater::Repeater(const int repeat) : BehaviorTree::Repeat(repeat) 
{
	if (repeat == BehaviorTree::Repeat::REPEAT_INFINITE)
	{
		// Repeater can't have infinite because it faills to infinite loop
		this->repeat = 0;
	}
}

BehaviorTree::Repeater::Repeater(BehaviorTree::Node* child, const int repeat) : BehaviorTree::Repeat(child, repeat)
{
	if (repeat == BehaviorTree::Repeat::REPEAT_INFINITE)
	{
		// Repeater can't have infinite because it faills to infinite loop
		this->repeat = 0;
	}
}

const BehaviorTree::NODE_STATE BehaviorTree::Repeater::update(const float delta)
{
	if (this->repeat == 0)
	{
		return BehaviorTree::NODE_STATE::ERROR;
	}

	// Repeat update for certain amount of times.
	for (int i = 0; i < repeat; i++)
	{
		// Update child.
		BehaviorTree::NODE_STATE state = this->child->update(delta);
		if (state == BehaviorTree::NODE_STATE::SUCCESS || state == BehaviorTree::NODE_STATE::FAILURE)
		{
			// If status was SUCCESS or FAILURE, keep go on
			continue;
		}
		else
		{
			return state;
		}
	}
	
	// Finished repeating. Return SUCCESS
	return BehaviorTree::NODE_STATE::SUCCESS;
}




BehaviorTree::RepeatUntil::RepeatUntil(const int repeat, const BehaviorTree::NODE_STATE conditionStatus) : BehaviorTree::Repeat(repeat), desiredStatus(conditionStatus) {}

BehaviorTree::RepeatUntil::RepeatUntil(BehaviorTree::Node* child, const int repeat, const BehaviorTree::NODE_STATE conditionStatus) : BehaviorTree::Repeat(child, repeat), desiredStatus(conditionStatus) {}

const BehaviorTree::NODE_STATE BehaviorTree::RepeatUntil::update(const float delta)
{
	if (this->repeat == 0)
	{
		return BehaviorTree::NODE_STATE::ERROR;
	}

	if (this->repeat == BehaviorTree::Repeat::REPEAT_INFINITE)
	{
		BehaviorTree::NODE_STATE state;
		// Run infitie loop until status is failure
		do
		{
			// Update child.
			state = this->child->update(delta);
		} while (state != this->desiredStatus);

		// Finished repeating. Return SUCCESS
		return BehaviorTree::NODE_STATE::SUCCESS;
	}
	else
	{
		// Repeat update for certain amount of times.
		for (int i = 0; i < this->repeat; i++)
		{
			// Update child.
			BehaviorTree::NODE_STATE state = this->child->update(delta);
			if (state == this->desiredStatus)
			{
				// Failed. return success.
				return BehaviorTree::NODE_STATE::SUCCESS;
			}
			// Else, continue.
		}

		// Finished repeating. Haven't failed. Return FAILURE
		return BehaviorTree::NODE_STATE::FAILURE;
	}
}






BehaviorTree::RepeatUntilFail::RepeatUntilFail(const int repeat) : BehaviorTree::RepeatUntil(repeat, BehaviorTree::NODE_STATE::FAILURE) {}

BehaviorTree::RepeatUntilFail::RepeatUntilFail(BehaviorTree::Node* child, const int repeat) : BehaviorTree::RepeatUntil(child, repeat, BehaviorTree::NODE_STATE::FAILURE) {}





BehaviorTree::RepeatUntilSuccess::RepeatUntilSuccess(const int repeat) : BehaviorTree::RepeatUntil(repeat, BehaviorTree::NODE_STATE::SUCCESS) {}

BehaviorTree::RepeatUntilSuccess::RepeatUntilSuccess(BehaviorTree::Node* child, const int repeat) : BehaviorTree::RepeatUntil(child, repeat, BehaviorTree::NODE_STATE::SUCCESS) {}




BehaviorTree::Limiter::Limiter(const int limit) : BehaviorTree::DecoratorNode(nullptr), limit(limit), limitCount(0) {}

BehaviorTree::Limiter::Limiter(BehaviorTree::Node* child, const int limit) : BehaviorTree::DecoratorNode(child), limit(limit), limitCount(0) {}

const BehaviorTree::NODE_STATE BehaviorTree::Limiter::update(const float delta)
{
	if (this->child == nullptr)
	{
		return BehaviorTree::NODE_STATE::ERROR;
	}

	if (this->limitCount < this->limit)
	{
		// Still can execute this node
		BehaviorTree::NODE_STATE state = this->child->update(delta);
		this->limitCount++;

		return state;
	}

	return BehaviorTree::NODE_STATE::FAILURE;
}




BehaviorTree::DelayTime::DelayTime(const float duration, const bool delayOnce) : BehaviorTree::DecoratorNode(nullptr), duration(duration), elapsedTime(0), delayOnce(delayOnce), childUpdateFinished(false) {}

BehaviorTree::DelayTime::DelayTime(BehaviorTree::Node* child, const float duration, const bool delayOnce) : BehaviorTree::DecoratorNode(child), duration(duration), elapsedTime(0), delayOnce(delayOnce), childUpdateFinished(false) {}

const BehaviorTree::NODE_STATE BehaviorTree::DelayTime::update(const float delta)
{
	if (this->child == nullptr)
	{
		return BehaviorTree::NODE_STATE::ERROR;
	}

	if (this->elapsedTime >= 0 && this->elapsedTime < this->duration)
	{
		// Delaying
		this->elapsedTime += delta;
		return BehaviorTree::NODE_STATE::RUNNING;
	}
	else
	{
		// Finished delaying
		if (this->childUpdateFinished == false)
		{
			// Haven't finished child update yet
			result = this->child->update(delta);

			if (result != BehaviorTree::NODE_STATE::RUNNING)
			{
				// Either SUCCESS, FAILURE or ERROR. Update finished
				this->childUpdateFinished = true;

				if(this->delayOnce == false)
				{
					// Delay again
					this->elapsedTime = 0;
					this->childUpdateFinished = false;
				}
				// Else, only delaying once.
			}
		}
		// Else, finished updating
		return result;
	}
}