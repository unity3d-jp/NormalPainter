#include "pch.h"
#include "NormalPainter.h"
#include "npFBXExporter.h"

#ifdef npEnableFBX
#include <fbxsdk.h>
#ifdef _WIN32
    #pragma comment(lib, "libfbxsdk-md.lib")
#endif

class FBXExporterContext : public IFBXExporterContext
{
public:
    FBXExporterContext();
    ~FBXExporterContext() override;
    void release() override;
    bool clear() override;

    bool createScene(const char *name) override;
    bool write(const char *path, bool ascii) override;

    FBXNode* getRootNode() override;
    FBXNode* addTransform(FBXNode *parent, const char *name, float3 t, quatf r, float3 s) override;
    FBXNode* addMesh(FBXNode *parent, const char *name,
        float3 t, quatf r, float3 s,
        int num_triangles, int num_vertices,
        const int indices[], const float3 points[], const float3 normals[], const float4 tangents[], const float2 uv[], const float4 colors[],
        Weights4 weights[], int num_bones, const char *bone_names[]) override;

private:
    FbxManager *m_manager = nullptr;
    FbxScene *m_scene = nullptr;
};

npAPI IFBXExporterContext* CreateFBXExporter()
{
    return new FBXExporterContext();
}


static inline FbxVector2 ToV2(float2 v) { return { v.x, v.y }; }
static inline FbxDouble3 ToP3(float3 v) { return { v.x, v.y, v.z }; }
static inline FbxDouble4 ToP4(float3 v) { return { v.x, v.y, v.z, 1.0f }; }
static inline FbxVector4 ToV4(float3 v) { return { v.x, v.y, v.z, 0.0f }; }
static inline FbxDouble4 ToP4(float4 v) { return { v.x, v.y, v.z, v.w }; }
static inline FbxVector4 ToV4(float4 v) { return { v.x, v.y, v.z, v.w }; }
static inline FbxColor   ToC4(float4 v) { return { v.x, v.y, v.z, v.w }; }



FBXExporterContext::FBXExporterContext()
{
    m_manager = FbxManager::Create();
}

FBXExporterContext::~FBXExporterContext()
{
    clear();
    m_manager->Destroy();
}

void FBXExporterContext::release()
{
    delete this;
}

bool FBXExporterContext::clear()
{
    if (m_scene) {
        m_scene->Destroy(true);
        m_scene = nullptr;
        return true;
    }
    return false;
}

bool FBXExporterContext::createScene(const char *name)
{
    if (!m_manager) { return false; }

    clear();
    m_scene = FbxScene::Create(m_manager, name);
    return m_scene != nullptr;
}

bool FBXExporterContext::write(const char *path, bool ascii)
{
    if (!m_scene) { return false; }

    auto exporter = FbxExporter::Create(m_manager, "");

    int file_format = m_manager->GetIOPluginRegistry()->GetNativeWriterFormat();
    if (ascii) {
        int count = m_manager->GetIOPluginRegistry()->GetWriterFormatCount();
        for (int i = 0; i < count; ++i) {
            if (m_manager->GetIOPluginRegistry()->WriterIsFBX(i)) {
                auto desc = m_manager->GetIOPluginRegistry()->GetWriterFormatDescription(i);
                if (std::strstr(desc, "ascii") != nullptr)
                {
                    file_format = i;
                    break;
                }
            }
        }
    }

    auto settings = m_manager->GetIOSettings();
    if (settings) {
        settings->SetBoolProp(EXP_FBX_MATERIAL, true);
        settings->SetBoolProp(EXP_FBX_TEXTURE, true);
        settings->SetBoolProp(EXP_FBX_EMBEDDED, false);
        settings->SetBoolProp(EXP_FBX_SHAPE, true);
        settings->SetBoolProp(EXP_FBX_GOBO, true);
        settings->SetBoolProp(EXP_FBX_ANIMATION, true);
        settings->SetBoolProp(EXP_FBX_GLOBAL_SETTINGS, true);
    }
    if (!exporter->Initialize(path, file_format, m_manager->GetIOSettings())) {
        return false;
    }

    bool ret = exporter->Export(m_scene);
    exporter->Destroy();
    return ret;
}

