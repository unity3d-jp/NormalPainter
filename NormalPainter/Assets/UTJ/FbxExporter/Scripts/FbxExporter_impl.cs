using System;
using System.Runtime.InteropServices;
using UnityEngine;

namespace UTJ.FbxExporter
{
    public partial class FbxExporter
    {
        public struct Context
        {
            public IntPtr ptr;
            public static Context Null { get { Context r; r.ptr = IntPtr.Zero; return r;  } }
            public static implicit operator bool(Context v) { return v.ptr != IntPtr.Zero; }
        }
        public struct Node
        {
            public IntPtr ptr;
            public static Node Null { get { Node r; r.ptr = IntPtr.Zero; return r; } }
            public static implicit operator bool(Node v) { return v.ptr != IntPtr.Zero; }
        }

        public enum Format
        {
            FbxBinary,
            FbxAscii,
            FbxEncrypted,
            Obj,
        };
        public enum Topology
        {
            Points,
            Lines,
            Triangles,
            Quads,
        };

        public struct ExportOptions
        {
            public bool flip_handedness;
            public bool flip_faces;
            public float scale_factor;

            public static ExportOptions defaultValue
            {
                get
                {
                    return new ExportOptions {
                        flip_handedness = true,
                        flip_faces = true,
                        scale_factor = 1.0f,
                    };
                }
            }
        };



        [DllImport("FbxExporterCore")] static extern Context fbxeCreateContext(ref ExportOptions opt);
        [DllImport("FbxExporterCore")] static extern void fbxeReleaseContext(Context ctx);

        [DllImport("FbxExporterCore")] static extern bool fbxeCreateScene(Context ctx, string name);
        [DllImport("FbxExporterCore")] static extern bool fbxeWrite(Context ctx, string path, Format format);

        [DllImport("FbxExporterCore")] static extern Node fbxeGetRootNode(Context ctx);
        [DllImport("FbxExporterCore")] static extern Node fbxeFindNodeByName(Context ctx, string name);

        [DllImport("FbxExporterCore")] static extern Node fbxeCreateNode(Context ctx, Node parent, string name);
        [DllImport("FbxExporterCore")] static extern void fbxeSetTRS(Context ctx, Node node, Vector3 t, Quaternion r, Vector3 s);
        [DllImport("FbxExporterCore")] static extern void fbxeAddMesh(Context ctx, Node node,
            int num_vertices, Vector3[] points, Vector3[] normals, Vector4[] tangents, Vector2[] uv, Color[] colors);
        [DllImport("FbxExporterCore")] static extern void fbxeAddMeshSubmesh(Context ctx, Node node,
            Topology topology, int num_indices, int[] indices, int material);
        [DllImport("FbxExporterCore")] static extern void fbxeAddMeshSkin(Context ctx, Node node,
            BoneWeight[] weights, int num_bones, Node[] bones, Matrix4x4[] bindposes);

    }

}
