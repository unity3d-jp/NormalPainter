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
            public static feContext Null { get { feContext r; r.ptr = IntPtr.Zero; return r;  } }
            public static implicit operator bool(feContext v) { return v.ptr != IntPtr.Zero; }
        }
        public struct feNode
        {
            public IntPtr ptr;
            public static feNode Null { get { feNode r; r.ptr = IntPtr.Zero; return r; } }
            public static implicit operator bool(feNode v) { return v.ptr != IntPtr.Zero; }
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

        public struct feExportOptions
        {
            public bool flip_handedness;
            public bool flip_faces;
            public float scale_factor;

            static feExportOptions default_value
            {
                get
                {
                    return new feExportOptions {
                        flip_handedness = true,
                        flip_faces = true,
                        scale_factor = 1.0f,
                    };
                }
            }
        };



        [DllImport("FbxExporterCore")] static extern feContext fbxeCreateContext(ref feExportOptions opt);
        [DllImport("FbxExporterCore")] static extern void fbxeReleaseContext(feContext ctx);

        [DllImport("FbxExporterCore")] static extern bool fbxeCreateScene(feContext ctx, string name);
        [DllImport("FbxExporterCore")] static extern bool fbxeWrite(feContext ctx, string path, feFormat format);

        [DllImport("FbxExporterCore")] static extern feNode fbxeGetRootNode(feContext ctx);
        [DllImport("FbxExporterCore")] static extern feNode fbxeFindNodeByName(feContext ctx, string name);

        [DllImport("FbxExporterCore")] static extern feNode fbxeCreateNode(feContext ctx, feNode parent, string name);
        [DllImport("FbxExporterCore")] static extern void fbxeSetTRS(feContext ctx, feNode node, Vector3 t, Quaternion r, Vector3 s);
        [DllImport("FbxExporterCore")] static extern void fbxeAddMesh(feContext ctx, feNode node,
            int num_vertices, Vector3[] points, Vector3[] normals, Vector4[] tangents, Vector2[] uv, Color[] colors);
        [DllImport("FbxExporterCore")] static extern void fbxeAddMeshSubmesh(feContext ctx, feNode node,
            feTopology topology, int num_indices, int[] indices, int material);
        [DllImport("FbxExporterCore")] static extern void fbxeAddSkin(feContext ctx, feNode node,
            BoneWeight[] weights, int num_bones, feNode[] bones, Matrix4x4[] bindposes);

    }

}
