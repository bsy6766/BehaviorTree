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

#ifndef BEHAVIOR_TREE_H
#define BEHAVIOR_TREE_H

#include <vector>
#include <algorithm>
#include <random>

/**
*	@mainpage Behavior Tree Documentaion
*	@brief Simple implemetation of Behavior Tree in C++.
*	@details Provides composite nodes and decorator nodes. Inherit decorator nodes to build your own AI.
*	@author Seung Youp Baek
*	@date 01/22/2017
*	@version 1.0
*/

/**
*	@namespace BehaviorTree
*/
namespace BehaviorTree
{
	// Constant variables
	const static int INFINITE_CHILDREN = -1;
	const static int NO_RUNNING_CHILD = -1;

	/**
	*	Set this true to ignore ERROR status while updating beahvior tree. 
	*	If it's true, ERROR will be same as FAILURE status in Selector and Random Selector.
	*	If it's true, ERROR will be same as SUCCESS status in Sequence and Random Sequence.
	*	Set this false to behavior tree terminate when ERROR is occured.
	*	True by default.
	*/
	bool IGNORE_ERROR = true;

	/**
	*	@enum BehaviorTree::NODE_STATUS
	*	@brief Status of node.
	*/
	enum class NODE_STATE
	{
		SUCCESS = 0,
		FAILURE,
		RUNNING,
		ERROR
	};


	/**
	*	@class Node
	*	@brief A default abstract class that can be inherit by user to implement own behavior.
	*
	*	Node usually can be used as an either action or contidition.
	*/
	class Node
	{
	public:
		//Default constructor
		Node() = default;

		//Disable copy constructor
		Node(Node const&) = delete;

		//Disable assign operator
		void operator=(Node const&) = delete;

		//default virtual destructor
		virtual ~Node() = default;

		/**
		*	@name update
		*	@brief Updates the node.
		*	@note Purevirtual function.
		*
		*	@param const float delta = 0 An elapsed time for current frame.
		*
		*	@return NODE_STATUS::SUCCESS Node succesfully executed.
		*	@return NODE_STATUS::FAILURE Node failed to execute.
		*	@return NODE_STATUS::RUNNING Node still waiting for goal.
		*	@return NODE_STATUS::ERROR Error occured.
		*/
		virtual const NODE_STATE update(const float delta = 0) = 0;

		/**
		*	@name clone
		*	@brief Clone node.
		virtual Node* clone() { return nullptr; };
		*/

		/**
		*	@name reset
		*	@brief Reset node
		*/
		virtual void reset() { return; };
	};

	/**
	*	@class CompositeNode
	*	@brief Provides a rule for children, such as how and when exactly child gets executed.
	*
	*	CompositeNode can be @Selector, @RandomSelector, @Sequence, @RandomSequence
	*/
	class CompositeNode : public Node
	{
	protected:
		//Default constructor
		CompositeNode();

		/**
		*	@name CompositeNode
		*	@brief Private CompositeNode constructor with single child node. @Node is ignored if it's a nullptr.
		*
		*	@param child A child node to initialize with.
		*/
		CompositeNode(Node* child);

		/**
		*	@name CompositeNode
		*	@brief CompositeNode constructor with multiple child node. Node is ignored if it's a nullptr.
		*
		*	@param child A child node to initialize with.
		*/
		CompositeNode(const std::vector<Node*>& children);

		//Disable copy constructor
		CompositeNode(CompositeNode const&) = delete;

		//Disable assign operator
		void operator=(CompositeNode const&) = delete;

		//Maximum number of children that this node can take. -1 if it's infinite. Can not be 0.
		int maxChildrenSize;

		//child node that is running. nullptr if none running.
		int runningChildIndex;

		// Check if running child index is valid.
		const bool isRunningChildIndexValid();

		//Holds the children. 
		std::vector<Node*> children;

