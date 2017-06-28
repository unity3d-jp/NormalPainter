using System;
using System.Collections.Generic;
using UnityEngine;

namespace UTJ.FbxExporter
{
    public partial class FbxExporter
    {
        feExportOptions m_opt;
        feContext m_ctx;
        Dictionary<Transform, feNode> m_nodes;

        public FbxExporter()
        {
        }

        ~FbxExporter()
        {
            Release();
        }

        public void Release()
        {
            fbxeReleaseContext(m_ctx);
            m_ctx = feContext.Null;
        }

        public bool CreateScene(string name)
        {
            Release();
            if (!m_ctx)
                m_ctx = fbxeCreateContext(ref m_opt);
            m_nodes = new Dictionary<Transform, feNode>();
            return fbxeCreateScene(m_ctx, name);
        }

        public void AddNode(Transform trans)
        {
            FindOrCreateNodeTree(trans, ProcessNode);
        }

        public bool Write(string path, feFormat format)
        {
            return fbxeWrite(m_ctx, path, format);
        }


        #region impl
        void ProcessNode(Transform trans, feNode node)
        {
            var mr = trans.GetComponent<MeshRenderer>();
            var smr = trans.GetComponent<SkinnedMeshRenderer>();

            if(smr)
                AddSkinnedMesh(node, smr);
            else if (mr)
                AddMesh(node, mr);
        }

        feNode FindOrCreateNodeTree(Transform trans, Action<Transform, feNode> act)
        {
            if (!trans) { return feNode.Null; }

            if (m_nodes.ContainsKey(trans))
            {
                return m_nodes[trans];
            }
            else
            {
                var parent = !trans.parent ? fbxeGetRootNode(m_ctx) : FindOrCreateNodeTree(trans.parent, act);
                var node = fbxeCreateNode(m_ctx, parent, trans.name);
                fbxeSetTRS(m_ctx, node, trans.localPosition, trans.localRotation, trans.localScale);
                m_nodes.Add(trans, node);

                if (act != null) { act.Invoke(trans, node); }
                return node;
            }
        }

        void AddMesh(feNode node, Mesh mesh)
        {
            feTopology topology = feTopology.Triangles;

            var indices = mesh.triangles;
            var points = mesh.vertices;
            fbxeAddMesh(m_ctx, node, points.Length, points, mesh.normals, mesh.tangents, mesh.uv, mesh.colors);
            fbxeAddMeshSubmesh(m_ctx, node, topology, indices.Length, indices, -1);
        }

        void AddMesh(feNode node, MeshRenderer mr)
        {
            var mf = mr.gameObject.GetComponent<MeshFilter>();
            if (mf)
            {
                var mesh = mf.sharedMesh;
                if (mesh)
                {
                    AddMesh(node, mesh);
                }
            }
        }

        void AddSkinnedMesh(feNode node, SkinnedMeshRenderer smr)
        {
            var mesh = smr.sharedMesh;
            if (mesh)
            {
                AddMesh(node, mesh);

                var bones = smr.bones;
                var bindposes = mesh.bindposes;
                var boneNodes = new feNode[bones.Length];

                fbxeAddSkin(m_ctx, node, mesh.boneWeights, boneNodes.Length, boneNodes, bindposes);
            }
        }
        #endregion
    }

}
