using System;
using System.Runtime.InteropServices;
using UnityEngine;

namespace UTJ.FbxExporter
{
    public partial class FbxExporter
    {
        public struct feContext
        {
            public IntPtr ptr;
            public static explicit operator bool(feContext v) { return v.ptr != IntPtr.Zero; }
        }
        public struct feNode
        {
            public IntPtr ptr;
            public static explicit operator bool(feNode v) { return v.ptr != IntPtr.Zero; }
        }

        public enum feFormat
        {
            FbxBinary,
            FbxAscii,
            FbxEncrypted,
            Obj,
        };
        public enum feTopology
        {
            Points,
            Lines,
            Triangles,
            Quads,
        };


        [DllImport("FbxExporterCore")] static extern feContext fbxeCreateContext();
        [DllImport("FbxExporterCore")] static extern void fbxeReleaseContext(feContext ctx);
        [DllImport("FbxExporterCore")] static extern bool fbxeCreateScene(feContext ctx, string name);
        [DllImport("FbxExporterCore")] static extern bool fbxeWrite(feContext ctx, string path, feFormat format);

        [DllImport("FbxExporterCore")] static extern feNode fbxeGetRootNode(feContext ctx);
        [DllImport("FbxExporterCore")] static extern feNode fbxeFindNodeByName(feContext ctx, string name);

        [DllImport("FbxExporterCore")] static extern feNode fbxeCreateNode(feContext ctx, feNode parent, string name);
        [DllImport("FbxExporterCore")] static extern void fbxeSetTRS(feContext ctx, feNode node, Vector3 t, Quaternion r, Vector3 s);
        [DllImport("FbxExporterCore")] static extern void fbxeAddMesh(feContext ctx, feNode node,
            feTopology topology, int num_indices, int num_vertices,
            int[] indices, Vector3[] points, Vector3[] normals, Vector4[] tangents, Vector2[] uv, Color[] colors);
        [DllImport("FbxExporterCore")] static extern void fbxeAddSkin(feContext ctx, feNode node,
            BoneWeight[] weights, feNode[] bones, Matrix4x4[] bindposes, int num_bones);

    }

}