		/**
		*	@name updateRunningChild
		*	@brief Update running child if there is one.
		*	@return
		*/
		const BehaviorTree::NODE_STATE updateRunningChild(const float delta, int& nextIndex);

		const BehaviorTree::NODE_STATE updateChildren(const int start, const float delta, const BehaviorTree::NODE_STATE continueCondition);
	public:
		//Default destructor.
		virtual ~CompositeNode();

		/**
		*	@name getChildren
		*	@brief get this node's children.
		*
		*	@return A children vector reference.
		*/
		const std::vector<Node*>& getChildren();

		/**
		*	@name addChild
		*	@brief Add a child to this node. Nothing happens if it's a nullptr.
		*
		*	@param child A child node.
		*	@return True if successfully adds child. False if fails.
		*/
		const bool addChild(Node* child);

		/**
		*	@name addChildren
		*	@brief Add children to this node. Nothing happens if it's a nullptr.
		*
		*	@param child Children nodes.
		*	@return True if successfully adds children. False if fails.
		*/
		const bool addChildren(const std::vector<Node*>& children);

		/**
		*	@name clearChildren
		*	@brief Clear all children.
		*
		*	@param cleanUp Deletes instance.
		*/
		void clearChildren(const bool cleanUp = true);

		/**
		*	@name setMaxChildren
		*	@brief Set maximum number of children that this node can take.
		*
		*	@param maxChildren Number of children that this node can take. Infinit if it's -1.
		*	@param cleanUpRemains Deletes remaing children instances if exists. Else, just removes it from children vector.
		*	@return True if successfully sets maximum child size. 
		*/
		const bool setMaxChildrenSize(const int maxChildrenSize, const bool cleanUpRemains);
	};

	/**
	*	@class Selector
	*	@brief Executes every child until anything successes.
	*
	*	@details Selector executes children nodes sequently unitl one of them returns SUCCESS or RUNNING.
	*/
	class Selector : public CompositeNode
	{
	public:
		/**
		*	@name Selector
		*	@brief Private Selector constructor with single child node. Node is ignored if it's a nullptr.
		*
		*	@param child A child node to initialize with.
		*/
		Selector(Node* child);

		/**
		*	@name Selector
		*	@brief Selector constructor with multiple child node. Node is ignored if it's a nullptr.
		*
		*	@param child A child node to initialize with.
		*/
		Selector(const std::vector<Node*>& children);

		//Disable copy constructor
		Selector(Selector const&) = delete;

		//Disable assign operator
		void operator=(Selector const&) = delete;

		//Default destructor
		~Selector();

		// @copydoc Node::update(const float delta = 0)
		virtual const NODE_STATE update(const float delta = 0) override;
	};

	/**
	*	@class RandomSelector
	*	@brief Exevutes every child randomly until anything successes.
	*
	*	@details Shuffles children on every execution. Not applied if there's only one child.
	*/
	class RandomSelector : public Selector
	{
	public:
		/**
		*	@name RandomSelector
		*	@brief RandomSelector constructor with single child node. Node is ignored if it's a nullptr.
		*
		*	@param child A child node to initialize with.
		*/
		RandomSelector(Node* child);

		/**
		*	@name RandomSelector
		*	@brief RandomSelector constructor with multiple child node. Node is ignored if it's a nullptr.
		*
		*	@param child A child node to initialize with.
		*/
		RandomSelector(const std::vector<Node*>& children);

		//Default destructor
		~RandomSelector();

		// @copydoc Node::update(const float delta = 0)
		virtual const NODE_STATE update(const float delta = 0) override;
	};

	/**
	*	@@class Sequence
	*	@brief Executes every child in order and only continues execution only if child successes.
	*/
	class Sequence : public CompositeNode
	{
	public:
		/**
		*	@name Sequence
		*	@brief Sequence constructor with single child node. Node is ignored if it's a nullptr.
		*
		*	@param child A child node to initialize with.
		*/
		Sequence(Node* child);

