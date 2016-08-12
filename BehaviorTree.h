#ifndef BEHAVIOR_TREE_H
#define BEHAVIOR_TREE_H

#include <vector>
#include <algorithm>
#include <random>

/**
*	@namespace BehaviorTree
*
*	@brief Beahvior Tree implementation with C++
*/
namespace MGD
{
	namespace BTree
	{
		const int INFINITE_CHILDREN = -1;

		enum class NODE_STATUS
		{
			NONE,
			SUCCESS,
			FAILURE,
			RUNNING,
			INVALID
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
			Node() {};

			//Disable copy constructor
			Node(Node const&) = delete;

			//Disable assign operator
			void operator=(Node const&) = delete;

			//default virtual destructor
			virtual ~Node();

			/**
			*	@name update
			*	@brief Updates the node.
			*
			*	@param const float delta = 0 An elapsed time for current frame.
			*
			*	@retval SUCCESS Node succesfully executed.
			*	@retval FAILURE Node failed to execute.
			*	@retval RUNNING Node still waiting for goal.
			*	@retval INVALID Unexpected error occured.
			*/
			virtual const NODE_STATUS update(const float delta = 0) = 0;

			/**
			*	@name clone
			*	@brief Clone node.
			*/
			virtual Node* clone() { return nullptr; };

			/**
			*	@name reset
			*	@brief Reset node
			*/
			virtual void reset() { return; };
		};

		/**
		*	@class CompositeNode
		*	@brief Provides a rule for children, such as how and when eacc child gets executed.
		*
		*	CompositeNode can be @Selector, @
		*/
		class CompositeNode : public Node
		{
		protected:
			//Default constructor
			CompositeNode();

			/**
			*	@name CompositeNode
			*	@brief Private CompositeNode constructor with single child node. Node is ignored if it's a nullptr.
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
			int maxChildren;

			//child node that is running. nullptr if none running.
			int runningChildIndex;

			//Holds the children. 
			std::vector<Node*> children;
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
			*/
			void addChild(Node* child);

			/**
			*	@name addChildren
			*	@brief Add children to this node. Nothing happens if it's a nullptr.
			*
			*	@param child Children nodes.
			*/
			void addChildren(const std::vector<Node*>& children);

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
			*/
			void setMaxChildren(const int maxChildren, const bool cleanUpRemains);
		};

		/**
		*	@class Selector
		*	@brief Executes every child until anything successes.
		*
		*	Selector executes children nodes sequently unitl one of them returns SUCCESS, RUNNING or ERROR.
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
			virtual const NODE_STATUS update(const float delta = 0) override;

			// @copydoc Node::clone()
			virtual Selector* clone() override;
		};

		/**
		*	@name RandomSelector
		*	@brief Exevutes every child randomly until anything successes.
		*
		*	Shuffles children on every execution. Not applied if there's only one child.
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
			virtual const NODE_STATUS update(const float delta = 0) override;

			// @copydoc Node::clone()
			virtual RandomSelector* clone() override;
		};

		/**
		*	@name Sequence
		*	@brief Executes every child in order and only continues execution if child successes.
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
			virtual const NODE_STATUS update(const float delta = 0) override;

			// @copydoc Node::clone()
			virtual Sequence* clone() override;
		};

		/**
		*	@name RandomSequence
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
			virtual const NODE_STATUS update(const float delta = 0) override;

			// @copydoc Node::clone()
			virtual RandomSequence* clone() override;
		};


		/**
		*	@name DecoratorNode
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
			void addChild(Node* child);
		};

		/**
		*	@name Inverter
		*	@brief Inverts the result of node
		*/
		class Inverter : public DecoratorNode
		{
		public:
			/**
			*	@name Inverter
			*	@brief Inverter constructor
			*
			*	@param child A child node to decorate.
			*/
			Inverter(Node* child);

			//Disable copy constructor
			Inverter(Inverter const&) = delete;

			//Disable assign operator
			void operator=(Inverter const&) = delete;

			// @copydoc Node::update(const float delta = 0)
			virtual const NODE_STATUS update(const float delta = 0) override;

			// @copydoc Node::clone()
			virtual Inverter* clone() override;
		};

		/**
		*	@name Succeeder
		*	@brief Node always successes
		*/
		class Succeeder : public DecoratorNode
		{
		public:
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

			// @copydoc Node::update(const float delta = 0)
			virtual const NODE_STATUS update(const float delta = 0) override;

			// @copydoc Node::clone()
			virtual Succeeder* clone() override;
		};

		/**
		*	@name Failer
		*	@brief Node always fails
		*/
		class Failer : public DecoratorNode
		{
		public:
			/**
			*	@name Failer
			*	@brief Failer constructor
			*
			*	@param child A child node to decorate.
			*/
			Failer(Node* child);

			//Disable copy constructor
			Failer(Failer const&) = delete;

			//Disable assign operator
			void operator=(Failer const&) = delete;

			// @copydoc Node::update(const float delta = 0)
			virtual const NODE_STATUS update(const float delta = 0) override;

			// @copydoc Node::clone()
			virtual Failer* clone() override;
		};

		/**
		*	@name Repeater
		*	@brief Node repeats for certain time if node successes or fails. Repeat stops if node Returns true after repeating.
		*/
		class Repeater : public DecoratorNode
		{
		private:
			//amount of number to repeat. 
			int repeat;
		public:
			/**
			*	@name Repeater
			*	@brief Repeater constructor
			*
			*	@param child A child node to decorate.
			*	@param repeat Amount of number to repeat this node.
			*/
			Repeater(Node* child, const int repeat);

			//Disable copy constructor
			Repeater(Repeater const&) = delete;

			//Disable assign operator
			void operator=(Repeater const&) = delete;

			// @copydoc Node::update(const float delta = 0)
			virtual const NODE_STATUS update(const float delta = 0) override;

			// @copydoc Node::clone()
			virtual Repeater* clone() override;
		};

		/**
		*	@name Locker
		*	@brief Locks on to this node after it gets exevuted until time expires.
		*/
		class Locker : public DecoratorNode
		{
		private:
			float duration;
			float elapsedTime;

			NODE_STATUS statusResult;
		public:
			/**
			*	@name Locker
			*	@brief Locker constructor
			*
			*	@param child A child node to decorate.
			*	@param duration A duration to lock this node
			*/
			Locker(Node* child, const float duration);

			//Disable copy constructor
			Locker(Locker const&) = delete;

			//Disable assign operator
			void operator=(Locker const&) = delete;


			// @copydoc Node::update(const float delta = 0)
			virtual const NODE_STATUS update(const float delta = 0) override;

			// @copydoc Node::clone()
			virtual Locker* clone() override;

			// @copydoc Node::reset()
			virtual void reset() override;
		};
	}
};

#endif