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

#include <sstream>

#include "ngraph/file_util.hpp"
#include "ngraph/runtime/backend.hpp"
#include "ngraph/runtime/backend_manager.hpp"
#include "ngraph/runtime/cpu/cpu_tensor_view.hpp"
#include "ngraph/util.hpp"

using namespace std;
using namespace ngraph;

runtime::Backend::~Backend()
{
}

unique_ptr<runtime::Backend> runtime::Backend::create(const string& type)
{
    return BackendManager::create_backend(type);
}

vector<string> runtime::Backend::get_registered_devices()
{
    return BackendManager::get_registered_backends();
}

void runtime::Backend::remove_compiled_function(shared_ptr<Function> func)
{
}

vector<ngraph::runtime::PerformanceCounter>
    runtime::Backend::get_performance_data(shared_ptr<Function> func) const
{
    return vector<PerformanceCounter>();
}

bool runtime::Backend::call_with_validate(
    std::shared_ptr<Function> func,
    const std::vector<std::shared_ptr<runtime::Tensor>>& outputs,
    const std::vector<std::shared_ptr<runtime::Tensor>>& inputs)
{
    vector<runtime::Tensor*> tout;
    vector<runtime::Tensor*> tin;
    for (auto t : outputs)
    {
        tout.push_back(t.get());
    }
    for (auto t : inputs)
    {
        tin.push_back(t.get());
    }
    validate(func, tout, tin);
    return call(func, outputs, inputs);
}

bool runtime::Backend::validate(Handle handle,
                                const vector<runtime::Tensor*>& outputs,
                                const vector<runtime::Tensor*>& inputs)
{
    const ParameterVector& parameters = get_parameters(handle);
    const ResultVector& results = get_results(handle);
    if (parameters.size() != inputs.size())
    {
        stringstream ss;
        ss << "Call input count " << inputs.size() << " does not match Function's Parameter count "
           << parameters.size();
        throw runtime_error(ss.str());
    }
    if (parameters.size() != outputs.size())
    {
        stringstream ss;
        ss << "Call output count " << outputs.size() << " does not match Function's Result count "
           << parameters.size();
        throw runtime_error(ss.str());
    }

    for (size_t i = 0; i < parameters.size(); i++)
    {
        if (parameters[i]->get_element_type() != inputs[i]->get_element_type())
        {
            stringstream ss;
            ss << "Input " << i << " type '" << inputs[i]->get_element_type()
               << "' does not match Parameter type '" << parameters[i]->get_element_type() << "'";
            throw runtime_error(ss.str());
        }
        if (parameters[i]->get_shape() != inputs[i]->get_shape())
        {
            stringstream ss;
            ss << "Input " << i << " shape {" << join(inputs[i]->get_shape())
               << "} does not match Parameter shape {" << join(parameters[i]->get_shape()) << "}";
            throw runtime_error(ss.str());
        }
    }

    for (size_t i = 0; i < results.size(); i++)
    {
        if (results[i]->get_element_type() != outputs[i]->get_element_type())
        {
            stringstream ss;
            ss << "Output " << i << " type '" << outputs[i]->get_element_type()
               << "' does not match Result type '" << results[i]->get_element_type() << "'";
            throw runtime_error(ss.str());
        }
        if (results[i]->get_shape() != outputs[i]->get_shape())
        {
            stringstream ss;
            ss << "Output " << i << " shape {" << join(outputs[i]->get_shape())
               << "} does not match Result shape {" << join(results[i]->get_shape()) << "}";
            throw runtime_error(ss.str());
        }
    }
    return true;
}

bool runtime::Backend::is_supported(const Node& node) const
{
    // The default behavior is that a backend does not support any ops. If this is not the case
    // then override this method and enhance.
    return false;
}

bool runtime::Backend::save(Handle handle, std::ostream& out) const
{
    return false;
}

runtime::Handle runtime::Backend::load(std::istream& in)
{
    return nullptr;
}

void runtime::Backend::enable_performance_data(std::shared_ptr<Function> func, bool enable)
{
}