		/**
		*	@name Sequence
		*	@brief Sequence constructor with multiple child node. Node is ignored if it's a nullptr.
		*
		*	@param child A child node to initialize with.
		*/
		Sequence(const std::vector<Node*>& children);

		//Default destructor
		~Sequence();

		// @copydoc Node::update(const float delta = 0)
		virtual const NODE_STATE update(const float delta = 0) override;
	};

	/**
	*	@@class RandomSequence
	*	@brief Executes every child randomly and only continues execution if child successes.
	*
	*	Shuffles children on every execution. Not applied if there's only one child.
	*/
	class RandomSequence : public Sequence
	{
	public:
		/**
		*	@name RandomSequence
		*	@brief RandomSequence constructor with single child node. Node is ignored if it's a nullptr.
		*
		*	@param child A child node to initialize with.
		*/
		RandomSequence(Node* child);

		/**
		*	@name RandomSequence
		*	@brief RandomSequence constructor with multiple child node. Node is ignored if it's a nullptr.
		*
		*	@param child A child node to initialize with.
		*/
		RandomSequence(const std::vector<Node*>& children);

		//Default destructor
		~RandomSequence();

		// @copydoc Node::update(const float delta = 0)
		virtual const NODE_STATE update(const float delta = 0) override;
	};

	/**
	*	@@class DecoratorNode
	*	@brief An abstract class wraps existing node and modifies the behavior or result.
	*/
	class DecoratorNode : public Node
	{
	protected:
		/**
		*	@name DecoratorNode
		*	@brief DecoratorNode constructor
		*
		*	@param child A node to initialize with.
		*	@param overwrite Deletes existing child and overwrite with new one. True by default.
		*/
		DecoratorNode(Node* child);

		//A node that is wrapped
		Node* child;
	public:
		//Disable copy constructor
		DecoratorNode(DecoratorNode const&) = delete;

		//Disable assign operator
		void operator=(DecoratorNode const&) = delete;

		//Default destructor
		virtual ~DecoratorNode();

		/**
		*	@name addChild
		*	@brief Adds child node to wrap.
		*
		*	@param child A node to wrap. Ignored if there is an existing child.
		*/
		void addChild(Node* child, const bool overwrite = true);
	};

	/**
	*	@@class Inverter
	*	@brief Inverts the result of node if status is SUCCESS or FIALURE.
	*/
	class Inverter : public DecoratorNode
	{
	public:
		/**
		*	@name Inverter
		*	@brief Constructor
		*	@note nullptr child will return ERROR.
		*/
		Inverter();

		/**
		*	@name Inverter
		*	@brief Constructor with child
		*	@param child A child node to decorate.
		*/
		Inverter(Node* child);

		//Disable copy constructor
		Inverter(Inverter const&) = delete;

		//Disable assign operator
		void operator=(Inverter const&) = delete;

		// Default destructor
		~Inverter() = default;

		// @copydoc Node::update(const float delta = 0)
		virtual const NODE_STATE update(const float delta = 0) override;
	};

	/**
	*	@@class Succeeder
	*	@brief Node always successes
	*	@note nullptr child will return ERROR.
	*/
	class Succeeder : public DecoratorNode
	{
	public:
		/**
		*	@name Succeeder
		*	@brief Succeeder constructor
		*	@note 
		*/
		Succeeder();

		/**
		*	@name Succeeder
		*	@brief Succeeder constructor
		*
		*	@param child A child node to decorate.
		*/
		Succeeder(Node* child);

		//Disable copy constructor
		Succeeder(Succeeder const&) = delete;

		//Disable assign operator
		void operator=(Succeeder const&) = delete;

		// Default destructor
		~Succeeder() = default;

		// @copydoc Node::update(const float delta = 0)
		virtual const NODE_STATE update(const float delta = 0) override;
	};

	/**
	*	@@class Failer
	*	@brief Node always fails
	*/
	class Failer : public DecoratorNode
	{
	public:
		/**
		*	@name Failer
		*	@brief Failer constructor
		*	@note nullptr child will return ERROR
		*/
		Failer();

