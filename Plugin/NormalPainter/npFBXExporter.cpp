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


static inline FbxDouble3 ToFbxD3(float3 v)
{
    return {v.x, v.y, v.z};
}



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

    {
        auto settings = m_manager->GetIOSettings();
        settings->SetBoolProp(EXP_FBX_MATERIAL, true);
        settings->SetBoolProp(EXP_FBX_TEXTURE, true);
        settings->SetBoolProp(EXP_FBX_EMBEDDED, false);
        settings->SetBoolProp(EXP_FBX_SHAPE, true);
        settings->SetBoolProp(EXP_FBX_GOBO, true);
        settings->SetBoolProp(EXP_FBX_ANIMATION, true);
        settings->SetBoolProp(EXP_FBX_GLOBAL_SETTINGS, true);
        if (!exporter->Initialize(path, file_format, m_manager->GetIOSettings())) {
            return false;
        }
    }

    bool ret = exporter->Export(m_scene);
    exporter->Destroy();
    return ret;
}

FBXNode* FBXExporterContext::addTransform(FBXNode *parent, const char *name, float3 t, quatf r, float3 s)
{
    if (!m_scene) { return nullptr; }

    auto node = FbxNode::Create(m_scene, name);
    node->LclTranslation.Set(ToFbxD3(t));

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
    auto node = FbxNode::Create(m_scene, name);
    node->LclTranslation.Set(ToFbxD3(t));

    node->SetNodeAttribute(mesh);
    node->SetShadingMode(FbxNode::eTextureShading);

    if (parent) {
        reinterpret_cast<FbxNode*>(parent)->AddChild(node);
    }

    return node;
}

#endif // npEnableFBX
