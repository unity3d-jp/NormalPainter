#include "pch.h"
#include "FbxExporter.h"
#include "fbxeContext.h"

#include <fbxsdk.h>
#ifdef _WIN32
    #pragma comment(lib, "libfbxsdk-md.lib")
#endif

namespace fbxe {

class Context : public IContext
{
public:
    Context(const ExportOptions *opt);
    ~Context() override;
    void release() override;
    bool clear() override;

    bool createScene(const char *name) override;
    bool write(const char *path, Format format) override;

    Node* getRootNode() override;
    Node* findNodeByName(const char *name) override;

    Node* createNode(Node *parent, const char *name) override;
    void setTRS(Node *node, float3 t, quatf r, float3 s) override;
    void addMesh(Node *node, Topology topology, int num_indices, int num_vertices,
        const int indices[], const float3 points[], const float3 normals[], const float4 tangents[], const float2 uv[], const float4 colors[]) override;
    void addSkin(Node *node, Weights4 weights[], Node *bones[], float4x4 bindposes[], int num_bones) override;

private:
    ExportOptions m_opt;
    FbxManager *m_manager = nullptr;
    FbxScene *m_scene = nullptr;
};

fbxeAPI IContext* CreateContext(const ExportOptions *opt)
{
    return new Context(opt);
}


static inline FbxVector2 ToV2(float2 v) { return { v.x, v.y }; }
static inline FbxDouble3 ToP3(float3 v) { return { v.x, v.y, v.z }; }
static inline FbxDouble4 ToP4(float3 v) { return { v.x, v.y, v.z, 1.0f }; }
static inline FbxVector4 ToV4(float3 v) { return { v.x, v.y, v.z, 0.0f }; }
static inline FbxDouble4 ToP4(float4 v) { return { v.x, v.y, v.z, v.w }; }
static inline FbxVector4 ToV4(float4 v) { return { v.x, v.y, v.z, v.w }; }
static inline FbxColor   ToC4(float4 v) { return { v.x, v.y, v.z, v.w }; }
static inline FbxAMatrix ToAM(float4x4 v)
{
    FbxAMatrix ret;
    auto src = &v[0][0];
    auto dst = (double*)ret;
    for (int i = 0; i < 16; ++i) {
        dst[i] = src[i];
    }
    return ret;
}

static inline void FlipHandedness(FbxDouble4 *v, int n)
{
    for (int i = 0; i < n; ++i) { v[i].mData[0] *= -1.0; }
}
static inline void FlipHandedness(FbxVector4 *v, int n)
{
    for (int i = 0; i < n; ++i) { v[i].mData[0] *= -1.0; }
}




Context::Context(const ExportOptions *opt)
{
    if (opt) { m_opt = *opt; }
    m_manager = FbxManager::Create();
}

Context::~Context()
{
    clear();
    m_manager->Destroy();
}

void Context::release()
{
    delete this;
}

bool Context::clear()
{
    if (m_scene) {
        m_scene->Destroy(true);
        m_scene = nullptr;
        return true;
    }
    return false;
}

bool Context::createScene(const char *name)
{
    if (!m_manager) { return false; }

    clear();
    m_scene = FbxScene::Create(m_manager, name);
    return m_scene != nullptr;
}

bool Context::write(const char *path, Format format)
{
    if (!m_scene) { return false; }

    int file_format = 0;
    {
        // search file format index
        const char *format_name = nullptr;
        switch (format) {
        case Format::FBXAscii: format_name = "FBX ascii"; break;
        case Format::FBXEncrypted: format_name = "FBX encrypted"; break;
        case Format::Obj: format_name = "(*.obj)"; break;
        default: format_name = "FBX binary"; break;
        }

        int n = m_manager->GetIOPluginRegistry()->GetWriterFormatCount();
        for (int i = 0; i < n; ++i) {
            auto desc = m_manager->GetIOPluginRegistry()->GetWriterFormatDescription(i);
            if (std::strstr(desc, format_name) != nullptr)
            {
                file_format = i;
                break;
            }
        }
    }

    // create exporter
    auto exporter = FbxExporter::Create(m_manager, "");
    if (!exporter->Initialize(path, file_format, m_manager->GetIOSettings())) {
        return false;
    }

    // do export
    bool ret = exporter->Export(m_scene);
    exporter->Destroy();
    return ret;
}

Node* Context::getRootNode()
{
    if (!m_scene) { return nullptr; }

    return m_scene->GetRootNode();
}

Node* Context::findNodeByName(const char *name)
{
    if (!m_scene) { return nullptr; }

    int n = m_scene->GetGenericNodeCount();
    for (int i = 0; i < n; ++i) {
        auto node = m_scene->GetGenericNode(i);
        if (std::strcmp(node->GetName(), name) == 0) {
            return node;
        }
    }
    return nullptr;
}

Node* Context::createNode(Node *parent, const char *name)
{
    if (!m_scene) { return nullptr; }

    auto node = FbxNode::Create(m_scene, name);
    if (!node) { return nullptr; }

    if (parent) {
        reinterpret_cast<FbxNode*>(parent)->AddChild(node);
    }
    return node;
}

void Context::setTRS(Node *node_, float3 t, quatf r, float3 s)
{
    if (!node_) { return; }

    t *= m_opt.scale_factor;
    if (m_opt.flip_handedness) {
        t = swap_handedness(t);
        r = swap_handedness(r);
    }

    auto node = (FbxNode*)node_;
    node->LclTranslation.Set(ToP3(t));
    node->RotationOrder.Set(FbxEuler::eOrderZXY);
    node->LclRotation.Set(ToP3(eularZXY(r)));
    node->LclScaling.Set(ToP3(s));
}


void Context::addMesh(Node *node_, Topology topology, int num_indices, int num_vertices,
    const int indices[], const float3 points[], const float3 normals[], const float4 tangents[], const float2 uv[], const float4 colors[])
{
    if (!node_) { return; }
    if (!points) { return; } // points must not be null

    auto node = reinterpret_cast<FbxNode*>(node_);
    auto mesh = FbxMesh::Create(m_scene, "");

    {
        // set points
        mesh->InitControlPoints(num_vertices);
        auto dst = mesh->GetControlPoints();
        for (int i = 0; i < num_vertices; ++i) {
            dst[i] = ToP4(points[i] * m_opt.scale_factor);
        }
        if (m_opt.flip_handedness) { FlipHandedness(dst, num_vertices); }
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
        if (m_opt.flip_handedness) { FlipHandedness(dst, num_vertices); }
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
        if (m_opt.flip_handedness) { FlipHandedness(dst, num_vertices); }
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

    {
        // set primitives

        int vertices_in_primitive = 1;
        switch (topology)
        {
        case Topology::Points:    vertices_in_primitive = 1; break;
        case Topology::Lines:     vertices_in_primitive = 2; break;
        case Topology::Triangles: vertices_in_primitive = 3; break;
        case Topology::Quads:     vertices_in_primitive = 4; break;
        default: break;
        }

        int pi = 0;
        if (m_opt.flip_faces) {
            while (pi < num_indices) {
                mesh->BeginPolygon();
                for (int vi = vertices_in_primitive - 1; vi >= 0; --vi) {
                    mesh->AddPolygon(indices[pi + vi]);
                }
                pi += vertices_in_primitive;
                mesh->EndPolygon();
            }
        }
        else {
            while (pi < num_indices) {
                mesh->BeginPolygon();
                for (int vi = 0; vi < vertices_in_primitive; ++vi) {
                    mesh->AddPolygon(indices[pi + vi]);
                }
                pi += vertices_in_primitive;
                mesh->EndPolygon();
            }
        }
    }

    node->SetNodeAttribute(mesh);
    node->SetShadingMode(FbxNode::eTextureShading);
}

template<int N>
static int GetInfluence(Weights<N> weights[], int num_vertices, int bone_index, RawVector<int>& dindices, RawVector<double>& dweights)
{
    dindices.clear();
    dweights.clear();

    for (int vi = 0; vi < num_vertices; ++vi) {
        for (int i = 0; i < N; ++i) {
            if (weights[vi].indices[i] == bone_index) {
                float w = weights[vi].weights[i];
                if (w > 0.0f) {
                    dindices.push_back(vi);
                    dweights.push_back(w);
                }
                break;
            }
        }
    }
    return (int)dindices.size();
}

static FbxMesh* FindMesh(FbxNode *node)
{
    int n = node->GetNodeAttributeCount();
    for (int i = 0; i < n; ++i) {
        auto attr = node->GetNodeAttributeByIndex(i);
        auto name = attr->GetTypeName();
        if (std::strcmp(name, "Mesh") == 0) {
            return static_cast<FbxMesh*>(attr);
        }
    }
    return nullptr;
}

void Context::addSkin(Node *node_, Weights4 weights[], Node *bones[], float4x4 bindposes[], int num_bones)
{
    if (!node_) { return; }

    auto node = reinterpret_cast<FbxNode*>(node_);
    auto mesh = FindMesh(node);
    if (!mesh) { return; }

    auto skin = FbxSkin::Create(m_scene, "");
    skin->SetGeometry(mesh);

    RawVector<int> dindices;
    RawVector<double> dweights;
    int num_vertices = mesh->GetControlPointsCount();
    for (int bi = 0; bi < num_bones; ++bi) {
        auto cluster = FbxCluster::Create(m_scene, "");
        cluster->SetLink((FbxNode*)bones[bi]);

        auto bindpose = bindposes[bi];
        (float3&)bindpose[3] *= m_opt.scale_factor;
        if (m_opt.flip_handedness) {
            bindpose = swap_handedness(bindpose);
        }
        cluster->SetTransformMatrix(ToAM(invert(bindpose)));

        GetInfluence(weights, num_vertices, bi, dindices, dweights);
        cluster->SetControlPointIWCount((int)dindices.size());
        dindices.copy_to(cluster->GetControlPointIndices());
        dweights.copy_to(cluster->GetControlPointWeights());

        skin->AddCluster(cluster);
    }
}

} // namespace fbxe
