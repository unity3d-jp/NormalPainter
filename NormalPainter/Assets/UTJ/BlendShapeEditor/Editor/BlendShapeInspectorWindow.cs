using UnityEngine;
using UnityEditor;
using System;
using System.Collections.Generic;

namespace UTJ.BlendShapeEditor
{
    public class BlendShapeInspectorWindow : EditorWindow
    {
        #region fields
        public static bool isOpen;

        Vector2 m_scrollPos;
        Mesh m_active;

        static readonly int indentSize = 18;
        static readonly int spaceSize = 5;
        static readonly int c1Width = 100;
        #endregion



        #region callbacks

        [MenuItem("Window/Blend Shape Inspector")]
        public static void Open()
        {
            var window = EditorWindow.GetWindow<BlendShapeInspectorWindow>();
            window.titleContent = new GUIContent("BS Inspector");
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

            if (m_active)
            {
                DrawBlendShapeInspector(m_active);
            }

            EditorGUILayout.EndScrollView();
        }

        private void OnSelectionChange()
        {
            m_active = null;

            var activeGameObject = Selection.activeGameObject;
            if (activeGameObject != null)
                m_active = Utils.ExtractMesh(activeGameObject);
            else
                m_active = Utils.ExtractMesh(Selection.activeObject);
            Repaint();
        }

        #endregion


        #region impl

        public static void DrawBlendShapeInspector(Mesh target)
        {
            int numShapes = target.blendShapeCount;
            if(numShapes > 0)
            {
                GUILayout.Label(numShapes + " blendshapes");
                GUILayout.BeginHorizontal();

                GUILayout.Space(indentSize);

                GUILayout.BeginVertical();
                for (int si = 0; si < numShapes; ++si)
                {
                    var name = target.GetBlendShapeName(si);
                    int numFrames = target.GetBlendShapeFrameCount(si);

                    GUILayout.BeginHorizontal();
                    GUILayout.Label(name + " (" + numFrames + " frames)");
                    if (GUILayout.Button("Extract"))
                    {
                        ExtractShapes(target, si);
                    }
                    GUILayout.EndHorizontal();

                    for (int fi = 0; fi < numFrames; ++fi)
                    {
                        float weight = target.GetBlendShapeFrameWeight(si, fi);
                    }
                }
                GUILayout.EndVertical();
                GUILayout.EndHorizontal();
            }
        }

        public static void ExtractShapes(Mesh target, int shapeIndex)
        {
            var name = target.GetBlendShapeName(shapeIndex);
            var numFrames = target.GetBlendShapeFrameCount(shapeIndex);

            var tmpVertices = new Vector3[target.vertexCount];
            var tmpNormals = new Vector3[target.vertexCount];
            var tmpTangents = new Vector4[target.vertexCount];

            var deltaVertices = new Vector3[target.vertexCount];
            var deltaNormals = new Vector3[target.vertexCount];
            var deltaTangents = new Vector3[target.vertexCount];

            var stripped = Instantiate(target);
            stripped.ClearBlendShapes();

            var width = target.bounds.extents.x * 2.0f;
            var mat = AssetDatabase.GetBuiltinExtraResource<Material>("Default-Diffuse.mat");
            var outputs = new GameObject[numFrames];

            for (int f = 0; f < numFrames; ++f)
            {
                target.GetBlendShapeFrameVertices(shapeIndex, f, deltaVertices, deltaNormals, deltaTangents);
                ApplyDelta(stripped.vertices, deltaVertices, tmpVertices);
                ApplyDelta(stripped.normals, deltaNormals, tmpNormals);
                ApplyDelta(stripped.tangents, deltaTangents, tmpTangents);

                stripped.vertices = tmpVertices;
                stripped.normals = tmpNormals;
                stripped.tangents = tmpTangents;

                var mesh = Instantiate(stripped);

                var go = new GameObject(name + " [" + f + "]");
                outputs[f] = go;
                var mf = go.AddComponent<MeshFilter>();
                mf.sharedMesh = mesh;
                var mr = go.AddComponent<MeshRenderer>();
                mr.sharedMaterial = mat;
                go.GetComponent<Transform>().position = new Vector3(width + (f + 1), 0.0f, 0.0f);
            }

            //Selection.objects = outputs;
        }

        private static void ApplyDelta(Vector3[] from, Vector3[] delta, Vector3[] dst)
        {
            var len = from.Length;
            for (int i = 0; i < len; ++i)
                dst[i] = from[i] + delta[i];
        }

        private static void ApplyDelta(Vector4[] from, Vector3[] delta, Vector4[] dst)
        {
            var len = from.Length;
            for (int i = 0; i < len; ++i)
            {
                dst[i].x = from[i].x + delta[i].x;
                dst[i].y = from[i].y + delta[i].y;
                dst[i].z = from[i].z + delta[i].z;
                dst[i].w = from[i].w;
            }
        }


        #endregion

    }
}
