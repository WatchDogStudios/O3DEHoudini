#include <HAPI/HapiWrapper.h>
#include <AzCore/std/string/conversions.h>

namespace HoudiniEngine::Hapi
{
    AZStd::string LastError(const HAPI_Session& session)
    {
        int len = 0;
        HAPI_GetStatusStringBufLength(
            &session, HAPI_STATUS_CALL_RESULT, HAPI_STATUSVERBOSITY_ERRORS, &len);
        if (len <= 0) { return {}; }
        AZStd::string s(static_cast<size_t>(len), '\0');
        HAPI_GetStatusString(&session, HAPI_STATUS_CALL_RESULT, s.data(), len);
        return s;
    }

    static AZ::Outcome<AZStd::string, AZStd::string> ReadString(
        const HAPI_Session& session, HAPI_StringHandle handle)
    {
        int len = 0;
        if (HAPI_GetStringBufLength(&session, handle, &len) != HAPI_RESULT_SUCCESS || len <= 0)
        {
            return AZ::Failure(AZStd::string("HAPI_GetStringBufLength failed"));
        }
        AZStd::string s(static_cast<size_t>(len), '\0');
        if (HAPI_GetString(&session, handle, s.data(), len) != HAPI_RESULT_SUCCESS)
        {
            return AZ::Failure(AZStd::string("HAPI_GetString failed"));
        }
        // HAPI strings are null-terminated within the buffer; trim the trailing null.
        if (!s.empty() && s.back() == '\0') { s.pop_back(); }
        return AZ::Success(s);
    }

    AZ::Outcome<AZStd::string, AZStd::string> LoadFirstOperator(
        const HAPI_Session& session, const char* hdaPath)
    {
        HAPI_AssetLibraryId libId = -1;
        if (HAPI_LoadAssetLibraryFromFile(&session, hdaPath, true, &libId) != HAPI_RESULT_SUCCESS)
        {
            return AZ::Failure(LastError(session));
        }
        int count = 0;
        if (HAPI_GetAvailableAssetCount(&session, libId, &count) != HAPI_RESULT_SUCCESS || count < 1)
        {
            return AZ::Failure(AZStd::string("HDA contains no operators"));
        }
        AZStd::vector<HAPI_StringHandle> names(static_cast<size_t>(count));
        if (HAPI_GetAvailableAssets(&session, libId, names.data(), count) != HAPI_RESULT_SUCCESS)
        {
            return AZ::Failure(LastError(session));
        }
        return ReadString(session, names[0]);
    }

    static VoidOutcome WaitForCook(const HAPI_Session& session)
    {
        int state = HAPI_STATE_STARTING_COOK;
        do
        {
            if (HAPI_GetStatus(&session, HAPI_STATUS_COOK_STATE, &state) != HAPI_RESULT_SUCCESS)
            {
                return AZ::Failure(LastError(session));
            }
        } while (state > HAPI_STATE_MAX_READY_STATE);

        if (state != HAPI_STATE_READY)
        {
            return AZ::Failure(AZStd::string::format("cook ended in error state %d: %s",
                state, LastError(session).c_str()));
        }
        return AZ::Success();
    }

    AZ::Outcome<HAPI_NodeId, AZStd::string> CreateAndCook(
        const HAPI_Session& session, const char* operatorName)
    {
        HAPI_NodeId nodeId = -1;
        if (HAPI_CreateNode(&session, -1, operatorName, "houdini_engine_node",
                /*cook_on_creation*/ true, &nodeId) != HAPI_RESULT_SUCCESS)
        {
            return AZ::Failure(LastError(session));
        }
        auto cooked = WaitForCook(session);
        if (!cooked.IsSuccess()) { return AZ::Failure(cooked.TakeError()); }
        return AZ::Success(nodeId);
    }

    VoidOutcome Cook(const HAPI_Session& session, HAPI_NodeId nodeId)
    {
        HAPI_CookOptions opts = HAPI_CookOptions_Create();
        if (HAPI_CookNode(&session, nodeId, &opts) != HAPI_RESULT_SUCCESS)
        {
            return AZ::Failure(LastError(session));
        }
        return WaitForCook(session);
    }

    static bool ReadFloatAttr(const HAPI_Session& session, HAPI_NodeId node, HAPI_PartId part,
        const char* name, HAPI_AttributeOwner owner, AZStd::vector<float>& out, int& tupleSize)
    {
        HAPI_AttributeInfo info;
        if (HAPI_GetAttributeInfo(&session, node, part, name, owner, &info) != HAPI_RESULT_SUCCESS
            || !info.exists)
        {
            return false;
        }
        tupleSize = info.tupleSize;
        out.resize(static_cast<size_t>(info.count) * info.tupleSize);
        return HAPI_GetAttributeFloatData(&session, node, part, name, &info,
            /*stride*/ -1, out.data(), 0, info.count) == HAPI_RESULT_SUCCESS;
    }

    VoidOutcome ExtractDisplayPart(
        const HAPI_Session& session, HAPI_NodeId nodeId, RawCookedPart& out)
    {
        out = {};

        HAPI_GeoInfo geoInfo;
        if (HAPI_GetDisplayGeoInfo(&session, nodeId, &geoInfo) != HAPI_RESULT_SUCCESS)
        {
            return AZ::Failure(LastError(session));
        }
        const HAPI_NodeId geoNode = geoInfo.nodeId;

        HAPI_PartInfo partInfo;
        if (HAPI_GetPartInfo(&session, geoNode, 0, &partInfo) != HAPI_RESULT_SUCCESS)
        {
            return AZ::Failure(LastError(session));
        }
        if (partInfo.faceCount == 0)
        {
            return AZ::Failure(AZStd::string("display part has no faces (not a polygon mesh)"));
        }

        // Positions (point domain).
        int tuple = 0;
        if (!ReadFloatAttr(session, geoNode, 0, "P", HAPI_ATTROWNER_POINT, out.m_positions, tuple))
        {
            return AZ::Failure(AZStd::string("missing point attribute P"));
        }

        // Face counts + vertex list.
        out.m_faceCounts.resize(static_cast<size_t>(partInfo.faceCount));
        if (HAPI_GetFaceCounts(&session, geoNode, 0, out.m_faceCounts.data(), 0, partInfo.faceCount)
            != HAPI_RESULT_SUCCESS)
        {
            return AZ::Failure(LastError(session));
        }
        out.m_vertexList.resize(static_cast<size_t>(partInfo.vertexCount));
        if (HAPI_GetVertexList(&session, geoNode, 0, out.m_vertexList.data(), 0, partInfo.vertexCount)
            != HAPI_RESULT_SUCCESS)
        {
            return AZ::Failure(LastError(session));
        }

        // Optional vertex-domain normals + uvs (best effort; absence is fine).
        int normalTuple = 0;
        ReadFloatAttr(session, geoNode, 0, "N", HAPI_ATTROWNER_VERTEX, out.m_normals, normalTuple);
        int uvTuple = 0;
        if (ReadFloatAttr(session, geoNode, 0, "uv", HAPI_ATTROWNER_VERTEX, out.m_uvs, uvTuple))
        {
            out.m_uvTupleSize = uvTuple;
        }

        return AZ::Success();
    }
} // namespace HoudiniEngine::Hapi
