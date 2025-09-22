/*
 * Copyright (C) 2025 Patrick Mours
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "d3d12/d3d12_device.hpp"
#include "d3d12/d3d12_command_list.hpp"
#include "d3d12/d3d12_impl_type_convert.hpp"
#include "runtime_dlss_preprocess.hpp"
#include "hook_manager.hpp"

#include <mutex>

struct ID3D11Resource;
struct ID3D12Resource;
struct NVSDK_NGX_Handle;

#ifndef NVSDK_CONV
#define NVSDK_CONV __cdecl
#endif

enum NVSDK_NGX_Result
{
	NVSDK_NGX_Result_Success = 0x1
};

struct NVSDK_NGX_Parameter
{
	virtual void Set(const char *, unsigned long long) = 0;
	virtual void Set(const char *, float) = 0;
	virtual void Set(const char *, double) = 0;
	virtual void Set(const char *, unsigned int) = 0;
	virtual void Set(const char *, int) = 0;
	virtual void Set(const char *, ID3D11Resource *) = 0;
	virtual void Set(const char *, ID3D12Resource *) = 0;
	virtual void Set(const char *, void *) = 0;

	virtual NVSDK_NGX_Result Get(const char *, unsigned long long *) const = 0;
	virtual NVSDK_NGX_Result Get(const char *, float *) const = 0;
	virtual NVSDK_NGX_Result Get(const char *, double *) const = 0;
	virtual NVSDK_NGX_Result Get(const char *, unsigned int *) const = 0;
	virtual NVSDK_NGX_Result Get(const char *, int *) const = 0;
	virtual NVSDK_NGX_Result Get(const char *, ID3D11Resource **) const = 0;
	virtual NVSDK_NGX_Result Get(const char *, ID3D12Resource **) const = 0;
	virtual NVSDK_NGX_Result Get(const char *, void **) const = 0;

	virtual void Reset() = 0;
};

typedef void (NVSDK_CONV *PFN_NVSDK_NGX_ProgressCallback)(float, bool &);

extern "C" NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_D3D12_EvaluateFeature(
	ID3D12GraphicsCommandList *InCmdList,
	const NVSDK_NGX_Handle *InFeatureHandle,
	NVSDK_NGX_Parameter *InParameters,
	PFN_NVSDK_NGX_ProgressCallback InCallback)
{
	static const auto trampoline = reshade::hooks::call(NVSDK_NGX_D3D12_EvaluateFeature);

	if (InCmdList == nullptr || InParameters == nullptr)
		return trampoline(InCmdList, InFeatureHandle, InParameters, InCallback);

	auto command_list_proxy = static_cast<D3D12GraphicsCommandList *>(InCmdList);
	auto cmd_list_impl = static_cast<reshade::d3d12::command_list_impl *>(command_list_proxy);
	auto device_impl = static_cast<reshade::d3d12::device_impl *>(cmd_list_impl->get_device());

	reshade::runtime *runtime_instance = nullptr;
	if (auto data = reshade::get_preprocess_runtime_data(device_impl))
	{
		std::unique_lock<std::mutex> lock(data->mutex);
		for (reshade::runtime *candidate : data->runtimes)
		{
			if (candidate != nullptr && candidate->get_device() == device_impl)
			{
				runtime_instance = candidate;
				break;
			}
		}
	}

	if (runtime_instance != nullptr && runtime_instance->is_preprocess_dlss_input_enabled())
	{
		ID3D12Resource *color_native = nullptr;
		if (InParameters->Get("Color", &color_native) == NVSDK_NGX_Result_Success && color_native != nullptr)
		{
			const reshade::api::resource color_resource = reshade::d3d12::to_handle(color_native);
			const reshade::api::resource_desc color_desc = device_impl->get_resource_desc(color_resource);

			reshade::api::format render_format = color_desc.texture.format;
			if (render_format != reshade::api::format::unknown &&
				reshade::api::format_to_typeless(render_format) == render_format)
			{
				const reshade::api::format typed_format = reshade::api::format_to_default_typed(render_format, 0);
				if (typed_format == reshade::api::format::unknown)
					render_format = reshade::api::format::unknown;
				else
					render_format = typed_format;
			}

			if (render_format != reshade::api::format::unknown)
			{
				reshade::api::resource_view rtv = {};
				reshade::api::resource_view rtv_srgb = {};

				const reshade::api::resource_view_desc view_desc(render_format);
				if (device_impl->create_resource_view(color_resource, reshade::api::resource_usage::render_target, view_desc, &rtv))
				{
					reshade::api::format srgb_format = reshade::api::format_to_default_typed(render_format, 1);
					if (srgb_format == reshade::api::format::unknown)
						srgb_format = render_format;

					if (srgb_format != render_format)
					{
						const reshade::api::resource_view_desc srgb_desc(srgb_format);
						if (!device_impl->create_resource_view(color_resource, reshade::api::resource_usage::render_target, srgb_desc, &rtv_srgb))
							srgb_format = render_format;
					}

					if (rtv_srgb.handle == 0)
						rtv_srgb = rtv;

					const reshade::api::resource resources[] = { color_resource };
					const reshade::api::resource_usage state_before[] = { reshade::api::resource_usage::shader_resource };
					const reshade::api::resource_usage state_target[] = { reshade::api::resource_usage::render_target };

                                        D3D12GraphicsCommandList::state_snapshot previous_state;
                                        command_list_proxy->capture_state(previous_state);

                                        cmd_list_impl->barrier(1, resources, state_before, state_target);
                                        runtime_instance->render_effects(cmd_list_impl, rtv, rtv_srgb);
                                        command_list_proxy->apply_state(previous_state);
                                        cmd_list_impl->barrier(1, resources, state_target, state_before);

					if (rtv_srgb != rtv)
						device_impl->destroy_resource_view(rtv_srgb);
					device_impl->destroy_resource_view(rtv);
				}
			}
		}
	}

	return trampoline(InCmdList, InFeatureHandle, InParameters, InCallback);
}