		/**
		*	@name Failer
		*	@brief Failer constructor
		*	@param child A child node to decorate.
		*/
		Failer(Node* child);

		//Disable copy constructor
		Failer(Failer const&) = delete;

		//Disable assign operator
		void operator=(Failer const&) = delete;

		// Default destructor
		~Failer() = default;

		// @copydoc Node::update(const float delta = 0)
		virtual const NODE_STATE update(const float delta = 0) override;
	};

	/**
	*	@@class Repeat
	*	@brief Base class for repeating nodes.
	*/
	class Repeat : public DecoratorNode
	{
	public:
		const static int REPEAT_INFINITE = -1;
	protected:
		/**
		*	@name Repeat
		*/
		Repeat(const int repeat);

		/**
		*	@name Repeat
		*	@brief Constructor with child
		*	@param child A child node to decorate.
		*	@param repeat Amount of number to repeat this node.
		*/
		Repeat(Node* child, const int repeat);

		//Disable copy constructor
		Repeat(Repeat const&) = delete;

		//Disable assign operator
		void operator=(Repeat const&) = delete;

		// Virtual destructor
		virtual ~Repeat() = default;

		//amount of number to repeat. 
		int repeat;

		// set repeat. Beaware that large amount of repeat might slow down your application.
		void setRepeat(const int repeat);
	};

	/**
	*	@@class Repeater
	*	@brief Node repeats for certain time if node successes or fails. Repeat stops if node Returns true after repeating.
	*	@note Returns ERROR if repeats for 0 times.
	*/
	class Repeater : public Repeat
	{
	public:
		/**
		*	@name Failer
		*	@brief Failer constructor
		*	@note nullptr child will return ERROR
		*/
		Repeater(const int repeat);

		/**
		*	@name Repeater
		*	@brief Repeater constructor
		*	@param child A child node to decorate.
		*	@param repeat Amount of number to repeat this node.
		*/
		Repeater(Node* child, const int repeat);

		//Disable copy constructor
		Repeater(Repeater const&) = delete;

		//Disable assign operator
		void operator=(Repeater const&) = delete;

		// Default destructor
		~Repeater() = default;

		// @copydoc Node::update(const float delta = 0)
		virtual const NODE_STATE update(const float delta = 0) override;
	};

	/**
	*	@@class Repeater
	*	@brief Repeats update until it meets desired status as result.  
	*	@note Returns ERROR if repeats for 0 times. If it repeats infinitely, it can fall into infitie loop. Use thread to avoid if you need.
	*/
	class RepeatUntil : public Repeat
	{
	protected:
		// status that repeat until wants
		NODE_STATE desiredStatus;

		/**
		*	@name RepeatUntil
		*/
		RepeatUntil(const int repeat, const NODE_STATE conditionStatus);

		/**
		*	@name RepeatUntil
		*	@brief Constructor with child
		*	@param child A child node to decorate.
		*	@param repeat Amount of number to repeat this node.
		*/
		RepeatUntil(Node* child, const int repeat, const NODE_STATE conditionStatus);

		//Disable copy constructor
		RepeatUntil(RepeatUntil const&) = delete;

		//Disable assign operator
		void operator=(RepeatUntil const&) = delete;

		// Virtual destructor
		virtual ~RepeatUntil() = default;

		// @copydoc Node::update(const float delta = 0)
		virtual const NODE_STATE update(const float delta = 0) override;
	};

	/**
	*	@@class RepeatUntilFail
	*	@brief Node repeats for certain time until node fails. If fails, returns SUCCESS.
	*	@note Returns ERROR if repeats for 0 times. If it repeats infinitely, it can fall into infitie loop. Use thread to avoid if you need.
	*/
	class RepeatUntilFail : public RepeatUntil
	{
	public:
		/**
		*	@name RepeatUntilFail
		*	@brief RepeatUntilFail constructor
		*	@param repeat Amount of number to repeat this node.
		*/
		RepeatUntilFail(const int repeat);
		/**
		*	@name RepeatUntilFail
		*	@brief RepeatUntilFail constructor
		*	@param child A child node to decorate.
		*	@param repeat Amount of number to repeat this node.
		*/
		RepeatUntilFail(Node* child, const int repeat);

