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
        public bool fold = true;
        public string name = "";
        public List<BlendShapeFrameData> frames = new List<BlendShapeFrameData>();
    }


    public class BlendShapeEditorWindow : EditorWindow
    {
        #region fields
        public static bool isOpen;

        Vector2 m_scrollPos;
        UnityEngine.Object m_base;
        List<BlendShapeData> m_data = new List<BlendShapeData>();

        bool m_foldBlendShapes = true;

        static readonly int indentSize = 18;
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
            m_scrollPos = EditorGUILayout.BeginScrollView(m_scrollPos);
            EditorGUILayout.BeginVertical(GUILayout.Height(position.height), GUILayout.Width(position.width));
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
            EditorGUI.BeginChangeCheck();
            var baseMesh = EditorGUILayout.ObjectField("Base Mesh", m_base, typeof(UnityEngine.Object), true);
            if (EditorGUI.EndChangeCheck())
                m_base = baseMesh;

            m_foldBlendShapes = EditorGUILayout.Foldout(m_foldBlendShapes, "BlendShapes");
            if (m_foldBlendShapes)
            {
                BlendShapeData delBS = null; ;

                EditorGUI.indentLevel++;
                foreach (var data in m_data)
                {
                    data.fold = EditorGUILayout.Foldout(data.fold, data.name);
                    if (data.fold)
                    {
                        EditorGUI.indentLevel++;
                        EditorGUI.BeginChangeCheck();
                        var name = EditorGUILayout.TextField("Name", data.name);
                        if (EditorGUI.EndChangeCheck())
                            data.name = name;

                        BlendShapeFrameData delFrame = null;
                        foreach (var frame in data.frames)
                        {
                            EditorGUILayout.BeginHorizontal();

                            EditorGUI.BeginChangeCheck();
                            var w = EditorGUILayout.FloatField(frame.weight, GUILayout.Width(80));
                            if (EditorGUI.EndChangeCheck())
                                frame.weight = w;

                            EditorGUI.BeginChangeCheck();
                            var m = EditorGUILayout.ObjectField(frame.mesh, typeof(UnityEngine.Object), true);
                            if (EditorGUI.EndChangeCheck())
                                frame.mesh = m;

                            if (GUILayout.Button("-"))
                                delFrame = frame;

                            EditorGUILayout.EndHorizontal();
                        }
                        if(delFrame != null)
                            data.frames.Remove(delFrame);

                        GUILayout.BeginHorizontal();
                        GUILayout.Space(indentSize * 2);
                        if (GUILayout.Button("Add Frame"))
                            data.frames.Add(new BlendShapeFrameData());
                        if (GUILayout.Button("Delete"))
                            delBS = data;
                        GUILayout.EndHorizontal();

                        EditorGUI.indentLevel--;
                    }
                }
                EditorGUI.indentLevel--;

                if (delBS != null)
                    m_data.Remove(delBS);

                GUILayout.Space(6);
                GUILayout.BeginHorizontal();
                GUILayout.Space(indentSize * 1);
                if (GUILayout.Button("Add Blend Shape"))
                {
                    var tmp = new BlendShapeData();
                    tmp.name = "NewBlendShape" + m_data.Count;
                    tmp.frames.Add(new BlendShapeFrameData { weight = 100.0f });
                    m_data.Add(tmp);
                }
                GUILayout.EndHorizontal();
            }

            GUILayout.Space(12);

            if (GUILayout.Button("Export"))
            {
                var result = Generate();
                if (result != null)
                {
                    Utils.MeshToGameObject(result.name, result, Vector3.zero);
                }
            }
        }


        public Mesh Generate()
        {
            var baseMesh = Utils.ExtractMesh(m_base);
            if(baseMesh == null)
            {
                Debug.LogError("BlendShapeEditor: Base mesh is not set");
                return null;
            }

            var ret = Instantiate(baseMesh);

            var baseVertices = baseMesh.vertices;
            var baseNormals = baseMesh.normals;
            var baseTangents = baseMesh.tangents;

            var deltaVertices = new Vector3[ret.vertexCount];
            var deltaNormals = new Vector3[ret.vertexCount];
            var deltaTangents = new Vector3[ret.vertexCount];

            foreach (var shape in m_data)
            {
                var name = shape.name;

                foreach(var frame in shape.frames)
                {
                    var mesh = Utils.ExtractMesh(frame.mesh);
                    if(mesh == null)
                    {
                        Debug.LogError("BlendShapeEditor: Invalid data in " + name + " at weight " + frame.weight);
                    }
                    else if (mesh.vertexCount != baseMesh.vertexCount)
                    {
                        Debug.LogError("BlendShapeEditor: Invalid mesh (vertex count doesn't match) in " + name + " at weight " + frame.weight);
                    }
                    else
                    {
                        GenerateDelta(baseVertices, mesh.vertices, deltaVertices);
                        GenerateDelta(baseNormals, mesh.normals, deltaNormals);
                        GenerateDelta(baseTangents, mesh.tangents, deltaTangents);
                        ret.AddBlendShapeFrame(name, frame.weight, deltaVertices, deltaNormals, deltaTangents);
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
