#include "Node.h"

void Node::Update(CommandList10& Cmdl, float DeltaTime) {
    // Initializing with local transform
    Matrix4 worldTransform(mLocalTransform);

    if (mParent) {
        worldTransform = mParent->GetTransform() * worldTransform;
    }

    if (mMeshInstance) {
        mMeshInstance->Update(Cmdl, worldTransform);
    }
}
