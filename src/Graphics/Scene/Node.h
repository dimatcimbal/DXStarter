#pragma once

#include <algorithm>
#include <functional>
#include <memory>
#include <ranges>
#include <stack>
#include <utility>
#include <vector>

#include "Graphics/Material/Material.h"
#include "Graphics/Mesh/MeshInstance.h"
#include "Math/Matrix.h"

class Node {
   public:
    using NodeVisitor = std::function<void(Node*)>;

    /**
     * Traverses a node tree in depth-first order, invoking the Visitor function for each node.
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
     * @param visitor A callable function that will be invoked for each node during traversal.
     *                Must accept a const Node* parameter. If empty/null, the function returns
     * immediately.
     */
    static void TraverseDepthFirst(Node* Root, NodeVisitor visitor) {
        if (!Root || !visitor) {
            return;
        }

        std::stack<Node*> visitingNodes;

        // Add root Node as starting point
        visitingNodes.push(Root);

        // Walk down the tree depth-first
        while (!visitingNodes.empty()) {
            // Get the next node from the top of the stack
            auto currentNode = visitingNodes.top();
            visitingNodes.pop();

            // Visit the current node
            visitor(currentNode);

            // Add all children to the stack in reverse order so that the first child gets placed at
            // the top of the stack.
            for (auto& it : std::ranges::reverse_view(currentNode->mChildren)) {
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
        : mMeshInstance(std::move(Mesh)), mMaterialId(MaterialId), mLocalTransform(Matrix4{}) {}

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
          mLocalTransform(std::exchange(other.mLocalTransform, Matrix4{})) {
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
            mParent = nullptr;

            UpdateChildrenParent();
            other.mParent = nullptr;
        }
        return *this;
    }

    void Update(CommandList10& Cmdl, float DeltaTime);

    MeshInstance* GetMeshInstance() const {
        return mMeshInstance.get();
    }

    MaterialId GetMaterialId() const {
        return mMaterialId;
    }

    Matrix4 GetTransform() const {
        return mLocalTransform;
    }

    Matrix4& GetTransform() {
        return mLocalTransform;
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
    std::unique_ptr<MeshInstance> mMeshInstance;

    // Intentionally uses MaterialId instead of a Material reference to decouple Node from Material
    // and enable efficient batching by grouping nodes with the same MaterialId to minimize PSO
    // switches.
    MaterialId mMaterialId{0};
};
