#ifndef AVL_TREE_HPP
#define AVL_TREE_HPP
#include <memory>

namespace avl
{
	class AvlTree
	{
	public:
		using ptr = std::unique_ptr<AvlTree>;
		class AvlTreeNode
		{
		public:
			using ptr = std::shared_ptr<AvlTreeNode>;
			using constPtr = std::shared_ptr<AvlTreeNode>;

			enum BalancingFactor
			{
				BALANCED = 0,
				LEFT_HEAVY = -1,
				RIGHT_HEAVY = 1
			};

			AvlTreeNode::ptr parent;
			AvlTreeNode::ptr left;
			AvlTreeNode::ptr right;

			long key;
			BalancingFactor balancingFactor;

			AvlTreeNode() = delete;
			explicit  AvlTreeNode(long literalFunctioningAsKey) 
				: parent(nullptr), left(nullptr), right(nullptr), key(literalFunctioningAsKey), balancingFactor(BALANCED), referenceCount(1) {}

			[[nodiscard]] std::size_t getReferenceCount() const
			{
				return referenceCount;
			}

			[[maybe_unused]] bool incrementReferenceCount()
			{
				if (referenceCount == SIZE_MAX) 
					return false;

				++referenceCount;
				return true;
			}

			[[maybe_unused]] bool decrementReferenceCount()
			{
				if (!referenceCount)
					return false;

				--referenceCount;
				return true;
			}


			friend BalancingFactor& operator++(BalancingFactor& factor)
			{
				switch (factor)
				{
				case BALANCED:
					factor = RIGHT_HEAVY;
					break;
				case LEFT_HEAVY:
					factor = BALANCED;
					break;
				default:
					break;
				}
				return factor;
			}

			friend BalancingFactor& operator--(BalancingFactor& factor)
			{
				switch (factor)
				{
				case BALANCED:
					factor = LEFT_HEAVY;
					break;
				case RIGHT_HEAVY:
					factor = BALANCED;
					break;
				default:
					break;
				}
				return factor;
			}
		protected:
			std::size_t referenceCount;
		};

		[[nodiscard]] bool find(long literal) const;
		[[maybe_unused]] bool insert(long literal);
		[[maybe_unused]] bool remove(long literal);
	protected:
		AvlTreeNode::ptr root;

		/*
		 *			P
		 *		X		Y
		 *			T1		T2
		 *
		 *
		 *			Y
		 *		P		T2
		 *	X		T1
		 */
		[[maybe_unused]] static AvlTreeNode::ptr rotateLeft(const AvlTreeNode::ptr& parentNode, const AvlTreeNode::ptr& rightChild);


		/*
		 *			P
		 *		X		Y
		 *	T1		T2
		 *			
		 *
		 *			X
		 *		T1		P
		 *			T2		Y		
		 */
		[[maybe_unused]] static AvlTreeNode::ptr rotateRight(const AvlTreeNode::ptr& parentNode, const AvlTreeNode::ptr& leftChild);


		/*
		 *			P
		 *		X		Y
		 *	t1		Z		t4
		 *		t2		t3
		 *
		 *			P
		 *		X		Z
		 *	t1		t2		Y
		 *				t3		t4
		 *
		 *				Z
		 *			P			Y
		 *		X		t2	t3		t4
		 *	t1
		 */
		[[maybe_unused]] static AvlTreeNode::ptr rotateRightLeft(const AvlTreeNode::ptr& parentNode, const AvlTreeNode::ptr& rightChild);

		/*
		 *			P
		 *		X		Y
		 *	t1		Z		t4
		 *		t2		t3
		 *
		 *				P
		 *			Z		Y
		 *		X		t3		t4
		 *	t1		t2
		 *
		 *
		 *				Z
		 *		X			P
		 *	t1		t2	t3		Y
		 *							t4
		 *
		 */		
		[[maybe_unused]] static AvlTreeNode::ptr rotateLeftRight(const AvlTreeNode::ptr& parentNode, const AvlTreeNode::ptr& leftChild);
		static void replaceNodeInAvlTreeStructure(const AvlTreeNode::ptr& nodeToReplace, const AvlTreeNode::ptr& replacementNode);

		static void assertNodeInvariant(const AvlTreeNode& treeNode);
		static bool doesNodeInvariantHold(const AvlTreeNode& treeNode);
	};
}

#endif