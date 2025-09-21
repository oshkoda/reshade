/*
 * Copyright (C) 2025 Patrick Mours
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "runtime_dlss_preprocess.hpp"
#include "reshade_api_device.hpp"
#include <algorithm>

namespace reshade
{
	preprocess_runtime_device_data *get_preprocess_runtime_data(api::device *device)
	{
		if (device == nullptr)
			return nullptr;

		return device->get_private_data<preprocess_runtime_device_data>();
	}

	static preprocess_runtime_device_data *ensure_preprocess_runtime_data(api::device *device)
	{
		auto data = get_preprocess_runtime_data(device);
		if (data == nullptr)
			data = device->create_private_data<preprocess_runtime_device_data>();
		return data;
	}

	void register_preprocess_runtime(api::device *device, runtime *runtime_instance)
	{
		if (device == nullptr || runtime_instance == nullptr)
			return;

		auto data = ensure_preprocess_runtime_data(device);
		std::lock_guard<std::mutex> lock(data->mutex);
		if (std::find(data->runtimes.begin(), data->runtimes.end(), runtime_instance) == data->runtimes.end())
			data->runtimes.push_back(runtime_instance);
	}

	void unregister_preprocess_runtime(api::device *device, runtime *runtime_instance)
	{
		if (device == nullptr)
			return;

		if (auto data = get_preprocess_runtime_data(device))
		{
			std::unique_lock<std::mutex> lock(data->mutex);
			if (runtime_instance != nullptr)
				data->runtimes.erase(std::remove(data->runtimes.begin(), data->runtimes.end(), runtime_instance), data->runtimes.end());

			const bool empty = data->runtimes.empty();
			lock.unlock();

			if (empty)
				device->destroy_private_data<preprocess_runtime_device_data>();
		}
	}
}
