#include "CommandList10.h"

void CommandList10::TransitionResource(ID3D12Resource2* Resource,
                                       D3D12_RESOURCE_STATES Before,
                                       D3D12_RESOURCE_STATES After) const {
    // Prepare a resource barrier for transition.
    D3D12_RESOURCE_BARRIER barrier;

    // Specifies this is a transition barrier.
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

    // The resource to transition.
    barrier.Transition.pResource = Resource;

    // Transition only the first subresource (subresource index 0).
    barrier.Transition.Subresource = 0;

    barrier.Transition.StateBefore = Before;
    barrier.Transition.StateAfter = After;

    mCommandList->ResourceBarrier(1, &barrier);
}