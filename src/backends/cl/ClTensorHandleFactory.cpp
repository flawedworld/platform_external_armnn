//
// Copyright © 2017 Arm Ltd. All rights reserved.
// SPDX-License-Identifier: MIT
//


#include "ClTensorHandleFactory.hpp"
#include "ClTensorHandle.hpp"

#include <arm_compute/runtime/CL/CLTensor.h>
#include <arm_compute/core/Coordinates.h>
#include <arm_compute/runtime/CL/CLSubTensor.h>

#include <boost/polymorphic_cast.hpp>


namespace armnn
{

using FactoryId = ITensorHandleFactory::FactoryId;

std::unique_ptr<ITensorHandle> ClTensorHandleFactory::CreateSubTensorHandle(ITensorHandle& parent,
                                                                            const TensorShape& subTensorShape,
                                                                            const unsigned int* subTensorOrigin) const
{
    arm_compute::Coordinates coords;
    arm_compute::TensorShape shape = armcomputetensorutils::BuildArmComputeTensorShape(subTensorShape);

    coords.set_num_dimensions(subTensorShape.GetNumDimensions());
    for (unsigned int i = 0; i < subTensorShape.GetNumDimensions(); ++i)
    {
        // Arm compute indexes tensor coords in reverse order.
        unsigned int revertedIndex = subTensorShape.GetNumDimensions() - i - 1;
        coords.set(i, boost::numeric_cast<int>(subTensorOrigin[revertedIndex]));
    }

    const arm_compute::TensorShape parentShape = armcomputetensorutils::BuildArmComputeTensorShape(
            parent.GetShape());
    if (!::arm_compute::error_on_invalid_subtensor(__func__, __FILE__, __LINE__, parentShape, coords, shape))
    {
        return nullptr;
    }

    return std::make_unique<ClSubTensorHandle>(
            boost::polymorphic_downcast<IClTensorHandle *>(&parent), shape, coords);
}

std::unique_ptr<ITensorHandle> ClTensorHandleFactory::CreateTensorHandle(const TensorInfo& tensorInfo) const
{
    return ClTensorHandleFactory::CreateTensorHandle(tensorInfo, true);
}

std::unique_ptr<ITensorHandle> ClTensorHandleFactory::CreateTensorHandle(const TensorInfo& tensorInfo,
                                                                         DataLayout dataLayout) const
{
    return ClTensorHandleFactory::CreateTensorHandle(tensorInfo, dataLayout, true);
}

std::unique_ptr<ITensorHandle> ClTensorHandleFactory::CreateTensorHandle(const TensorInfo& tensorInfo,
                                                                         const bool IsMemoryManaged) const
{
    std::unique_ptr<ClTensorHandle> tensorHandle = std::make_unique<ClTensorHandle>(tensorInfo);
    if (IsMemoryManaged)
    {
        tensorHandle->SetMemoryGroup(m_MemoryManager->GetInterLayerMemoryGroup());
    }
    return tensorHandle;
}

std::unique_ptr<ITensorHandle> ClTensorHandleFactory::CreateTensorHandle(const TensorInfo& tensorInfo,
                                                                         DataLayout dataLayout,
                                                                         const bool IsMemoryManaged) const
{
    std::unique_ptr<ClTensorHandle> tensorHandle = std::make_unique<ClTensorHandle>(tensorInfo, dataLayout);
    if (IsMemoryManaged)
    {
        tensorHandle->SetMemoryGroup(m_MemoryManager->GetInterLayerMemoryGroup());
    }
    return tensorHandle;
}

const FactoryId& ClTensorHandleFactory::GetIdStatic()
{
    static const FactoryId s_Id(ClTensorHandleFactoryId());
    return s_Id;
}

const FactoryId& ClTensorHandleFactory::GetId() const
{
    return GetIdStatic();
}

bool ClTensorHandleFactory::SupportsSubTensors() const
{
    return true;
}

MemorySourceFlags ClTensorHandleFactory::GetExportFlags() const
{
    return m_ExportFlags;
}

MemorySourceFlags ClTensorHandleFactory::GetImportFlags() const
{
    return m_ImportFlags;
}

} // namespace armnn