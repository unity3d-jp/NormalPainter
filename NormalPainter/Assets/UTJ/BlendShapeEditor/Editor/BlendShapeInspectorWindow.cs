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
            m_scrollPos = EditorGUILayout.BeginScrollView(m_scrollPos);
            EditorGUILayout.BeginVertical();
            if (m_active)
            {
                DrawBlendShapeInspector(m_active);
            }
            EditorGUILayout.EndVertical();
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
                EditorGUILayout.LabelField(numShapes + " blendshapes");
                EditorGUILayout.BeginHorizontal();
                GUILayout.Space(indentSize);
                EditorGUILayout.BeginVertical();

                for (int si = 0; si < numShapes; ++si)
                {
                    var name = target.GetBlendShapeName(si);
                    int numFrames = target.GetBlendShapeFrameCount(si);

                    EditorGUILayout.Space();
                    EditorGUILayout.BeginHorizontal();
                    GUILayout.Label(name + " (" + numFrames + " frames)");
                    if (GUILayout.Button("Extract All", GUILayout.Width(90)))
                        ExtractBlendShapeFrames(target, si);
                    EditorGUILayout.EndHorizontal();

                    EditorGUILayout.BeginHorizontal();
                    GUILayout.Space(indentSize);
                    EditorGUILayout.BeginVertical();
                    for (int fi = 0; fi < numFrames; ++fi)
                    {
                        float weight = target.GetBlendShapeFrameWeight(si, fi);
                        EditorGUILayout.BeginHorizontal();
                        GUILayout.Label(weight.ToString(), GUILayout.Width(30));
                        if (GUILayout.Button("Extract", GUILayout.Width(60)))
                            ExtractBlendShapeFrames(target, si, fi);
                        EditorGUILayout.EndHorizontal();
                    }
                    EditorGUILayout.EndVertical();
                    EditorGUILayout.EndHorizontal();
                }
                EditorGUILayout.EndVertical();
                EditorGUILayout.EndHorizontal();

                GUILayout.Space(6);
                if (GUILayout.Button("Convert To Compose Data"))
                    ConvertToComposeData(target);
            }
        }

        // frameIndex = -1: extract all frames
        public static GameObject[] ExtractBlendShapeFrames(Mesh target, int shapeIndex, int frameIndex = -1)
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

            if (frameIndex < 0)
            {
                var ret = new GameObject[numFrames];
                for (int f = 0; f < numFrames; ++f)
                {
                    target.GetBlendShapeFrameVertices(shapeIndex, f, deltaVertices, deltaNormals, deltaTangents);
                    ApplyDelta(target.vertices, deltaVertices, tmpVertices);
                    ApplyDelta(target.normals, deltaNormals, tmpNormals);
                    ApplyDelta(target.tangents, deltaTangents, tmpTangents);

                    stripped.vertices = tmpVertices;
                    stripped.normals = tmpNormals;
                    stripped.tangents = tmpTangents;

                    var mesh = Instantiate(stripped);
                    mesh.name = name + " [" + f + "]";
                    var pos = new Vector3(width + (f + 1), 0.0f, 0.0f);
                    ret[f] = Utils.MeshToGameObject(mesh.name, mesh, pos);
                }
                return ret;
            }
            else if(frameIndex < numFrames)
            {
                target.GetBlendShapeFrameVertices(shapeIndex, frameIndex, deltaVertices, deltaNormals, deltaTangents);
                ApplyDelta(target.vertices, deltaVertices, tmpVertices);
                ApplyDelta(target.normals, deltaNormals, tmpNormals);
                ApplyDelta(target.tangents, deltaTangents, tmpTangents);

                stripped.vertices = tmpVertices;
                stripped.normals = tmpNormals;
                stripped.tangents = tmpTangents;

                var mesh = Instantiate(stripped);
                mesh.name = name + " [" + frameIndex + "]";
                var pos = new Vector3(width + (frameIndex + 1), 0.0f, 0.0f);
                return new GameObject[1] { Utils.MeshToGameObject(mesh.name, mesh, pos) };
            }
            else
            {
                Debug.LogError("Invalid frame index");
                return null;
            }
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


        public static void ConvertToComposeData(Mesh target)
        {
            int numShapes = target.blendShapeCount;
            if(numShapes == 0)
            {
                Debug.Log("BlendShapeInspector: This mesh has no BlendShape.");
                return;
            }

            var data = new List<BlendShapeData>();
            for (int si = 0; si < numShapes; ++si)
            {
                var name = target.GetBlendShapeName(si);
                int numFrames = target.GetBlendShapeFrameCount(si);
                var gos = ExtractBlendShapeFrames(target, si);

                float step = 100.0f / numFrames;
                float weight = step;
                var bsd = new BlendShapeData();
                data.Add(bsd);
                bsd.name = name;
                foreach(var go in gos)
                {
                    var fd = new BlendShapeFrameData();
                    bsd.frames.Add(fd);
                    fd.mesh = go;
                    fd.weight = weight;
                    weight += step;
                }
            }

            BlendShapeEditorWindow.Open();
            var window = EditorWindow.GetWindow<BlendShapeEditorWindow>();
            window.ModifyBlendShapeData(d => {
                d.baseMesh = target;
                d.blendShapeData = data;
            });
        }

        #endregion

    }
}