		//Disable copy constructor
		RepeatUntilFail(RepeatUntilFail const&) = delete;

		//Disable assign operator
		void operator=(RepeatUntilFail const&) = delete;

		// Default destructor
		~RepeatUntilFail() = default;
	};

	/**
	*	@@class RepeatUntilSuccess
	*	@brief Node repeats for certain time until node scucess. If scucess, returns SUCCESS.
	*	@note Returns ERROR if repeats for 0 times. If it repeats infinitely, it can fall into infitie loop. Use thread to avoid if you need.
	*/
	class RepeatUntilSuccess : public RepeatUntil
	{
	public:
		/**
		*	@name RepeatUntilSuccess
		*	@brief RepeatUntilSuccess constructor
		*	@param repeat Amount of number to repeat this node.
		*/
		RepeatUntilSuccess(const int repeat);
		/**
		*	@name RepeatUntilSuccess
		*	@brief RepeatUntilSuccess constructor
		*	@param child A child node to decorate.
		*	@param repeat Amount of number to repeat this node.
		*/
		RepeatUntilSuccess(Node* child, const int repeat);

		//Disable copy constructor
		RepeatUntilSuccess(RepeatUntilSuccess const&) = delete;

		//Disable assign operator
		void operator=(RepeatUntilSuccess const&) = delete;

		// Default destructor
		~RepeatUntilSuccess() = default;
	};

	/**
	*	@class Limiter
	*	@breif Limit the number of execution of this node. After a certain number of execution, this node will not be excuted and return FAILURE status as result always.
	*/
	class Limiter : public DecoratorNode
	{
	private:
		int limit;
		int limitCount;
	public:
		/**
		*	@name Limiter
		*	@brief Limiter constructor
		*	@note nullptr child will return ERROR
		*/
		Limiter(const int limit);

		/**
		*	@name Limiter
		*	@brief Limiter constructor with child
		*	@param child A child node to decorate.
		*	@param limit A number of limit of excution of this node
		*/
		Limiter(Node* child, const int limit);

		//Disable copy constructor
		Limiter(Limiter const&) = delete;

		//Disable assign operator
		void operator=(Limiter const&) = delete;

		// Default destructor
		~Limiter() = default;

		// @copydoc Node::update(const float delta = 0)
		virtual const NODE_STATE update(const float delta = 0) override;
	};

	/**
	*	@class DelayTime
	*	@brief Delay certain amount of time before the node gets executed on every execution.
	*	@note State of this node will be RUNNING while delaying. After delay, node will not get executed but return the state that node retruned when executed. If you want to delay only once for a lifetime, @see DelayOnce.
	*/
	class DelayTime : public DecoratorNode
	{
	private:
		float duration;
		float elapsedTime;
		bool delayOnce;
		bool childUpdateFinished;
		BehaviorTree::NODE_STATE result;
	public:
		/**
		*	@name Delay
		*	@brief Delay constructor
		*	@note nullptr child will return ERROR
		*/
		DelayTime(const float duration, const bool delayOnce);

		/**
		*	@name Delay
		*	@brief Delay constructor with child and duration
		*	@param child A child node to decorate.
		*	@param duration Time to delay.
		*/
		DelayTime(Node* child, const float duration, const bool delayOnce);

		//Disable copy constructor
		DelayTime(DelayTime const&) = delete;

		//Disable assign operator
		void operator=(DelayTime const&) = delete;

		// Default destructor
		~DelayTime() = default;

		// @copydoc Node::update(const float delta = 0)
		virtual const NODE_STATE update(const float delta = 0) override;
	};
}

#endif