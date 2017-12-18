using UnityEngine;
using UnityEditor;
using System;
using System.Collections.Generic;

namespace UTJ.BlendShapeEditor
{
    [Serializable]
    public class BlendShapeFrameData
    {
        public float weight;
        public UnityEngine.Object mesh;
    }

    [Serializable]
    public class BlendShapeData
    {
        public string name;
        public List<BlendShapeFrameData> frames = new List<BlendShapeFrameData>();
    }


    public class BlendShapeEditorWindow : EditorWindow
    {
        #region fields
        public static bool isOpen;

        Vector2 m_scrollPos;
        Mesh m_active;
        List<BlendShapeData> m_data = new List<BlendShapeData>();

        static readonly int indentSize = 18;
        static readonly int spaceSize = 5;
        static readonly int c1Width = 100;
        #endregion



        #region callbacks

        [MenuItem("Window/Blend Shape Editor")]
        public static void Open()
        {
            var window = EditorWindow.GetWindow<BlendShapeEditorWindow>();
            window.titleContent = new GUIContent("BS Editor");
            window.Show();
            window.OnSelectionChange();
        }


        private void OnEnable()
        {
            isOpen = true;
        }

        private void OnDisable()
        {
            isOpen = false;
        }

        private void OnGUI()
        {
            var tooltipHeight = 24;
            var windowHeight = position.height;

            EditorGUILayout.BeginVertical(GUILayout.Height(windowHeight - tooltipHeight));
            m_scrollPos = EditorGUILayout.BeginScrollView(m_scrollPos);

            DrawBlendShapeEditor();

            EditorGUILayout.EndScrollView();
        }

        private void OnSelectionChange()
        {
        }

        #endregion


        #region impl

        public void Clear()
        {
            m_data.Clear();
        }

        public void DrawBlendShapeEditor()
        {
            GUILayout.BeginHorizontal();
            if (GUILayout.Button("Add Blend Shape"))
            {
            }
            GUILayout.EndHorizontal();

            if (GUILayout.Button("Export"))
            {

            }
        }

        static void DrawBlendShapeField(BlendShapeData data)
        {

        }


        public static Mesh MergeShapes(Mesh target, BlendShapeData[] shapes)
        {
            var ret = Instantiate(target);

            var deltaVertices = new Vector3[target.vertexCount];
            var deltaNormals = new Vector3[target.vertexCount];
            var deltaTangents = new Vector3[target.vertexCount];

            foreach (var shape in shapes)
            {
                var lastTarget = target;
                var name = shape.name;

                foreach(var frame in shape.frames)
                {
                    var mesh = Utils.ExtractMesh(frame.mesh);
                    if(mesh == null)
                    {
                        Debug.LogError("BlendShapeEditor: Invalid data in " + name + " at weight " + frame.weight);
                    }
                    else {
                        GenerateDelta(lastTarget.vertices, mesh.vertices, deltaVertices);
                        GenerateDelta(lastTarget.normals, mesh.normals, deltaNormals);
                        GenerateDelta(lastTarget.tangents, mesh.tangents, deltaTangents);
                        ret.AddBlendShapeFrame(name, frame.weight, deltaVertices, deltaNormals, deltaTangents);

                        lastTarget = mesh;
                    }
                }
            }
            return ret;
        }

        private static void GenerateDelta(Vector3[] from, Vector3[] to, Vector3[] dst)
        {
            var len = from.Length;
            for (int i = 0; i < len; ++i)
                dst[i] = to[i] - from[i];
        }

        private static void GenerateDelta(Vector4[] from, Vector4[] to, Vector3[] dst)
        {
            var len = from.Length;
            for (int i = 0; i < len; ++i)
                dst[i] = to[i] - from[i];
        }

        #endregion

    }
}
