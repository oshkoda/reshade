/*
 * Copyright (C) 2014 Patrick Mours
 * SPDX-License-Identifier: BSD-3-Clause OR MIT
 */

#pragma once

#include "d3d12_impl_command_list.hpp"

#include <vector>

struct D3D12Device;

struct DECLSPEC_UUID("479B29E3-9A2C-11D0-B696-00A0C903487A") D3D12GraphicsCommandList final : ID3D12GraphicsCommandList10, public reshade::d3d12::command_list_impl
{
        struct state_snapshot
        {
                state_snapshot() = default;
                state_snapshot(const state_snapshot &) = delete;
                state_snapshot &operator=(const state_snapshot &) = delete;
                ~state_snapshot()
                {
                        release();
                }

                void release()
                {
                        if (pipeline_state != nullptr)
                        {
                                pipeline_state->Release();
                                pipeline_state = nullptr;
                        }
                        if (raytracing_pipeline_state != nullptr)
                        {
                                raytracing_pipeline_state->Release();
                                raytracing_pipeline_state = nullptr;
                        }
                        for (ID3D12RootSignature *&signature : root_signatures)
                        {
                                if (signature != nullptr)
                                {
                                        signature->Release();
                                        signature = nullptr;
                                }
                        }
                        for (ID3D12DescriptorHeap *&heap : descriptor_heaps)
                        {
                                if (heap != nullptr)
                                {
                                        heap->Release();
                                        heap = nullptr;
                                }
                        }

                        root_states[0].clear();
                        root_states[1].clear();
                }

                struct root_parameter_state
                {
                        void clear()
                        {
                                descriptor_tables.clear();
                                constant_buffer_views.clear();
                                shader_resource_views.clear();
                                unordered_access_views.clear();
                                constants.clear();
                                constant_masks.clear();
                        }

                        std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> descriptor_tables;
                        std::vector<D3D12_GPU_VIRTUAL_ADDRESS> constant_buffer_views;
                        std::vector<D3D12_GPU_VIRTUAL_ADDRESS> shader_resource_views;
                        std::vector<D3D12_GPU_VIRTUAL_ADDRESS> unordered_access_views;
                        std::vector<std::vector<uint32_t>> constants;
                        std::vector<std::vector<uint8_t>> constant_masks;
                } root_states[2];

                ID3D12PipelineState *pipeline_state = nullptr;
                ID3D12StateObject *raytracing_pipeline_state = nullptr;
                ID3D12RootSignature *root_signatures[2] = {};
                ID3D12DescriptorHeap *descriptor_heaps[2] = {};
                UINT num_render_targets = 0;
                D3D12_CPU_DESCRIPTOR_HANDLE render_targets[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = {};
                BOOL render_targets_single_handle_range = FALSE;
                D3D12_CPU_DESCRIPTOR_HANDLE depth_stencil = {};
                UINT num_viewports = 0;
                D3D12_VIEWPORT viewports[D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE] = {};
                UINT num_scissor_rects = 0;
                D3D12_RECT scissor_rects[D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE] = {};
                D3D12_PRIMITIVE_TOPOLOGY primitive_topology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
                float blend_factor[4] = {};
                bool blend_factor_valid = false;
                bool depth_bias_valid = false;
                float depth_bias[3] = {};
                bool stencil_ref_valid = false;
                UINT stencil_ref = 0;
                bool front_back_stencil_valid = false;
                UINT front_stencil_ref = 0;
                UINT back_stencil_ref = 0;
        };

        D3D12GraphicsCommandList(D3D12Device *device, ID3D12GraphicsCommandList *original);
        ~D3D12GraphicsCommandList();

        void capture_state(state_snapshot &state) const;
        void apply_state(const state_snapshot &state);