FBXNode* FBXExporterContext::getRootNode()
{
    if (!m_scene) { return nullptr; }
    return m_scene->GetRootNode();
}

FBXNode* FBXExporterContext::addTransform(FBXNode *parent, const char *name, float3 t, quatf r, float3 s)
{
    if (!m_scene) { return nullptr; }

    auto node = FbxNode::Create(m_scene, name);
    node->LclTranslation.Set(ToP3(t));

    if (parent) {
        reinterpret_cast<FbxNode*>(parent)->AddChild(node);
    }

    return node;
}

FBXNode* FBXExporterContext::addMesh(FBXNode *parent, const char *name,
    float3 t, quatf r, float3 s,
    int num_triangles, int num_vertices,
    const int indices[], const float3 points[], const float3 normals[], const float4 tangents[], const float2 uv[], const float4 colors[],
    Weights4 weights[], int num_bones, const char *bone_names[])
{
    if (!m_scene) { return nullptr; }

    auto mesh = FbxMesh::Create(m_scene, "");

    if (points) {
        // set points
        mesh->InitControlPoints(num_vertices);
        auto dst = mesh->GetControlPoints();
        for (int i = 0; i < num_vertices; ++i) {
            dst[i] = ToP4(points[i]);
        }
    }
    if (normals) {
        // set normals
        auto element = mesh->CreateElementNormal();
        element->SetMappingMode(FbxGeometryElement::eByControlPoint);
        element->SetReferenceMode(FbxGeometryElement::eDirect);

        element->GetDirectArray().Resize(num_vertices);
        auto dst = (FbxVector4*)element->GetDirectArray().GetLocked();
        for (int i = 0; i < num_vertices; ++i) {
            dst[i] = ToV4(normals[i]);
        }
    }
    if (tangents) {
        // set tangents
        auto element = mesh->CreateElementTangent();
        element->SetMappingMode(FbxGeometryElement::eByControlPoint);
        element->SetReferenceMode(FbxGeometryElement::eDirect);

        element->GetDirectArray().Resize(num_vertices);
        auto dst = (FbxVector4*)element->GetDirectArray().GetLocked();
        for (int i = 0; i < num_vertices; ++i) {
            dst[i] = ToV4(tangents[i]);
        }
    }
    if (uv) {
        // set uv
        auto element = mesh->CreateElementUV("UVSet1");
        element->SetMappingMode(FbxGeometryElement::eByControlPoint);
        element->SetReferenceMode(FbxGeometryElement::eDirect);

        element->GetDirectArray().Resize(num_vertices);
        auto dst = (FbxVector2*)element->GetDirectArray().GetLocked();
        for (int i = 0; i < num_vertices; ++i) {
            dst[i] = ToV2(uv[i]);
        }
    }
    if (colors) {
        // set colors
        auto element = mesh->CreateElementVertexColor();
        element->SetMappingMode(FbxGeometryElement::eByControlPoint);
        element->SetReferenceMode(FbxGeometryElement::eDirect);

        element->GetDirectArray().Resize(num_vertices);
        auto dst = (FbxColor*)element->GetDirectArray().GetLocked();
        for (int i = 0; i < num_vertices; ++i) {
            dst[i] = ToC4(colors[i]);
        }
    }

    for (int ti = 0; ti < num_triangles; ++ti) {
        mesh->BeginPolygon();
        for (int i = 0; i < 3; ++i) {
            mesh->AddPolygon(indices[ti * 3 + i]);
        }
        mesh->EndPolygon();
    }

    auto node = FbxNode::Create(m_scene, name);
    node->LclTranslation.Set(ToP3(t));

    node->SetNodeAttribute(mesh);
    node->SetShadingMode(FbxNode::eTextureShading);

    if (parent) {
        reinterpret_cast<FbxNode*>(parent)->AddChild(node);
    }

    return node;
}

#endif // npEnableFBX
