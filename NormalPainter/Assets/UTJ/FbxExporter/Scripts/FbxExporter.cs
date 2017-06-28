using System;
using System.Collections.Generic;
using UnityEngine;

namespace UTJ.FbxExporter
{
    public partial class FbxExporter
    {
        feExportOptions m_opt;
        feContext m_ctx;
        List<feNode> m_nodes;

        public FbxExporter()
        {
        }

        ~FbxExporter()
        {
            fbxeReleaseContext(m_ctx);
        }

        public bool CreateScene(string name)
        {
            if(!m_ctx)
                m_ctx = fbxeCreateContext(ref m_opt);
            return fbxeCreateScene(m_ctx, name);
        }

        public feNode GetRootNode()
        {
            return fbxeGetRootNode(m_ctx);
        }
        public feNode CreateNode(feNode parent, string name)
        {
            return fbxeCreateNode(m_ctx, parent, name);
        }

        public void SetTransform(feNode node, Vector3 t, Quaternion r, Vector3 s)
        {
            fbxeSetTRS(m_ctx, node, t, r, s);
        }
        public void SetTransform(feNode node, Transform trans)
        {
            fbxeSetTRS(m_ctx, node, trans.localPosition, trans.localRotation, trans.localScale);
        }

        public void AddMesh(feNode node, Mesh mesh)
        {
            feTopology topology = feTopology.Triangles;

            var indices = mesh.triangles;
            var points = mesh.vertices;
            fbxeAddMesh(m_ctx, node, topology, indices.Length, points.Length, indices, points,
                mesh.normals, mesh.tangents, mesh.uv, mesh.colors);
        }
    }

}