	#pragma region IUnknown
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObj) override;
	ULONG   STDMETHODCALLTYPE AddRef() override;
	ULONG   STDMETHODCALLTYPE Release() override;
	#pragma endregion
	#pragma region ID3D12Object
	HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID guid, UINT *pDataSize, void *pData) override;
	HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID guid, UINT DataSize, const void *pData) override;
	HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID guid, const IUnknown *pData) override;
	HRESULT STDMETHODCALLTYPE SetName(LPCWSTR Name) override;
	#pragma endregion
	#pragma region ID3D12DeviceChild
	HRESULT STDMETHODCALLTYPE GetDevice(REFIID riid, void **ppvDevice) override;
	#pragma endregion
	#pragma region ID3D12CommandList
	D3D12_COMMAND_LIST_TYPE STDMETHODCALLTYPE GetType() override;
	#pragma endregion
	#pragma region ID3D12GraphicsCommandList
	HRESULT STDMETHODCALLTYPE Close() override;
	HRESULT STDMETHODCALLTYPE Reset(ID3D12CommandAllocator *pAllocator, ID3D12PipelineState *pInitialState) override;
	void    STDMETHODCALLTYPE ClearState(ID3D12PipelineState *pPipelineState) override;
	void    STDMETHODCALLTYPE DrawInstanced(UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation) override;
	void    STDMETHODCALLTYPE DrawIndexedInstanced(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation) override;
	void    STDMETHODCALLTYPE Dispatch(UINT ThreadGroupCountX, UINT ThreadGroupCountY, UINT ThreadGroupCountZ) override;
	void    STDMETHODCALLTYPE CopyBufferRegion(ID3D12Resource *pDstBuffer, UINT64 DstOffset, ID3D12Resource *pSrcBuffer, UINT64 SrcOffset, UINT64 NumBytes) override;
	void    STDMETHODCALLTYPE CopyTextureRegion(const D3D12_TEXTURE_COPY_LOCATION *pDst, UINT DstX, UINT DstY, UINT DstZ, const D3D12_TEXTURE_COPY_LOCATION *pSrc, const D3D12_BOX *pSrcBox) override;
	void    STDMETHODCALLTYPE CopyResource(ID3D12Resource *pDstResource, ID3D12Resource *pSrcResource) override;
	void    STDMETHODCALLTYPE CopyTiles(ID3D12Resource *pTiledResource, const D3D12_TILED_RESOURCE_COORDINATE *pTileRegionStartCoordinate, const D3D12_TILE_REGION_SIZE *pTileRegionSize, ID3D12Resource *pBuffer, UINT64 BufferStartOffsetInBytes, D3D12_TILE_COPY_FLAGS Flags) override;
	void    STDMETHODCALLTYPE ResolveSubresource(ID3D12Resource *pDstResource, UINT DstSubresource, ID3D12Resource *pSrcResource, UINT SrcSubresource, DXGI_FORMAT Format) override;
	void    STDMETHODCALLTYPE IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY PrimitiveTopology) override;
	void    STDMETHODCALLTYPE RSSetViewports(UINT NumViewports, const D3D12_VIEWPORT *pViewports) override;
	void    STDMETHODCALLTYPE RSSetScissorRects(UINT NumRects, const D3D12_RECT *pRects) override;
	void    STDMETHODCALLTYPE OMSetBlendFactor(const FLOAT BlendFactor[4]) override;
	void    STDMETHODCALLTYPE OMSetStencilRef(UINT StencilRef) override;
	void    STDMETHODCALLTYPE SetPipelineState(ID3D12PipelineState *pPipelineState) override;
	void    STDMETHODCALLTYPE ResourceBarrier(UINT NumBarriers, const D3D12_RESOURCE_BARRIER *pBarriers) override;
	void    STDMETHODCALLTYPE ExecuteBundle(ID3D12GraphicsCommandList *pCommandList) override;
	void    STDMETHODCALLTYPE SetDescriptorHeaps(UINT NumDescriptorHeaps, ID3D12DescriptorHeap *const *ppDescriptorHeaps) override;
	void    STDMETHODCALLTYPE SetComputeRootSignature(ID3D12RootSignature *pRootSignature) override;
	void    STDMETHODCALLTYPE SetGraphicsRootSignature(ID3D12RootSignature *pRootSignature) override;
	void    STDMETHODCALLTYPE SetComputeRootDescriptorTable(UINT RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor) override;
	void    STDMETHODCALLTYPE SetGraphicsRootDescriptorTable(UINT RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor) override;
	void    STDMETHODCALLTYPE SetComputeRoot32BitConstant(UINT RootParameterIndex, UINT SrcData, UINT DestOffsetIn32BitValues) override;
	void    STDMETHODCALLTYPE SetGraphicsRoot32BitConstant(UINT RootParameterIndex, UINT SrcData, UINT DestOffsetIn32BitValues) override;
	void    STDMETHODCALLTYPE SetComputeRoot32BitConstants(UINT RootParameterIndex, UINT Num32BitValuesToSet, const void *pSrcData, UINT DestOffsetIn32BitValues) override;
	void    STDMETHODCALLTYPE SetGraphicsRoot32BitConstants(UINT RootParameterIndex, UINT Num32BitValuesToSet, const void *pSrcData, UINT DestOffsetIn32BitValues) override;
	void    STDMETHODCALLTYPE SetComputeRootConstantBufferView(UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) override;
	void    STDMETHODCALLTYPE SetGraphicsRootConstantBufferView(UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) override;
	void    STDMETHODCALLTYPE SetComputeRootShaderResourceView(UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) override;
	void    STDMETHODCALLTYPE SetGraphicsRootShaderResourceView(UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) override;
	void    STDMETHODCALLTYPE SetComputeRootUnorderedAccessView(UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) override;
	void    STDMETHODCALLTYPE SetGraphicsRootUnorderedAccessView(UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) override;
	void    STDMETHODCALLTYPE IASetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW *pView) override;
	void    STDMETHODCALLTYPE IASetVertexBuffers(UINT StartSlot, UINT NumViews, const D3D12_VERTEX_BUFFER_VIEW *pViews) override;
	void    STDMETHODCALLTYPE SOSetTargets(UINT StartSlot, UINT NumViews, const D3D12_STREAM_OUTPUT_BUFFER_VIEW *pViews) override;
	void    STDMETHODCALLTYPE OMSetRenderTargets(UINT NumRenderTargetDescriptors, const D3D12_CPU_DESCRIPTOR_HANDLE *pRenderTargetDescriptors, BOOL RTsSingleHandleToDescriptorRange, const D3D12_CPU_DESCRIPTOR_HANDLE *pDepthStencilDescriptor) override;
	void    STDMETHODCALLTYPE ClearDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView, D3D12_CLEAR_FLAGS ClearFlags, FLOAT Depth, UINT8 Stencil, UINT NumRects, const D3D12_RECT *pRects) override;
	void    STDMETHODCALLTYPE ClearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView, const FLOAT ColorRGBA[4], UINT NumRects, const D3D12_RECT *pRects) override;
	void    STDMETHODCALLTYPE ClearUnorderedAccessViewUint(D3D12_GPU_DESCRIPTOR_HANDLE ViewGPUHandleInCurrentHeap, D3D12_CPU_DESCRIPTOR_HANDLE ViewCPUHandle, ID3D12Resource *pResource, const UINT Values[4], UINT NumRects, const D3D12_RECT *pRects) override;
	void    STDMETHODCALLTYPE ClearUnorderedAccessViewFloat(D3D12_GPU_DESCRIPTOR_HANDLE ViewGPUHandleInCurrentHeap, D3D12_CPU_DESCRIPTOR_HANDLE ViewCPUHandle, ID3D12Resource *pResource, const FLOAT Values[4], UINT NumRects, const D3D12_RECT *pRects) override;
	void    STDMETHODCALLTYPE DiscardResource(ID3D12Resource *pResource, const D3D12_DISCARD_REGION *pRegion) override;
	void    STDMETHODCALLTYPE BeginQuery(ID3D12QueryHeap *pQueryHeap, D3D12_QUERY_TYPE Type, UINT Index) override;
	void    STDMETHODCALLTYPE EndQuery(ID3D12QueryHeap *pQueryHeap, D3D12_QUERY_TYPE Type, UINT Index) override;
	void    STDMETHODCALLTYPE ResolveQueryData(ID3D12QueryHeap *pQueryHeap, D3D12_QUERY_TYPE Type, UINT StartIndex, UINT NumQueries, ID3D12Resource *pDestinationBuffer, UINT64 AlignedDestinationBufferOffset) override;
	void    STDMETHODCALLTYPE SetPredication(ID3D12Resource *pBuffer, UINT64 AlignedBufferOffset, D3D12_PREDICATION_OP Operation) override;
	void    STDMETHODCALLTYPE SetMarker(UINT Metadata, const void *pData, UINT Size) override;
	void    STDMETHODCALLTYPE BeginEvent(UINT Metadata, const void *pData, UINT Size) override;
	void    STDMETHODCALLTYPE EndEvent() override;
	void    STDMETHODCALLTYPE ExecuteIndirect(ID3D12CommandSignature *pCommandSignature, UINT MaxCommandCount, ID3D12Resource *pArgumentBuffer, UINT64 ArgumentBufferOffset, ID3D12Resource *pCountBuffer, UINT64 CountBufferOffset) override;
	#pragma endregion
	#pragma region ID3D12GraphicsCommandList1
	void    STDMETHODCALLTYPE AtomicCopyBufferUINT(ID3D12Resource *pDstBuffer, UINT64 DstOffset, ID3D12Resource *pSrcBuffer, UINT64 SrcOffset, UINT Dependencies, ID3D12Resource *const *ppDependentResources, const D3D12_SUBRESOURCE_RANGE_UINT64 *pDependentSubresourceRanges) override;
	void    STDMETHODCALLTYPE AtomicCopyBufferUINT64(ID3D12Resource *pDstBuffer, UINT64 DstOffset, ID3D12Resource *pSrcBuffer, UINT64 SrcOffset, UINT Dependencies, ID3D12Resource *const *ppDependentResources, const D3D12_SUBRESOURCE_RANGE_UINT64 *pDependentSubresourceRanges) override;
	void    STDMETHODCALLTYPE OMSetDepthBounds(FLOAT Min, FLOAT Max) override;
	void    STDMETHODCALLTYPE SetSamplePositions(UINT NumSamplesPerPixel, UINT NumPixels, D3D12_SAMPLE_POSITION *pSamplePositions) override;
	void    STDMETHODCALLTYPE ResolveSubresourceRegion(ID3D12Resource *pDstResource, UINT DstSubresource, UINT DstX, UINT DstY, ID3D12Resource *pSrcResource, UINT SrcSubresource, D3D12_RECT *pSrcRect, DXGI_FORMAT Format, D3D12_RESOLVE_MODE ResolveMode) override;
	void    STDMETHODCALLTYPE SetViewInstanceMask(UINT Mask) override;
	#pragma endregion
	#pragma region ID3D12GraphicsCommandList2
	void    STDMETHODCALLTYPE WriteBufferImmediate(UINT Count, const D3D12_WRITEBUFFERIMMEDIATE_PARAMETER *pParams, const D3D12_WRITEBUFFERIMMEDIATE_MODE *pModes) override;
	#pragma endregion
	#pragma region ID3D12GraphicsCommandList3
	void    STDMETHODCALLTYPE SetProtectedResourceSession(ID3D12ProtectedResourceSession *pProtectedResourceSession) override;
	#pragma endregion
	#pragma region ID3D12GraphicsCommandList4
	void    STDMETHODCALLTYPE BeginRenderPass(UINT NumRenderTargets, const D3D12_RENDER_PASS_RENDER_TARGET_DESC *pRenderTargets, const D3D12_RENDER_PASS_DEPTH_STENCIL_DESC *pDepthStencil, D3D12_RENDER_PASS_FLAGS Flags) override;
	void    STDMETHODCALLTYPE EndRenderPass() override;
	void    STDMETHODCALLTYPE InitializeMetaCommand(ID3D12MetaCommand *pMetaCommand, const void *pInitializationParametersData, SIZE_T InitializationParametersDataSizeInBytes) override;
	void    STDMETHODCALLTYPE ExecuteMetaCommand(ID3D12MetaCommand *pMetaCommand, const void *pExecutionParametersData, SIZE_T ExecutionParametersDataSizeInBytes) override;
	void    STDMETHODCALLTYPE BuildRaytracingAccelerationStructure(const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC *pDesc, UINT NumPostbuildInfoDescs, const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC *pPostbuildInfoDescs) override;
	void    STDMETHODCALLTYPE EmitRaytracingAccelerationStructurePostbuildInfo(const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC *pDesc, UINT NumSourceAccelerationStructures, const D3D12_GPU_VIRTUAL_ADDRESS *pSourceAccelerationStructureData) override;
	void    STDMETHODCALLTYPE CopyRaytracingAccelerationStructure(D3D12_GPU_VIRTUAL_ADDRESS DestAccelerationStructureData, D3D12_GPU_VIRTUAL_ADDRESS SourceAccelerationStructureData, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE Mode) override;
	void    STDMETHODCALLTYPE SetPipelineState1(ID3D12StateObject *pStateObject) override;
	void    STDMETHODCALLTYPE DispatchRays(const D3D12_DISPATCH_RAYS_DESC *pDesc) override;
	#pragma endregion
	#pragma region ID3D12GraphicsCommandList5
	void   STDMETHODCALLTYPE RSSetShadingRate(D3D12_SHADING_RATE BaseShadingRate, const D3D12_SHADING_RATE_COMBINER *pCombiners) override;
	void   STDMETHODCALLTYPE RSSetShadingRateImage(ID3D12Resource *pShadingRateImage) override;
	#pragma endregion
	#pragma region ID3D12GraphicsCommandList6
	void   STDMETHODCALLTYPE DispatchMesh(UINT ThreadGroupCountX, UINT ThreadGroupCountY, UINT ThreadGroupCountZ) override;
	#pragma endregion
	#pragma region ID3D12GraphicsCommandList7
	void   STDMETHODCALLTYPE Barrier(UINT32 NumBarrierGroups, const D3D12_BARRIER_GROUP *pBarrierGroups) override;
	#pragma endregion
	#pragma region ID3D12GraphicsCommandList8
	void   STDMETHODCALLTYPE OMSetFrontAndBackStencilRef(UINT FrontStencilRef, UINT BackStencilRef) override;
	#pragma endregion
	#pragma region ID3D12GraphicsCommandList9
	void   STDMETHODCALLTYPE RSSetDepthBias(FLOAT DepthBias, FLOAT DepthBiasClamp, FLOAT SlopeScaledDepthBias) override;
	void   STDMETHODCALLTYPE IASetIndexBufferStripCutValue(D3D12_INDEX_BUFFER_STRIP_CUT_VALUE IBStripCutValue) override;
	#pragma endregion
	#pragma region ID3D12GraphicsCommandList10
	void   STDMETHODCALLTYPE SetProgram(const D3D12_SET_PROGRAM_DESC *pDesc) override;
        void   STDMETHODCALLTYPE DispatchGraph(const D3D12_DISPATCH_GRAPH_DESC *pDesc) override;
        #pragma endregion

        bool check_and_upgrade_interface(REFIID riid);

        ULONG _ref = 1;
        unsigned short _interface_version = 0;
        D3D12Device *const _device;

