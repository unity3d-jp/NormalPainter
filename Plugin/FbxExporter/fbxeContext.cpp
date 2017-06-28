#include "pch.h"
#include "FbxExporter.h"
#include "fbxeContext.h"
#include "fbxeUtils.h"

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
    void addMesh(Node *node, int num_vertices,
        const float3 points[], const float3 normals[], const float4 tangents[], const float2 uv[], const float4 colors[]) override;
    void addMeshSubmesh(Node *node, Topology topology, int num_indices, const int indices[], int material) override;
    void addMeshSkin(Node *node, Weights4 weights[], int num_bones, Node *bones[], float4x4 bindposes[]) override;

private:
    ExportOptions m_opt;
    FbxManager *m_manager = nullptr;
    FbxScene *m_scene = nullptr;
};

fbxeAPI IContext* CreateContext(const ExportOptions *opt)
{
    return new Context(opt);
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
    if (!exporter->Initialize(path, file_format)) {
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


void Context::addMesh(Node *node_, int num_vertices,
    const float3 points[], const float3 normals[], const float4 tangents[], const float2 uv[], const float4 colors[])
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

        auto& da = element->GetDirectArray();
        da.Resize(num_vertices);
        auto dst = (FbxVector4*)da.GetLocked();
        for (int i = 0; i < num_vertices; ++i) {
            dst[i] = ToV4(normals[i]);
        }
        if (m_opt.flip_handedness) { FlipHandedness(dst, num_vertices); }
        da.Release((void**)&dst);
    }
    if (tangents) {
        // set tangents
        auto element = mesh->CreateElementTangent();
        element->SetMappingMode(FbxGeometryElement::eByControlPoint);
        element->SetReferenceMode(FbxGeometryElement::eDirect);

        auto& da = element->GetDirectArray();
        da.Resize(num_vertices);
        auto dst = (FbxVector4*)da.GetLocked();
        for (int i = 0; i < num_vertices; ++i) {
            dst[i] = ToV4(tangents[i]);
        }
        if (m_opt.flip_handedness) { FlipHandedness(dst, num_vertices); }
        da.Release((void**)&dst);
    }
    if (uv) {
        // set uv
        auto element = mesh->CreateElementUV("UVSet1");
        element->SetMappingMode(FbxGeometryElement::eByControlPoint);
        element->SetReferenceMode(FbxGeometryElement::eDirect);

        auto& da = element->GetDirectArray();
        da.Resize(num_vertices);
        auto dst = (FbxVector2*)da.GetLocked();
        for (int i = 0; i < num_vertices; ++i) {
            dst[i] = ToV2(uv[i]);
        }
        da.Release((void**)&dst);
    }
    if (colors) {
        // set colors
        auto element = mesh->CreateElementVertexColor();
        element->SetMappingMode(FbxGeometryElement::eByControlPoint);
        element->SetReferenceMode(FbxGeometryElement::eDirect);

        auto& da = element->GetDirectArray();
        da.Resize(num_vertices);
        auto dst = (FbxColor*)da.GetLocked();
        for (int i = 0; i < num_vertices; ++i) {
            dst[i] = ToC4(colors[i]);
        }
        da.Release((void**)&dst);
    }

    node->SetNodeAttribute(mesh);
    node->SetShadingMode(FbxNode::eTextureShading);
}

void Context::addMeshSubmesh(Node *node_, Topology topology, int num_indices, const int indices[], int material)
{
    if (!node_) { return; }

    auto node = reinterpret_cast<FbxNode*>(node_);
    auto mesh = node->GetMesh();
    if (!mesh) { return; }

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
            mesh->BeginPolygon(material);
            for (int vi = vertices_in_primitive - 1; vi >= 0; --vi) {
                mesh->AddPolygon(indices[pi + vi]);
            }
            pi += vertices_in_primitive;
            mesh->EndPolygon();
        }
    }
    else {
        while (pi < num_indices) {
            mesh->BeginPolygon(material);
            for (int vi = 0; vi < vertices_in_primitive; ++vi) {
                mesh->AddPolygon(indices[pi + vi]);
            }
            pi += vertices_in_primitive;
            mesh->EndPolygon();
        }
    }

}


void Context::addMeshSkin(Node *node_, Weights4 weights[], int num_bones, Node *bones[], float4x4 bindposes[])
{
    if (!node_) { return; }

    auto node = reinterpret_cast<FbxNode*>(node_);
    auto mesh = node->GetMesh();
    if (!mesh) { return; }

    auto skin = FbxSkin::Create(m_scene, "");
    skin->SetGeometry(mesh);

    RawVector<int> dindices;
    RawVector<double> dweights;
    int num_vertices = mesh->GetControlPointsCount();
    for (int bi = 0; bi < num_bones; ++bi) {
        if (!bones[bi]) { continue; }

        auto bone = reinterpret_cast<FbxNode*>(bones[bi]);
        auto cluster = FbxCluster::Create(m_scene, "");
        cluster->SetLink(bone);
        cluster->SetLinkMode(FbxCluster::eNormalize);

        auto bindpose = bindposes[bi];
        (float3&)bindpose[3] *= m_opt.scale_factor;
        if (m_opt.flip_handedness) {
            bindpose = swap_handedness(bindpose);
        }
        cluster->SetTransformMatrix(ToAM44(bindpose));

        GetInfluence(weights, num_vertices, bi, dindices, dweights);
        cluster->SetControlPointIWCount((int)dindices.size());
        dindices.copy_to(cluster->GetControlPointIndices());
        dweights.copy_to(cluster->GetControlPointWeights());

        skin->AddCluster(cluster);
    }
}

} // namespace fbxe
