//*****************************************************************************
// Copyright 2017-2018 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//*****************************************************************************

#pragma once

#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "ngraph/runtime/backend.hpp"
#include "ngraph/runtime/host_tensor.hpp"
#include "ngraph/runtime/tensor.hpp"

namespace ngraph
{
    namespace runtime
    {
        namespace nop
        {
            class NOPBackend;
        }
    }
}

class ngraph::runtime::nop::NOPBackend : public Backend
{
public:
    std::shared_ptr<Tensor>
        create_tensor(const element::Type& type, const Shape& shape, void* memory_pointer) override;

    std::shared_ptr<Tensor> create_tensor(const element::Type& type, const Shape& shape) override;

    Handle compile(std::shared_ptr<Function> function) override;
    Handle compile(const Function& function, bool enable_performance_collection) override;

    bool call(std::shared_ptr<Function> function,
              const std::vector<std::shared_ptr<Tensor>>& outputs,
              const std::vector<std::shared_ptr<Tensor>>& intputs) override;
    bool execute(Handle handle,
                 const std::vector<Tensor*>& outputs,
                 const std::vector<Tensor*>& intputs) override;
    const ParameterVector& get_parameter_descriptors(Handle handle) const override;
    const ResultVector& get_result_descriptors(Handle handle) const override;
};
