#include <Asset/HoudiniModelBuilderUtil.h>
#include <HoudiniEngine/HoudiniCookedMesh.h>

#include <Atom/RPI.Reflect/Model/ModelAssetCreator.h>
#include <Atom/RPI.Reflect/Model/ModelLodAssetCreator.h>
#include <Atom/RPI.Reflect/Buffer/BufferAssetCreator.h>
#include <Atom/RPI.Reflect/ResourcePoolAssetCreator.h>
#include <AzCore/Math/Aabb.h>

namespace HoudiniEngine
{
    namespace
    {
        // Helper: pack a typed array into a BufferAsset.
        template<typename T>
        AZ::Data::Asset<AZ::RPI::BufferAsset> MakeBuffer(
            const AZStd::vector<T>& data, AZ::RHI::Format format, AZStd::string_view name)
        {
            AZ::RPI::BufferAssetCreator creator;
            creator.Begin(AZ::Uuid::CreateRandom());

            AZ::RHI::BufferDescriptor desc;
            desc.m_bindFlags = AZ::RHI::BufferBindFlags::InputAssembly | AZ::RHI::BufferBindFlags::ShaderRead;
            desc.m_byteCount = data.size() * sizeof(T);
            creator.SetBuffer(data.data(), desc.m_byteCount, desc);

            AZ::RHI::BufferViewDescriptor viewDesc =
                AZ::RHI::BufferViewDescriptor::CreateTyped(0, static_cast<uint32_t>(data.size()), format);
            creator.SetBufferViewDescriptor(viewDesc);
            creator.SetUseCommonPool(AZ::RPI::CommonBufferPoolType::StaticInputAssembly);

            AZ::Data::Asset<AZ::RPI::BufferAsset> asset;
            creator.End(asset);
            if (asset.Get())
            {
                asset.SetHint(AZStd::string(name));
            }
            return asset;
        }
    }

    AZ::Data::Asset<AZ::RPI::ModelAsset> HoudiniModelBuilderUtil::BuildModelAsset(
        const HoudiniCookedMesh& mesh, AZStd::string_view name)
    {
        if (!mesh.IsValid())
        {
            return {};
        }

        // Compute bounds.
        AZ::Aabb aabb = AZ::Aabb::CreateNull();
        for (const AZ::Vector3& p : mesh.m_positions) { aabb.AddPoint(p); }

        // Flatten positions to float3, normals to float3, uvs to float2.
        AZStd::vector<float> positions; positions.reserve(mesh.m_positions.size() * 3);
        for (const AZ::Vector3& p : mesh.m_positions)
        { positions.push_back(p.GetX()); positions.push_back(p.GetY()); positions.push_back(p.GetZ()); }

        AZStd::vector<float> normals;
        const bool haveN = !mesh.m_normals.empty();
        if (haveN) { normals.reserve(mesh.m_normals.size() * 3);
            for (const AZ::Vector3& n : mesh.m_normals)
            { normals.push_back(n.GetX()); normals.push_back(n.GetY()); normals.push_back(n.GetZ()); } }

        AZStd::vector<float> uvs;
        const bool haveUv = !mesh.m_uvs.empty();
        if (haveUv) { uvs.reserve(mesh.m_uvs.size() * 2);
            for (const AZ::Vector2& uv : mesh.m_uvs)
            { uvs.push_back(uv.GetX()); uvs.push_back(uv.GetY()); } }

        auto indexBuffer = MakeBuffer(mesh.m_indices, AZ::RHI::Format::R32_UINT, "houdini_indices");
        auto positionBuffer = MakeBuffer(positions, AZ::RHI::Format::R32G32B32_FLOAT, "houdini_pos");
        AZ::Data::Asset<AZ::RPI::BufferAsset> normalBuffer;
        if (haveN) { normalBuffer = MakeBuffer(normals, AZ::RHI::Format::R32G32B32_FLOAT, "houdini_nrm"); }
        AZ::Data::Asset<AZ::RPI::BufferAsset> uvBuffer;
        if (haveUv) { uvBuffer = MakeBuffer(uvs, AZ::RHI::Format::R32G32_FLOAT, "houdini_uv"); }

        const auto vertexCount = static_cast<uint32_t>(mesh.m_positions.size());
        const auto indexCount = static_cast<uint32_t>(mesh.m_indices.size());

        AZ::RPI::ModelLodAssetCreator lodCreator;
        lodCreator.Begin(AZ::Uuid::CreateRandom());
        lodCreator.BeginMesh();
        lodCreator.SetMeshAabb(AZStd::move(aabb));
        lodCreator.SetMeshIndexBuffer({ indexBuffer,
            AZ::RHI::BufferViewDescriptor::CreateTyped(0, indexCount, AZ::RHI::Format::R32_UINT) });
        lodCreator.AddMeshStreamBuffer(AZ::RHI::ShaderSemantic{ "POSITION" }, AZ::Name(),
            { positionBuffer, AZ::RHI::BufferViewDescriptor::CreateTyped(0, vertexCount, AZ::RHI::Format::R32G32B32_FLOAT) });
        if (haveN)
        {
            lodCreator.AddMeshStreamBuffer(AZ::RHI::ShaderSemantic{ "NORMAL" }, AZ::Name(),
                { normalBuffer, AZ::RHI::BufferViewDescriptor::CreateTyped(0, vertexCount, AZ::RHI::Format::R32G32B32_FLOAT) });
        }
        if (haveUv)
        {
            lodCreator.AddMeshStreamBuffer(AZ::RHI::ShaderSemantic{ "UV", 0 }, AZ::Name(),
                { uvBuffer, AZ::RHI::BufferViewDescriptor::CreateTyped(0, vertexCount, AZ::RHI::Format::R32G32_FLOAT) });
        }
        lodCreator.EndMesh();
        AZ::Data::Asset<AZ::RPI::ModelLodAsset> lodAsset;
        if (!lodCreator.End(lodAsset)) { return {}; }

        AZ::RPI::ModelAssetCreator modelCreator;
        modelCreator.Begin(AZ::Uuid::CreateRandom());
        modelCreator.SetName(name);
        modelCreator.AddLodAsset(AZStd::move(lodAsset));
        AZ::Data::Asset<AZ::RPI::ModelAsset> modelAsset;
        if (!modelCreator.End(modelAsset)) { return {}; }

        modelAsset.SetHint(AZStd::string(name));
        return modelAsset;
    }
} // namespace HoudiniEngine