private:
        struct root_binding_state
        {
                void clear()
                {
                        descriptor_tables.clear();
                        constant_buffer_views.clear();
                        shader_resource_views.clear();
                        unordered_access_views.clear();
                        constants.clear();
                        constant_masks.clear();
                }

                std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> descriptor_tables;
                std::vector<D3D12_GPU_VIRTUAL_ADDRESS> constant_buffer_views;
                std::vector<D3D12_GPU_VIRTUAL_ADDRESS> shader_resource_views;
                std::vector<D3D12_GPU_VIRTUAL_ADDRESS> unordered_access_views;
                std::vector<std::vector<uint32_t>> constants;
                std::vector<std::vector<uint8_t>> constant_masks;
        };

        template <typename T>
        static void ensure_size(std::vector<T> &container, size_t index)
        {
                if (container.size() <= index)
                        container.resize(index + 1);
        }

        template <typename T>
        static void ensure_size(std::vector<T> &container, size_t index, const T &value)
        {
                if (container.size() <= index)
                        container.resize(index + 1, value);
        }

        root_binding_state &_get_root_state(bool compute_stage)
        {
                return compute_stage ? _root_binding_state[1] : _root_binding_state[0];
        }

        void _reset_root_state(bool compute_stage)
        {
                _get_root_state(compute_stage).clear();
        }

        void _set_root_descriptor_table(bool compute_stage, UINT index, D3D12_GPU_DESCRIPTOR_HANDLE handle);
        void _set_root_constant_buffer_view(bool compute_stage, UINT index, D3D12_GPU_VIRTUAL_ADDRESS address);
        void _set_root_shader_resource_view(bool compute_stage, UINT index, D3D12_GPU_VIRTUAL_ADDRESS address);
        void _set_root_unordered_access_view(bool compute_stage, UINT index, D3D12_GPU_VIRTUAL_ADDRESS address);
        void _set_root_32bit_constant(bool compute_stage, UINT index, UINT offset, UINT value);
        void _set_root_32bit_constants(bool compute_stage, UINT index, UINT dest_offset, UINT count, const void *data);

        ID3D12PipelineState *_current_pipeline_state = nullptr;
        ID3D12StateObject *_current_raytracing_pipeline_state = nullptr;
        UINT _current_num_render_targets = 0;
        D3D12_CPU_DESCRIPTOR_HANDLE _current_render_targets[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = {};
        BOOL _current_rts_single_handle = FALSE;
        D3D12_CPU_DESCRIPTOR_HANDLE _current_depth_stencil = {};
        UINT _current_num_viewports = 0;
        D3D12_VIEWPORT _current_viewports[D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE] = {};
        UINT _current_num_scissor_rects = 0;
        D3D12_RECT _current_scissor_rects[D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE] = {};
        D3D12_PRIMITIVE_TOPOLOGY _current_primitive_topology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
        float _current_blend_factor[4] = {};
        bool _current_blend_factor_valid = false;
        float _current_depth_bias[3] = {};
        bool _current_depth_bias_valid = false;
        UINT _current_stencil_ref = 0;
        bool _current_stencil_ref_valid = false;
        UINT _current_front_stencil_ref = 0;
        UINT _current_back_stencil_ref = 0;
        bool _current_front_back_stencil_valid = false;
        root_binding_state _root_binding_state[2];
};
