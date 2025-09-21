/*
 * Copyright (C) 2025 Patrick Mours
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include "runtime.hpp"
#include <mutex>
#include <vector>

namespace reshade
{
	struct __declspec(uuid("8A9549DA-6243-49E5-9A04-2F4C0D1F9C62")) preprocess_runtime_device_data
	{
		std::mutex mutex;
		std::vector<runtime *> runtimes;
	};

	preprocess_runtime_device_data *get_preprocess_runtime_data(api::device *device);
	void register_preprocess_runtime(api::device *device, runtime *runtime_instance);
	void unregister_preprocess_runtime(api::device *device, runtime *runtime_instance);
}
