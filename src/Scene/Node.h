#pragma once

#include <memory>
#include <queue>
#include <ranges>
#include <stack>
#include <utility>
#include <vector>

#include "Graphics/Material/Material.h"
#include "Graphics/Mesh/MeshInstance.h"
#include "Math/Matrix.h"

class Node;

class NodeVisitor {
   public:
    virtual ~NodeVisitor() = default;
    virtual void Visit(Node* node) = 0;
};

class Node {
   public:
    /**
     * Traverses a node tree in depth-first order, invoking the visitor's Visit method for each
     * node.
     *
     * Uses an iterative stack-based approach to traverse the tree. Nodes are visited in
     * depth-first order (parent before children, first child before siblings).
     *
     * Example tree traversal order:
     *
     *           A
     *          /|\
     *         B C D
     *        /|   |
     *       E F   G
     *
     * Visit Order: A → B → E → F → C → D → G
     *
     * @param Root The root node of the tree to traverse. If null, the function returns immediately.
     * @param visitors A vararg of visitor objects that implements NodeVisitor. The visitor's Visit
     * method will be called for each node during traversal.
     */
    template <typename... Visitors>
    static void TraverseDepthFirst(Node* Root, Visitors&... visitors) {
        if (!Root) {
            return;
        }

        // Create a Stack and add root node as starting point
        std::stack<Node*> visitingNodes;
        visitingNodes.push(Root);

        // Walk down the tree depth-first
        while (!visitingNodes.empty()) {
            // Get the next node from the top of the stack
            auto& node = visitingNodes.top();
            visitingNodes.pop();

            // Call all visitors using C++17 fold expression
            (visitors.Visit(node), ...);

            // Add all children to the stack in reverse order so that the first child gets placed at
            // the top of the stack.
            for (auto& it : std::ranges::reverse_view(node->mChildren)) {
                visitingNodes.push(it.get());
            }
        }
    }

    /**
     * Traverses a node tree in breadth-first order, invoking the visitor's Visit method for each
     * node.
     *
     * Uses an iterative queue-based approach to traverse the tree. Nodes are visited in
     * breadth-first order (all nodes at the same depth before moving to the next level).
     *
     * Example tree traversal order:
     *
     *           A
     *          /|\
     *         B C D
     *        /|   |
     *       E F   G
     *
     * Visit Order: A → B → C → D → E → F → G
     *
     * @param Root The root node of the tree to traverse. If null, the function returns immediately.
     * @param visitor A visitor object that implements NodeVisitor. The visitor's Visit method
     *                will be called for each node during traversal.
     */
    static void TraverseBreadthFirst(Node* Root, NodeVisitor& visitor) {
        if (!Root) {
            return;
        }

        // Create a Queue and add root node as starting point
        std::queue<Node*> visitingNodes;
        visitingNodes.push(Root);

        while (!visitingNodes.empty()) {
            // Get the next node from the front of the queue
            auto& node = visitingNodes.front();
            visitingNodes.pop();

            visitor.Visit(node);

            // Add all children to the queue in the natural order so that the first child gets
            // placed at the front of the queue
            for (auto& it : std::ranges::reverse_view(node->mChildren)) {
                visitingNodes.push(it.get());
            }
        }
    }

    static bool Create(MaterialId MaterialId,
                       std::unique_ptr<MeshInstance>&& MeshInstance,
                       std::unique_ptr<Node>& OutNode) {
        OutNode = std::make_unique<Node>(MaterialId, std::move(MeshInstance));
        return true;
    }

    Node(MaterialId MaterialId, std::unique_ptr<MeshInstance>&& Mesh)
        : mMeshInstance(std::move(Mesh)),
          mMaterialId(MaterialId),
          mLocalTransform(Matrix4{}),
          mWorldTransform(Matrix4{}) {}

    Node() = default;

    ~Node() = default;

    // Prohibit copying
    Node(const Node&) = delete;
    Node& operator=(const Node&) = delete;

    // Move constructor
    Node(Node&& other) noexcept
        : mMeshInstance(std::exchange(other.mMeshInstance, nullptr)),
          mMaterialId(std::exchange(other.mMaterialId, MaterialId{0})),
          mChildren(std::exchange(other.mChildren, {})),
          mLocalTransform(std::exchange(other.mLocalTransform, Matrix4{})),
          mWorldTransform(std::exchange(other.mWorldTransform, Matrix4{})) {
        UpdateChildrenParent();
        other.mParent = nullptr;
    }

    // Move assignment operator
    Node& operator=(Node&& other) noexcept {
        if (this != &other) {
            mMeshInstance = std::exchange(other.mMeshInstance, nullptr);
            mMaterialId = std::exchange(other.mMaterialId, MaterialId{0});
            mChildren = std::exchange(other.mChildren, {});
            mLocalTransform = std::exchange(other.mLocalTransform, Matrix4{});
            mWorldTransform = std::exchange(other.mWorldTransform, Matrix4{});
            mParent = nullptr;

            UpdateChildrenParent();
            other.mParent = nullptr;
        }
        return *this;
    }

    MeshInstance* GetMeshInstance() const {
        return mMeshInstance.get();
    }

    MaterialId GetMaterialId() const {
        return mMaterialId;
    }

    Matrix4& GetTransform() {
        return mLocalTransform;
    }

    const Matrix4& GetWorldTransform() const {
        return mWorldTransform;
    }

    Matrix4& GetWorldTransform() {
        return mWorldTransform;
    }

    void SetWorldTransform(const Matrix4& Transform) {
        mWorldTransform = Transform;
    }

    bool HasParent() const {
        return mParent != nullptr;
    }

    Node* GetParent() const {
        return mParent;
    }

    void AddChild(std::unique_ptr<Node>&& Child) {
        Child->mParent = this;
        mChildren.push_back(std::move(Child));
    }

   private:
    void UpdateChildrenParent() {
        for (auto& child : mChildren) {
            child->mParent = this;
        }
    }

    // Scene tree
    Node* mParent{nullptr};
    std::vector<std::unique_ptr<Node>> mChildren;

    // Owned components
    Matrix4 mLocalTransform;
    Matrix4 mWorldTransform;
    std::unique_ptr<MeshInstance> mMeshInstance;

    // Intentionally uses MaterialId instead of a Material reference to decouple Node from Material
    // and enable efficient batching by grouping nodes with the same MaterialId to minimize PSO
    // switches.
    MaterialId mMaterialId{0};
};
